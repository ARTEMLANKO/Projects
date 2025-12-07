#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <type_traits>

namespace ct {

template <typename T, std::size_t SMALL_SIZE>
class SocowVector {
  static_assert(std::is_copy_constructible_v<T>, "T must have a copy constructor");
  static_assert(std::is_nothrow_move_constructible_v<T>, "T must have a non-throwing move constructor");
  static_assert(std::is_copy_assignable_v<T>, "T must have a copy assignment operator");
  static_assert(std::is_nothrow_move_assignable_v<T>, "T must have a non-throwing move assignment operator");
  static_assert(std::is_nothrow_swappable_v<T>, "T must have a non-throwing swap");

  static_assert(SMALL_SIZE > 0, "SMALL_SIZE must be positive");

  struct DynamicBuffer {
    size_t capacity;
    size_t ref_count;
    T data[0];

    explicit DynamicBuffer(size_t new_capacity)
        : capacity(new_capacity)
        , ref_count(1) {}
  };

  size_t size_;
  bool small;

  union Buffer {
    T small_buffer[SMALL_SIZE];
    DynamicBuffer* dynamic_buffer;

    Buffer() {}

    ~Buffer() {}
  } sb;

  void destroy_and_deallocate(DynamicBuffer* ptr, size_t size) {
    if (!ptr) {
      return;
    }
    std::destroy_n(ptr->data, size);
    operator delete(ptr, std::align_val_t{alignof(DynamicBuffer)});
  }

  bool big_and_not_unique() {
    return !is_small() && sb.dynamic_buffer->ref_count > 1;
  }

  bool big_and_unique() {
    return !is_small() && sb.dynamic_buffer->ref_count == 1;
  }

  void ensure_unique() {
    if (big_and_not_unique()) {
      SocowVector tmp;
      tmp.change_capacity(capacity());
      std::uninitialized_copy(sb.dynamic_buffer->data, sb.dynamic_buffer->data + size_, tmp.sb.dynamic_buffer->data);
      tmp.size_ = size_;
      swap(tmp);
    }
  }

  void change_capacity(size_t new_capacity) {
    if ((small && new_capacity == SMALL_SIZE) || (!small && sb.dynamic_buffer->capacity == new_capacity)) {
      return;
    }
    if (new_capacity <= SMALL_SIZE) {
      if (!is_small()) {
        DynamicBuffer* old = sb.dynamic_buffer;
        try {
          copy_in_change_capacity(old, sb.small_buffer);
        } catch (...) {
          sb.dynamic_buffer = old;
          throw;
        }
        small = true;
      }
    } else {
      DynamicBuffer* new_buffer = allocate_buffer(new_capacity);
      if (is_small()) {
        std::uninitialized_move(sb.small_buffer, sb.small_buffer + size_, new_buffer->data);
        std::destroy_n(sb.small_buffer, size_);
      } else {
        try {
          copy_in_change_capacity(sb.dynamic_buffer, new_buffer->data);
        } catch (...) {
          operator delete(new_buffer, std::align_val_t{alignof(DynamicBuffer)});
          throw;
        }
      }
      small = false;
      sb.dynamic_buffer = new_buffer;
    }
  }

  void copy_in_change_capacity(DynamicBuffer* buffer, T* second) {
    if (sb.dynamic_buffer->ref_count == 1) {
      std::uninitialized_move(buffer->data, buffer->data + size_, second);
      destroy_and_deallocate(buffer, size_);
    } else {
      std::uninitialized_copy(buffer->data, buffer->data + size_, second);
      buffer->ref_count -= 1;
    }
  }

  void ensure_capacity(size_t required_capacity) {
    if (required_capacity > capacity()) {
      size_t new_capacity = std::max(required_capacity, capacity() * 2);
      change_capacity(new_capacity);
    }
  }

  void copy_erase_range(T* original, T* final, size_t prefix, size_t del) {
    bool last = false;
    try {
      std::uninitialized_copy_n(original, prefix, final);
      last = true;
      std::uninitialized_copy_n(original + prefix + del, size_ - prefix - del, final + prefix);
    } catch (...) {
      if (last) {
        std::destroy_n(final, prefix);
      }
      throw;
    }
  }

public:
  using Iterator = T*;
  using ConstIterator = const T*;
  using Reference = T&;
  using ConstReference = const T&;
  using ValueType = T;
  using Pointer = T*;
  using ConstPointer = const T*;

