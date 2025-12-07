#pragma once
#include "constant.h"

#include <cstddef>
#include <cstdint>

namespace ct {
class BitSet;

template <typename U>
class BitsetIterator;

template <typename T>
class BitsetReference {
  T* pointer;
  std::size_t shift;

public:
  ~BitsetReference() = default;

private:
  BitsetReference(T* data, std::size_t newshift)
      : pointer(data + newshift / Word_size)
      , shift(newshift % Word_size) {}

public:
  friend BitSet;

  template <typename U>
  friend class BitsetIterator;

  template <typename U>
  friend class BitsetReference;

  BitsetReference& operator=(const bool other) {
    (*pointer) &= (~(ONE << shift));
    (*pointer) |= static_cast<Word>(other) << shift;
    return *this;
  }

  BitsetReference& operator=(const Word other) {
    (*pointer) &= (~(ONE << shift));
    (*pointer) |= other << shift;
    return *this;
  }

  BitsetReference& operator=(const int other) {
    (*pointer) &= (~(ONE << shift));
    (*pointer) |= other << shift;
    return *this;
  }

  operator BitsetReference<const T>() {
    return BitsetReference<const T>(pointer, shift);
  }

  operator bool() const {
    return ((*pointer) >> shift) & 1;
  }

  void flip() const {
    (*pointer) ^= (ONE << shift);
  }
};
} // namespace ct
