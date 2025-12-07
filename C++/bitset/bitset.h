#pragma once

#include "bitset-iterator.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace ct {

// template <typename T>
// class BitsetView;

class BitSet {
  std::size_t size_;
  uint64_t* data_;

public:
  using Word = uint64_t;
  using Value = bool;
  using Reference = BitsetReference<Word>;
  using ConstReference = BitsetReference<const Word>;
  using Iterator = BitsetIterator<Word>;
  using ConstIterator = BitsetIterator<const Word>;
  using View = BitsetView<Word>;
  using ConstView = BitsetView<const Word>;

  using iterator = Iterator;
  using const_iterator = ConstIterator;
  using value_type = bool;
  using reference = Reference;
  using const_reference = ConstReference;
  using difference_type = std::ptrdiff_t;
  static constexpr std::size_t NPOS = -1;

  BitSet();
  ~BitSet();

  BitSet(std::size_t size, bool value = false);

  explicit BitSet(std::string_view str);

  explicit BitSet(const ConstView& other);

  BitSet(ConstIterator begin, ConstIterator end);
  BitSet& operator=(const BitSet& other);
  BitSet& operator=(std::string_view str);

  BitSet& operator=(const ConstView& other);
  BitSet(const BitSet& other);

  void swap(BitSet& other);
  std::size_t size() const;

  bool empty() const;

  Iterator begin();

  ConstIterator begin() const;
  Iterator end();

  ConstIterator end() const;

  BitSet& operator<<=(std::size_t count);

  BitSet& operator>>=(std::size_t count);

  BitSet& flip();
  const BitSet& flip() const;

  BitSet& set();
  const BitSet& set() const;

  BitSet& reset();
  const BitSet& reset() const;

  bool all() const;

  bool any() const;
  std::size_t count() const;

  operator ConstView() const;

  operator View();

  View subview(std::size_t offset = 0, std::size_t count = -1);

  ConstView subview(std::size_t offset = 0, std::size_t count = -1) const;
  BitsetReference<Word> operator[](std::size_t index);
  ConstReference operator[](std::size_t index) const;

  template <typename U>
  BitSet& operator&=(const BitsetView<U>& other) {
    for (std::size_t i = 0; i < size_; i++) {
      (*this)[i] = (*this)[i] && other[i];
    }
    return *this;
  }

  template <typename U>
  BitSet& operator|=(const BitsetView<U>& other) {
    for (std::size_t i = 0; i < size_; i++) {
      (*this)[i] = (*this)[i] || other[i];
    }
    return *this;
  }

  BitSet& operator^=(const ConstView& other);

  BitSet& operator&=(const BitSet& other);

  BitSet& operator|=(const BitSet& other);

  BitSet operator<<(std::size_t count) const;

  BitSet operator>>(std::size_t count) const;
  friend void swap(BitSet& lhs, BitSet& rhs);
};

bool operator!=(const BitSet& left, const BitSet& right);

void swap(BitSet& lhs, BitSet& rhs);

BitSet operator^(const BitSet& lhs, const BitSet& rhs);

BitSet operator|(const BitSet& lhs, const BitSet& rhs);

BitSet operator&(const BitSet& lhs, const BitSet& rhs);

std::string to_string(const BitSet& bs);

std::ostream& operator<<(std::ostream& out, const BitSet& bs);

template <typename U>
bool operator!=(const BitsetView<U>& lhs, const BitSet& rhs) {
  return rhs != lhs;
}

template <typename U>
bool operator!=(const BitSet& lhs, const BitsetView<U>& rhs) {
  BitSet cur(rhs);
  return cur != lhs;
}

} // namespace ct

#include "bitset-view.h"