  ~SocowVector() {
    if (!is_small()) {
      if (sb.dynamic_buffer->ref_count == 1) {
        destroy_and_deallocate(sb.dynamic_buffer, size_);
      } else {
        sb.dynamic_buffer->ref_count -= 1;
      }
    } else if (is_small()) {
      std::destroy_n(sb.small_buffer, size_);
    }
    small = true;
    size_ = 0;
  }

  size_t capacity() {
    return small ? SMALL_SIZE : sb.dynamic_buffer->capacity;
  }

  SocowVector()
      : size_(0)
      , small(true) {}

  bool is_small() const {
    return small;
  }

  SocowVector(const SocowVector& other)
      : SocowVector() {
    if (other.is_small()) {
      std::uninitialized_copy(other.sb.small_buffer, other.sb.small_buffer + other.size_, sb.small_buffer);
    } else {
      sb.dynamic_buffer = other.sb.dynamic_buffer;
      sb.dynamic_buffer->ref_count += 1;
    }
    size_ = other.size_;
    small = other.small;
  }

  SocowVector(SocowVector&& other) noexcept {
    size_ = other.size_;
    small = other.small;
    if (other.is_small()) {
      std::uninitialized_move(other.sb.small_buffer, other.sb.small_buffer + size_, sb.small_buffer);
      other.clear();
    } else {
      sb.dynamic_buffer = other.sb.dynamic_buffer;
      other.sb.dynamic_buffer = nullptr;
      other.size_ = 0;
      other.small = true;
    }
  }

  SocowVector& operator=(const SocowVector& other) {
    if (this != &other) {
      SocowVector tmp(other);
      clear();
      swap(tmp);
    }
    return *this;
  }

  SocowVector& operator=(SocowVector&& other) noexcept {
    if (this != &other) {
      clear();
      swap(other);
    }
    return *this;
  }

  DynamicBuffer* allocate_buffer(size_t capacity) {
    void* mem = operator new(sizeof(DynamicBuffer) + sizeof(T) * capacity, std::align_val_t{alignof(DynamicBuffer)});
    return new (mem) DynamicBuffer(capacity);
  }

  void swap(SocowVector& other) noexcept {
    if (&other == this) {
      return;
    }
    if (!is_small() && !other.is_small()) {
      std::swap(sb.dynamic_buffer, other.sb.dynamic_buffer);
    } else if (is_small() && other.is_small() && size() <= other.size()) {
      std::swap_ranges(sb.small_buffer, sb.small_buffer + size(), other.sb.small_buffer);
      std::uninitialized_move_n(other.begin() + size_, other.size_ - size_, begin() + size_);
      std::destroy_n(other.begin() + size_, other.size_ - size_);
    } else if (is_small() && !other.is_small()) {
      DynamicBuffer* tmp = other.sb.dynamic_buffer;
      std::uninitialized_move_n(sb.small_buffer, size_, other.sb.small_buffer);
      std::destroy_n(begin(), size_);
      sb.dynamic_buffer = tmp;
      // other.sb.dynamic_buffer = nullptr;
    }
    if ((is_small() && other.is_small() && size() > other.size()) || (!is_small() && other.is_small())) {
      other.swap(*this);
      return;
    }
    std::swap(size_, other.size_);
    std::swap(small, other.small);
  }

  size_t size() const {
    return size_;
  }

  size_t capacity() const {
    return small ? SMALL_SIZE : (sb.dynamic_buffer)->capacity;
  }

  bool empty() const {
    return size() == 0;
  }

  T& operator[](size_t index) {
    return begin()[index];
  }

  T* data() {
    ensure_unique();
    return small ? sb.small_buffer : sb.dynamic_buffer->data;
  }

  const T* data() const {
    return small ? sb.small_buffer : sb.dynamic_buffer->data;
  }

  T* const_begin() {
    return small ? sb.small_buffer : sb.dynamic_buffer->data;
  }

  T* const_end() {
    return small ? sb.small_buffer + size_ : sb.dynamic_buffer->data + size_;
  }

  const T* const_begin() const {
    return small ? sb.small_buffer : sb.dynamic_buffer->data;
  }

  const T* const_end() const {
    return small ? sb.small_buffer + size_ : sb.dynamic_buffer->data + size_;
  }

