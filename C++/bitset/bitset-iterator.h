#pragma once
#include "bitset-reference.h"
// #include "constant.h"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <ostream>

namespace ct {

template <typename T>
class BitsetView;

template <typename T>
class BitsetIterator {
  T* data;
  std::size_t shift;

public:
  template <typename V>
  friend class BitsetIterator;

  friend bool operator==(const BitsetView<const Word>&, const BitsetView<const Word>&);
  friend BitSet operator~(const BitsetView<const Word>&);

  template <typename V>
  friend class BitsetView;

  friend BitSet;
  using Value = bool;
  using Reference = BitsetReference<T>;
  using ConstReference = BitsetReference<const T>;
  using Iterator = BitsetIterator<T>;
  using ConstIterator = BitsetIterator<const T>;
  using View = BitsetView<T>;
  using ConstView = BitsetView<const T>;
  using Word = uint64_t;

  using iterator = Iterator;
  using const_iterator = ConstIterator;
  using value_type = bool;
  using reference = Reference;
  using const_reference = ConstReference;
  using difference_type = std::ptrdiff_t;

private:
  BitsetIterator(T* data_, std::size_t shift_)
      : data(data_)
      , shift(shift_) {}

public:
  BitsetIterator() = default;
  BitsetIterator(const Iterator& ref1) = default;

  operator BitsetIterator<const T>() const {
    return BitsetIterator<const T>(data, shift);
  }

  ~BitsetIterator() = default;

  using iterator_category = std::random_access_iterator_tag;

  void swap(BitsetIterator& other) noexcept {
    using std::swap;
    swap(data, other.data);
    swap(shift, other.shift);
  }

private:
  Word read_next_bits(std::size_t k) const {
    if (k == 0) {
      return 0ULL;
    }
    size_t start_word = shift / Word_size;
    size_t start_bit = shift % Word_size;
    size_t bits_first = std::min(k, Word_size - start_bit);
    size_t bits_second = k - bits_first;
    Word mask1 = (bits_first == Word_size) ? ~Word(0) : ((Word(1) << bits_first) - 1);
    Word part1 = (data[start_word] >> start_bit) & mask1;
    Word part2 = 0;
    if (bits_second > 0) {
      Word mask2 = (bits_second == Word_size) ? ~Word(0) : ((Word(1) << bits_second) - 1);
      part2 = data[start_word + 1] & mask2;
    }
    return (bits_first < Word_size) ? (part1 | (part2 << bits_first)) : part1;
  }

  void write_next_bits(Word value, size_t k) {
    if (k == 0) {
      return;
    }
    size_t start_word = shift / Word_size;
    size_t start_bit = shift % Word_size;
    size_t bits_first = std::min(k, Word_size - start_bit);
    size_t bits_second = k - bits_first;
    Word mask1 = (bits_first == Word_size) ? ~0ULL : (1ULL << bits_first) - 1;
    data[start_word] &= ~(mask1 << start_bit);
    data[start_word] |= (value & mask1) << start_bit;
    if (bits_second > 0) {
      Word mask2 = (bits_second == Word_size) ? ~0ULL : (1ULL << bits_second) - 1;
      data[start_word + 1] &= ~mask2;
      data[start_word + 1] |= (value >> bits_first) & mask2;
    }
  }

public:
  BitsetIterator& operator+=(std::size_t diff) {
    shift += diff;
    return *this;
  }

  BitsetIterator& operator-=(std::size_t diff) {
    shift -= diff;
    return *this;
  }

  BitsetIterator operator++(int) {
    BitsetIterator iter1(*this);
    *this += 1;
    return iter1;
  }

  BitsetIterator& operator++() {
    *this += 1;
    return *this;
  }

  BitsetIterator& operator=(const Iterator& iter1) = default;

  template <typename U>
  BitsetIterator& operator=(const BitsetIterator<U>& iter1) {
    data = iter1.data;
    shift = iter1.shift;
    return *this;
  }

  template <typename V>
  difference_type operator-(const BitsetIterator<V>& iter1) const {
    return shift - iter1.shift;
  }

  BitsetIterator operator+(std::size_t diff) const {
    BitsetIterator iter1(*this);
    iter1 += diff;
    return iter1;
  }

  BitsetIterator operator-(std::size_t diff) const {
    BitsetIterator iter1(*this);
    iter1 -= diff;
    return iter1;
  }

  BitsetIterator operator--(int) {
    BitsetIterator iter1(*this);
    *this -= 1;
    return iter1;
  }

  BitsetIterator& operator--() {
    *this -= 1;
    return *this;
  }

  template <typename U>
  bool operator==(const BitsetIterator<U>& iter1) const {
    return iter1.shift == shift;
  }

  template <typename U>
  bool operator!=(const BitsetIterator<U>& iter1) const {
    return iter1.shift != shift;
  }

  BitsetReference<T> operator*() const {
    return {data + shift / Word_size, shift % Word_size};
  }

  BitsetReference<T> operator[](std::size_t diff) const {
    return {data + (shift + diff) / Word_size, (shift + diff) % Word_size};
  }

  template <typename U>
  auto operator<=>(const BitsetIterator<U>& iter1) const {
    return shift <=> iter1.shift;
  }

  friend BitsetIterator operator+(int diff, const BitsetIterator& iter1) {
    BitsetIterator iter2(iter1);
    iter2 += diff;
    return iter2;
  }
};

} // namespace ct
