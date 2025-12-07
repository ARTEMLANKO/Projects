#pragma once

#include <cstddef>
#include <type_traits>

namespace ct {

template <typename T>
class Vector {
  static_assert(std::is_nothrow_move_constructible_v<T> || std::is_copy_constructible_v<T>);

  Vector(size_t new_capacity)
      : Vector() {
    data_ = static_cast<T*>(operator new(sizeof(T) * new_capacity, static_cast<std::align_val_t>(alignof(T))));
    capacity_ = new_capacity;
  }

  template <typename U>
  void push_back_impl(U&& value) {
    if (size_ < capacity_) {
      new (data_ + size_) T(std::forward<U>(value));
      size_++;
      return;
    }
    size_t new_capacity = capacity_ ? 2 * capacity_ : 1;
    Vector tmp(new_capacity);
    new (tmp.data_ + size_) T(std::forward<U>(value));
    ++tmp.size_;
    for (size_t i = 0; i < size_; i++) {
      try {
        new (tmp.data_ + tmp.size_ - 1) T(std::move_if_noexcept(data_[i]));
        ++tmp.size_;
      } catch (...) {
        tmp.data_[size_].~T();
        --tmp.size_;
        throw;
      }
    }
    swap(tmp);
  }

public:
  using ValueType = T;

  using Reference = T&;
  using ConstReference = const T&;

  using Pointer = T*;
  using ConstPointer = const T*;

  using Iterator = Pointer;
  using ConstIterator = ConstPointer;

  size_t size_;
  size_t capacity_;
  T* data_;

private:
  template <typename U>
  Iterator insert_impl(ConstIterator pos, U&& value) {
    size_t diff = pos - begin();
    push_back_impl(std::forward<U>(value));
    Iterator iter1 = begin() + diff;
    Iterator iter2 = end() - 1;
    while (iter2 != iter1) {
      std::iter_swap(iter2, iter2 - 1);
      iter2--;
    }
    return iter2;
  }

public:
  // O(1) nothrow
  Vector() noexcept
      : size_(0)
      , capacity_(0)
      , data_(nullptr) {}

  // O(N) strong
  Vector(const Vector& other)
      : Vector() {
    data_ = other.capacity_ == 0
              ? nullptr
              : static_cast<T*>(operator new(sizeof(T) * other.capacity_, static_cast<std::align_val_t>(alignof(T))));
    if (data_ != nullptr) {
      size_ = 0;
      for (; size_ < other.size_; ++size_) {
        new (data_ + size_) T(other.data_[size_]);
      }
    }
    capacity_ = other.size_;
  }

  // O(1) strong
  Vector(Vector&& other)
      : Vector() {
    swap(other);
  }

  // O(N) strong
  Vector& operator=(const Vector& other) {
    if (this != &other) {
      Vector tmp(other);
      swap(tmp);
    }
    return *this;
  }

  // O(1) strong
  Vector& operator=(Vector&& other) {
    Vector tmp(std::move(other));
    swap(tmp);
    return *this;
  }

  // O(N) nothrow
  ~Vector() noexcept {
    for (size_t i = 0; i < size_; i++) {
      data_[size_ - 1 - i].~T();
    }
    operator delete(data_, static_cast<std::align_val_t>(alignof(T)));
  }

  // O(1) nothrow
  Reference operator[](size_t index) {
    return data_[index];
  }

  // O(1) nothrow
  ConstReference operator[](size_t index) const {
    return data_[index];
  }

  // O(1) nothrow
  Pointer data() noexcept {
    return data_;
  }

  // O(1) nothrow
  ConstPointer data() const noexcept {
    return data_;
  }

  // O(1) nothrow
  size_t size() const noexcept {
    return size_;
  }

  // O(1) nothrow
  Reference front() {
    return *data_;
  }

  // O(1) nothrow
  ConstReference front() const {
    return *data_;
  }

  // O(1) nothrow
  Reference back() {
    return *(data_ + size_ - 1);
  }

  // O(1) nothrow
  ConstReference back() const {
    return *(data_ + size_ - 1);
  }

  void push_back(T&& value) {
    push_back_impl(std::move(value));
  }

  void push_back(const T& value) {
    push_back_impl(value);
  }

  // O(1) nothrow
  void pop_back() {
    erase(end() - 1, end());
  }

  // O(1) nothrow
  bool empty() const noexcept {
    return size_ == 0;
  }

  // O(1) nothrow
  size_t capacity() const noexcept {
    return capacity_;
  }

  // O(N) strong
  void reserve(size_t new_capacity) {
    if (capacity_ < new_capacity) {
      change_capacity(new_capacity);
    }
  }

  // O(N) strong
  void shrink_to_fit() {
    if (capacity_ != size_) {
      change_capacity(size_);
    }
  }

  void change_capacity(size_t new_capacity) {
    Vector tmp(new_capacity);
    for (size_t i = 0; i < size_; i++) {
      tmp.push_back(std::move_if_noexcept(data_[i]));
    }
    swap(tmp);
  }

  // O(N) nothrow
  void clear() noexcept {
    for (size_t i = 0; i < size_; i++) {
      data_[size_ - 1 - i].~T();
    }
    size_ = 0;
  }

  // O(1) nothrow
  void swap(Vector& other) noexcept {
    std::swap(other.size_, size_);
    std::swap(other.capacity_, capacity_);
    std::swap(other.data_, data_);
  }

  // O(1) nothrow
  Iterator begin() noexcept {
    return data_;
  }

  // O(1) nothrow
  Iterator end() noexcept {
    return data_ + size_;
  }

  // O(1) nothrow
  ConstIterator begin() const noexcept {
    return data_;
  }

  // O(1) nothrow
  ConstIterator end() const noexcept {
    return data_ + size_;
  }

  // O(N) strong
  Iterator insert(ConstIterator pos, const T& value) {
    return insert_impl(pos, value);
  }

  // O(N) strong
  Iterator insert(ConstIterator pos, T&& value) {
    return insert_impl(pos, std::move(value));
  }

  // O(N) nothrow(swap)
  Iterator erase(ConstIterator pos) {
    return erase(pos, pos + 1);
  }

  // O(N) nothrow(swap)
  Iterator erase(ConstIterator first, ConstIterator last) {
    size_t begin1 = first - begin();
    ptrdiff_t length = last - first;
    if (length == 0) {
      return begin() + begin1;
    }
    for (size_t i = begin1; i + length < size(); i++) {
      std::swap(data_[i], data_[i + length]);
    }
    for (ptrdiff_t i = 0; i < length; i++) {
      data_[size_ - i - 1].~T();
    }
    size_ -= length;
    return begin() + begin1;
  }
};

} // namespace ct