  T& front() {
    return *begin();
  }

  const T& front() const {
    return *begin();
  }

  T& back() {
    return *(begin() + size() - 1);
  }

  const T& back() const {
    return *(begin() + size() - 1);
  }

  const T& operator[](size_t index) const {
    return begin()[index];
  }

  T* begin() {
    return data();
  }

  const T* begin() const {
    return data();
  }

  T* end() {
    return data() + size();
  }

  const T* end() const {
    return data() + size();
  }

  size_t get_refcount() {
    return sb.dynamic_buffer->ref_count;
  }

  void push_back(T&& value) {
    push_back_impl(std::move(value));
  }

  void push_back(const T& value) {
    push_back_impl(value);
  }

  template <typename U>
  void push_back_impl(U&& value) {
    if ((is_small() && size_ < SMALL_SIZE) ||
        (!is_small() && sb.dynamic_buffer->ref_count == 1 && size_ < sb.dynamic_buffer->capacity)) {
      new (data() + size()) T(std::forward<U>(value));
      size_++;
      return;
    }
    size_t new_capacity = capacity() == 0 ? 1 : 2 * capacity();
    SocowVector tmp;
    tmp.change_capacity(new_capacity);
    if (big_and_unique() || is_small()) {
      // тут можно data(), а не const_begin(), бо в любом случае уникальный
      new (tmp.data() + size()) T(std::forward<U>(value));
      std::uninitialized_move_n(data(), size(), tmp.data());
    } else {
      std::uninitialized_copy_n(const_begin(), size(), tmp.const_begin());
      try {
        new (tmp.const_begin() + size()) T(std::forward<U>(value));
      } catch (...) {
        std::destroy_n(tmp.const_begin(), size());
        throw;
      }
    }
    tmp.size_ = size_ + 1;
    swap(tmp);
  }

  template <typename U>
  T* insert(const T* index, U&& value) {
    size_t insert_pos = index - const_begin();
    // ensure_unique();
    push_back(std::forward<U>(value));
    std::rotate(const_begin() + insert_pos, const_end() - 1, const_end());
    return const_begin() + insert_pos;
  }

  void pop_back() {
    erase(const_end() - 1, const_end());
  }

  T* erase(const T* first) {
    return erase(first, first + 1);
  }

  T* erase(const T* first, const T* last) {
    size_t length = last - first;
    if (length == 0) {
      return const_begin() + (first - const_begin());
    }
    size_t del = last - first;
    size_t new_size = size_ - del;
    size_t prefix = first - const_begin();
    if (big_and_not_unique()) {
      if (new_size <= SMALL_SIZE) {
        DynamicBuffer* tmp = sb.dynamic_buffer;
        try {
          copy_erase_range(tmp->data, sb.small_buffer, prefix, del);
        } catch (...) {
          sb.dynamic_buffer = tmp;
          throw;
        }
        tmp->ref_count--;
        small = true;
      } else {
        SocowVector tmp;
        tmp.change_capacity(new_size);
        copy_erase_range(sb.dynamic_buffer->data, tmp.data(), prefix, del);
        swap(tmp);
      }
      size_ = new_size;
      return const_begin() + prefix;
    }
    std::rotate(const_begin() + prefix, const_begin() + prefix + del, const_end());
    std::destroy_n(const_end() - del, del);
    size_ -= del;
    return const_begin() + prefix;
  }

  void clear() noexcept {
    if (is_small()) {
      std::destroy_n(sb.small_buffer, size_);
    } else {
      if (sb.dynamic_buffer->ref_count == 1) {
        std::destroy_n(sb.dynamic_buffer->data, size_);
      } else {
        sb.dynamic_buffer->ref_count -= 1;
        small = true;
      }
    }
    size_ = 0;
  }

  void reserve(size_t new_capacity) {
    if (new_capacity < size_) {
      return;
    }
    new_capacity = std::max(new_capacity, SMALL_SIZE);
    if (!is_small()) {
      if (sb.dynamic_buffer->ref_count == 1 && new_capacity <= capacity()) {
        return;
      }
    }
    if (new_capacity > capacity() || (new_capacity > size() && new_capacity < capacity())) {
      change_capacity(new_capacity);
    }
  }

  void shrink_to_fit() {
    if (!is_small() && size() != capacity()) {
      change_capacity(size_);
    }
  }
};

} // namespace ct
