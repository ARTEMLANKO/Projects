#pragma once
// #include "bitset.h"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <iosfwd>

// #include "bitset-iterator.h"

namespace ct {

template <typename T>
class BitsetIterator;
// class BitSet;

template <typename T>
class BitsetView {
  BitsetIterator<T> _begin;
  BitsetIterator<T> _end;

public:
  template <typename U>
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

  friend void swap(BitsetView& lhs, BitsetView& rhs) {
    (lhs._begin).swap(rhs._begin);
    (lhs._end).swap(rhs._end);
  }

  BitsetView() = default;

  ~BitsetView() = default;

  BitsetView(const BitsetView& view) = default;

  operator BitsetView<const T>() const {
    return BitsetView<const T>(_begin, _end);
  }

private:
  BitsetView(T* data, std::size_t bit_index, std::size_t bit_count = -1)
      : _begin(BitsetIterator<T>(data, bit_index))
      , _end(BitsetIterator<T>(data, bit_index + bit_count)) {}

  static constexpr Word make_mask(size_t size) {
    return (size == Word_size) ? ~Word(0) : (1ULL << size) - 1;
  }

public:
  BitsetView(const BitsetIterator<T>& iter1, const BitsetIterator<T>& iter2)
      : _begin(iter1)
      , _end(iter2) {}

  BitsetView& operator=(const BitsetView& other) = default;

  template <typename U>
  BitsetView& operator=(const BitsetView<U>& other) {
    _begin = other._begin;
    _end = other._end;
    return *this;
  }

  BitsetReference<T> operator[](std::size_t index) const {
    return _begin[index];
  }

  BitsetIterator<T> begin() const {
    return _begin;
  }

  BitsetIterator<T> end() const {
    return _end;
  }

  bool all() const {
    if (size() == 0) {
      return true;
    }
    auto check_all = [](Word word, Word size) -> std::pair<size_t, bool> {
      size_t cnt = std::popcount(word);
      if (word != make_mask(size)) {
        return {0, false};
      }
      return {cnt, true};
    };
    return count(check_all) == size();
  }

  bool any() const {
    if (size() == 0) {
      return false;
    }
    auto check_any = [](Word word, Word) -> std::pair<size_t, bool> {
      if (word > 0) {
        return {1, false};
      }
      return {0, true};
    };
    return count(check_any) > 0;
  }

  size_t count() const {
    auto check_count = [](Word word, Word) -> std::pair<size_t, bool> {
      return {std::popcount(word), true};
    };
    return count(check_count);
  }

  std::size_t size() const {
    return _end - _begin;
  }

  bool empty() const {
    return size() == 0;
  }

private:
  template <typename Func>
  size_t count(Func check) const {
    size_t sz = size();
    size_t ans = 0;
    BitsetIterator<T> iter = begin();
    if (sz == 0) {
      return 0;
    }
    Word tmp = Word_size - iter.shift % Word_size;
    tmp = std::min(tmp, sz);
    Word c = iter.read_next_bits(tmp);
    auto [tmp_ans, need_continue] = check(c, tmp);
    ans += tmp_ans;
    if (!need_continue) {
      return ans;
    }
    iter += tmp;
    sz -= tmp;

    while (sz >= Word_size) {
      c = iter.read_next_bits(Word_size);
      auto [tmp_ans_1, need_continue_1] = check(c, Word_size);
      ans += tmp_ans_1;
      if (!need_continue_1) {
        return ans;
      }
      iter += Word_size;
      sz -= Word_size;
    }
    if (sz > 0) {
      c = iter.read_next_bits(sz);
      auto [tmp_ans_2, need_continue_2] = check(c, sz);
      ans += tmp_ans_2;
    }
    return ans;
  }

public:
  void swap(BitsetView& other) {
    using std::swap;
    swap(_begin, other._begin);
    swap(_end, other._end);
  }

  BitsetView subview(std::size_t offset = 0, std::size_t count = -1) const {
    if (offset > size()) {
      return {end(), end()};
    }
    BitsetIterator<T> iter1 = begin() + offset;
    BitsetIterator<T> iter2;
    if (count > size() || offset + count > size()) {
      iter2 = end();
    } else {
      iter2 = begin() + offset + count;
    }
    return {iter1, iter2};
  }

  const BitsetView& set() const {
    auto set_tmp = [](Word mask, Word) {
      return mask;
    };
    return set_reset_flip(set_tmp);
  }

  const BitsetView& flip() const {
    auto flip_tmp = [](Word mask, Word word) {
      return mask ^ word;
    };
    return set_reset_flip(flip_tmp);
  }

  const BitsetView& reset() const {
    auto reset_tmp = [](Word, Word) {
      return ZERO;
    };
    return set_reset_flip(reset_tmp);
  }

private:
  template <typename Oper>
  const BitsetView& set_reset_flip(Oper op) const {
    size_t sz = size();
    BitsetIterator<T> iter = begin();
    size_t tmp = Word_size - iter.shift % Word_size;
    tmp = std::min(tmp, sz);
    Word mask = make_mask(tmp);
    iter.write_next_bits(op(mask, iter.read_next_bits(tmp)), tmp);
    iter += tmp;
    sz -= tmp;
    while (sz >= Word_size) {
      mask = make_mask(Word_size);
      iter.write_next_bits(op(mask, iter.read_next_bits(Word_size)), Word_size);
      iter += Word_size;
      sz -= Word_size;
    }
    mask = make_mask(sz);
    iter.write_next_bits(op(mask, iter.read_next_bits(sz)), sz);
    return *this;
  }

public:
  template <typename U>
  const BitsetView& operator&=(const BitsetView<U>& view) const {
    BitSet other(view);
    return and_or_xor_assignment(*this, other, std::bit_and<Word>{});
  }

