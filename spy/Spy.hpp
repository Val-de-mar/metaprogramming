#pragma once
#include <bits/iterator_concepts.h>
#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>
#include <concepts>
#include <iostream>


// Allocator is only used in bonus SBO tests,
// ignore if you don't need bonus points.

struct LoggerVirtualTable {
  typedef LoggerVirtualTable*(* TCopyFunction)(LoggerVirtualTable* from);
  typedef void(* TDestroyFunction)(LoggerVirtualTable* ptr);
  typedef void(* TLogFunction)(LoggerVirtualTable* logger);

  TCopyFunction copy;
  TDestroyFunction destroy;
  TLogFunction initLog;
  TLogFunction endLog;

  [[gnu::always_inline]] LoggerVirtualTable* Copy() {
    return copy(this);
  }
  [[gnu::always_inline]] void Destroy() {
    destroy(this);
  }
  [[gnu::always_inline]] void InitLog() {
    initLog(this);
  }
  [[gnu::always_inline]] void EndLog() {
    endLog(this);
  }
};


template <std::invocable<unsigned int> T>
struct ErasedLogger : LoggerVirtualTable {
  T data;
  int counter = 0;


  static LoggerVirtualTable* CopyFunction(LoggerVirtualTable* from) requires std::copyable<T> {
    auto& object = *static_cast<ErasedLogger<T>*>(from);
    auto copy = new ErasedLogger<T> (object);
    return copy;
  }
  consteval static TCopyFunction GetCopyFunction() {
    if constexpr (std::copyable<T>) {
      return &CopyFunction;
    } else {
      return nullptr;
    }
  }

  static void DestroyFunction(LoggerVirtualTable* ptr) {
    delete static_cast<ErasedLogger<T>*>(ptr);
  }
  static void InitLogFunction(LoggerVirtualTable* logger) {
    auto& object = *static_cast<ErasedLogger<T>*>(logger);
    ++object.counter;
  }
  static void EndLogFunction(LoggerVirtualTable* logger) {
    auto& object = *static_cast<ErasedLogger<T>*>(logger);
    if(object.counter != 0) {
      object.data(object.counter);
      object.counter = 0;
    }
  }

  ErasedLogger(const ErasedLogger& other)
    : LoggerVirtualTable(other)
    , data(other.data)
    , counter(0){
  }

  ErasedLogger<T>(T logger)
    : LoggerVirtualTable{.copy = GetCopyFunction(), .destroy=&DestroyFunction, .initLog=&InitLogFunction, .endLog=&EndLogFunction}
    , data(std::move(logger))
  { }
};

template <class T>
class Spy {
public:
  explicit Spy(T val) : value_{std::move(val)} {}

  T& operator *() {
    
    return value_;
  }
  const T& operator *() const {
    return value_;
  }

  struct ImplicitPointer {
    T* ptr;
    LoggerVirtualTable* vt;
    T& operator*() {
      return *ptr;
    }
    T* operator->() {
      return ptr;
    }
    ~ImplicitPointer() {
      if(vt){
        vt->EndLog();
      }
    }
  };

  ImplicitPointer operator ->() {
    if(logger_) logger_->InitLog();
    return {&value_, logger_};
  }

  Spy() requires std::default_initializable<T> = default;

  bool operator==(const Spy<T>& other) const requires std::regular<T> {
    return value_ == other.value_;
  }

  Spy(const Spy& other) requires std::copyable<T>
    : value_(other.value_)
    , logger_(other.logger_?other.logger_->Copy():nullptr)
  {}

  Spy& operator=(const Spy& other) requires std::copyable<T> {
    if (this == &other) {
      return *this;
    }
    value_ = other.value_;
    resetLogger();
    logger_ = other.logger_ ? other.logger_->Copy(): nullptr;
    return *this;
  }

  Spy(Spy&& other) noexcept(std::is_nothrow_move_constructible_v<T>) requires std::movable<T> 
    : value_(std::move(other.value_))
    , logger_(other.logger_)
  {
    other.logger_ = nullptr;
  }

  Spy& operator=(Spy&& other) noexcept(std::is_nothrow_move_assignable_v<T>) requires std::movable<T> {
    if (this == &other) {
      return *this;
    }
    value_ = std::move(other.value_);
    resetLogger();
    logger_ = other.logger_;
    other.logger_ = nullptr;
    return *this;
  }
  /*
   * if needed (see task readme):
   *   default constructor
   *   copy and move construction
   *   copy and move assignment
   *   equality operators
   *   destructor
  */

  // Resets logger
  void resetLogger() {
    if (logger_ == nullptr) {
      return;
    }
    logger_->Destroy();
    logger_ = nullptr;
  }
  
  template <std::invocable<unsigned int> Logger> requires 
    (std::is_nothrow_destructible_v<std::remove_reference_t<Logger>> || !std::is_nothrow_destructible_v<T>) &&
    (std::copyable<std::remove_reference_t<Logger>> || (!std::copyable<T>))
  void setLogger(Logger&& logger) {
    resetLogger();
    logger_ = new ErasedLogger<std::remove_reference_t<Logger>>(std::forward<Logger>(logger));
  }

  ~Spy() noexcept(std::is_nothrow_destructible_v<T>){
    if(logger_) logger_->Destroy();
  }

private:
  T value_;
  LoggerVirtualTable *logger_ = nullptr;
};