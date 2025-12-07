#include "bitset.h"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <string_view>

namespace ct {

BitSet::~BitSet() {
  delete[] data_;
}

BitSet::BitSet(std::size_t size, bool value)
    : size_(size)
    , data_(size_ == 0 ? nullptr : new Word[(size + Word_size - 1) / Word_size]()) {
  if (value) {
    set();
  }
}

BitSet::BitSet(const BitSet& other)
    : BitSet(other.begin(), other.end()) {}

void BitSet::swap(BitSet& other) {
  using std::swap;
  std::swap(size_, other.size_);
  std::swap(data_, other.data_);
}

BitSet::BitSet()
    : size_(0)
    , data_(nullptr) {}

BitSet::BitSet(const ConstView& other)
    : BitSet(other.begin(), other.end()) {}

BitSet::BitSet(ConstIterator first, ConstIterator second)
    : size_(second - first)
    , data_(size_ == 0 ? nullptr : new Word[(size_ + Word_size - 1) / Word_size]()) {
  std::size_t i = 0;
  for (auto it = first; it < second; it++) {
    if (*it) {
      data_[i / Word_size] |= (ONE << (i % Word_size));
    }
    i++;
  }
}

BitSet& BitSet::operator=(const BitSet& other) {
  if (this == &other) {
    return *this;
  }
  BitSet another(other);
  swap(another);
  return *this;
}

BitSet& BitSet::operator=(std::string_view str) {
  BitSet tmp(str);
  swap(tmp);
  return *this;
}

BitSet& BitSet::operator=(const ConstView& other) {
  BitSet tmp(other);
  swap(tmp);
  return *this;
}

BitSet::BitSet(std::string_view str)
    : size_(str.size())
    , data_(size_ == 0 ? nullptr : new Word[(size_ + Word_size - 1) / Word_size]()) {
  for (std::size_t i = 0; i < size_; i++) {
    if (str[i] == '1') {
      data_[i / Word_size] |= (1ULL << (i % Word_size));
    }
  }
}

std::size_t BitSet::size() const {
  return size_;
}

bool BitSet::empty() const {
  return size_ == 0;
}

BitSet::Iterator BitSet::begin() {
  return Iterator(data_, 0);
}

BitSet::ConstIterator BitSet::begin() const {
  return ConstIterator(data_, 0);
}

BitSet::Iterator BitSet::end() {
  return Iterator(data_, size_);
}

BitSet::ConstIterator BitSet::end() const {
  return ConstIterator(data_, size_);
}

BitSet& BitSet::flip() {
  View cur(data_, 0, size_);
  cur.flip();
  return *this;
}

BitSet& BitSet::set() {
  View cur(data_, 0, size_);
  cur.set();
  return *this;
}

BitSet& BitSet::reset() {
  View cur(data_, 0, size_);
  cur.reset();
  return *this;
}

const BitSet& BitSet::flip() const {
  View cur(data_, 0, size_);
  cur.flip();
  return *this;
}

const BitSet& BitSet::set() const {
  View cur(data_, 0, size_);
  cur.set();
  return *this;
}

const BitSet& BitSet::reset() const {
  View cur(data_, 0, size_);
  cur.reset();
  return *this;
}

bool BitSet::all() const {
  View cur(data_, 0, size_);
  return cur.all();
}

bool BitSet::any() const {
  View cur(data_, 0, size_);
  return cur.any();
}

std::size_t BitSet::count() const {
  View cur(data_, 0, size_);
  return cur.count();
}

BitSet::operator ConstView() const {
  ConstView ans(data_, 0, size_);
  return ans;
}

BitSet::operator View() {
  View ans(data_, 0, size_);
  return ans;
}

BitSet::View BitSet::subview(std::size_t offset, std::size_t count) {
  View ans(begin(), end());
  return ans.subview(offset, count);
}

BitSet::ConstView BitSet::subview(std::size_t offset, std::size_t count) const {
  ConstView ans(begin(), end());
  return ans.subview(offset, count);
}

BitSet::Reference BitSet::operator[](std::size_t index) {
  return {data_ + index / Word_size, index % Word_size};
}

BitSet::ConstReference BitSet::operator[](std::size_t index) const {
  return {data_ + index / Word_size, index % Word_size};
}

BitSet& BitSet::operator&=(const BitSet& other) {
  static_cast<View>(*this) &= other;
  return *this;
}

BitSet& BitSet::operator|=(const BitSet& other) {
  static_cast<View>(*this) |= other;
  return *this;
}

BitSet& BitSet::operator^=(const ConstView& other) {
  static_cast<View>(*this) ^= other;
  return *this;
}

BitSet operator^(const BitSet& lhs, const BitSet& bs) {
  BitSet result = lhs;
  result ^= bs;
  return result;
}

BitSet operator|(const BitSet& lhs, const BitSet& bs) {
  BitSet result = lhs;
  result |= bs;
  return result;
}

BitSet operator&(const BitSet& lhs, const BitSet& bs) {
  BitSet result = lhs;
  result &= bs;
  return result;
}

BitSet& BitSet::operator<<=(std::size_t shift) {
  if (shift == 0) {
    return *this;
  }
  if ((size_ + shift + Word_size - 1) / Word_size != (size_ + Word_size - 1) / Word_size) {
    BitSet tmp(size_ + shift, false);
    std::copy_n(data_, (size_ + Word_size - 1) / Word_size, tmp.data_);
    swap(tmp);
  } else {
    size_ += shift;
  }
  return *this;
}

BitSet& BitSet::operator>>=(std::size_t shift) {
  if (shift == 0) {
    return *this;
  }
  if (size_ < shift) {
    BitSet tmp = BitSet(0, false);
    swap(tmp);
    return *this;
  }
  if ((size_ - shift + Word_size - 1) / Word_size != (size_ + Word_size - 1) / Word_size) {
    BitSet tmp(size_ - shift, false);
    std::copy_n(data_, (size_ - shift + Word_size - 1) / Word_size, tmp.data_);
    swap(tmp);
  } else {
    size_ -= shift;
  }
  size_t valid_bits = size_ % Word_size;
  if (valid_bits != 0) {
    Word mask = (Word(1) << valid_bits) - 1;
    data_[(size_ - 1) / Word_size] &= mask;
  }
  return *this;
}

BitSet BitSet::operator<<(std::size_t shift) const {
  BitSet ans = *this;
  ans <<= shift;
  return ans;
}

BitSet BitSet::operator>>(std::size_t shift) const {
  BitSet ans = *this;
  ans >>= shift;
  return ans;
}

bool operator!=(const BitSet& left, const BitSet& right) {
  return !(left == right);
}

void swap(BitSet& lhs, BitSet& rhs) {
  std::swap(lhs.data_, rhs.data_);
  std::swap(lhs.size_, rhs.size_);
}

std::string to_string(const BitSet& bs) {
  std::string ans;
  for (std::size_t i = 0; i < bs.size(); i++) {
    ans += bs[i] ? '1' : '0';
  }
  return ans;
}

std::ostream& operator<<(std::ostream& out, const BitSet& bs) {
  BitsetView view(bs.begin(), bs.end());
  return out << view;
}

} // namespace ct