  template <typename U>
  const BitsetView& operator|=(const BitsetView<U>& view) const {
    BitSet other(view);
    return and_or_xor_assignment(*this, other, std::bit_or<Word>{});
  }

  template <typename U>
  const BitsetView& operator^=(const BitsetView<U>& view) const {
    BitSet other(view);
    return and_or_xor_assignment(*this, other, std::bit_xor<Word>{});
  }

  template <typename V>
  friend BitSet operator&(const BitsetView<T>& lhs, const BitsetView<V>& right) {
    BitSet ans(lhs);
    ans &= right;
    return ans;
  }

  template <typename V>
  friend BitSet operator^(const BitsetView<T>& lhs, const BitsetView<V>& right) {
    BitSet ans(lhs);
    ans ^= right;
    return ans;
  }

  template <typename V>
  friend BitSet operator|(const BitsetView<T>& lhs, const BitsetView<V>& right) {
    BitSet ans(lhs);
    ans |= right;
    return ans;
  }

  friend const BitsetView& operator&=(const BitsetView& lhs, const BitSet& right) {
    return and_or_xor_assignment(lhs, right, std::bit_and<Word>{});
  }

  friend const BitsetView& operator|=(const BitsetView& lhs, const BitSet& right) {
    return and_or_xor_assignment(lhs, right, std::bit_or<Word>{});
  }

  friend const BitsetView& operator^=(const BitsetView& lhs, const BitSet& right) {
    return and_or_xor_assignment(lhs, right, std::bit_xor<Word>{});
  }

private:
  template <typename BinaryOperation>
  friend const BitsetView& and_or_xor_assignment(const BitsetView& lhs, const BitSet& right, BinaryOperation op) {
    size_t sz = lhs.size();
    BitsetIterator<T> iter2 = lhs.begin();
    BitsetIterator<const Word> iter3 = right.begin();
    while (sz >= Word_size) {
      iter2.write_next_bits(op(iter2.read_next_bits(Word_size), iter3.read_next_bits(Word_size)), Word_size);
      iter2 += Word_size;
      iter3 += Word_size;
      sz -= Word_size;
    }
    iter2.write_next_bits(op(iter2.read_next_bits(sz), iter3.read_next_bits(sz)), sz);
    return lhs;
  }

public:
  friend BitSet operator>>(const BitsetView& lhs, std::size_t count) {
    BitSet ans(lhs);
    ans >>= count;
    return ans;
  }

  friend BitSet operator<<(const BitsetView& lhs, std::size_t count) {
    BitSet ans(lhs);
    ans <<= count;
    return ans;
  }

  friend std::string to_string(const BitsetView& bs) {
    std::string ans;
    for (std::size_t i = 0; i < bs.size(); i++) {
      ans += bs[i] ? '1' : '0';
    }
    return ans;
  }

  friend std::ostream& operator<<(std::ostream& out, const BitsetView<T>& bs) {
    for (size_t i = 0; i < bs.size(); i++) {
      out << (bs[i] ? '1' : '0');
    }
    return out;
  }
};

inline bool operator==(const BitsetView<const Word>& left, const BitsetView<const Word>& right) {
  if (left.size() != right.size()) {
    return false;
  }
  BitsetIterator<const Word> iter1 = left.begin();
  BitsetIterator<const Word> iter2 = right.begin();
  size_t sz = left.size();
  if (iter1.shift % 64 == iter2.shift % 64) {
    size_t tmp = Word_size - iter2.shift % Word_size;
    tmp = std::min(tmp, sz);
    if (iter1.read_next_bits(tmp) != iter2.read_next_bits(tmp)) {
      return false;
    }
    iter2 += tmp;
    iter1 += tmp;
    sz -= tmp;
  }
  while (sz >= Word_size) {
    if (iter1.read_next_bits(Word_size) != iter2.read_next_bits(Word_size)) {
      return false;
    }
    iter2 += Word_size;
    iter1 += Word_size;
    sz -= Word_size;
  }
  if (iter1.read_next_bits(sz) != iter2.read_next_bits(sz)) {
    return false;
  }
  return true;
}

inline BitSet operator~(const BitsetView<const Word>& lhs) {
  BitSet ans(lhs.size(), 0);
  size_t sz = lhs.size();
  BitsetIterator<Word> iter2 = ans.begin();
  BitsetIterator<const Word> iter3 = lhs.begin();
  while (sz >= Word_size) {
    iter2.write_next_bits(~(iter3.read_next_bits(Word_size)), Word_size);
    iter2 += Word_size;
    iter3 += Word_size;
    sz -= Word_size;
  }
  iter2.write_next_bits(~iter3.read_next_bits(sz), sz);
  return ans;
}
} // namespace ct
