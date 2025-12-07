#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <numeric>

namespace ct {

template <typename T>
class Matrix {
public:
  template <typename U>
  struct ColumnIterator {
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;
    using reference = U&;
    using pointer = U*;

    template <typename V>
    friend struct ColumnIterator;

    friend class Matrix;

    ColumnIterator() = default;
    ~ColumnIterator() = default;
    ColumnIterator(const ColumnIterator&) = default;

    operator ColumnIterator<const U>() const {
      return ColumnIterator<const U>(_data, _col, _cols);
    }

    ColumnIterator& operator=(const ColumnIterator& other) = default;

    reference operator*() const {
      return *(_data + _col * _cols);
    }

    pointer operator->() const {
      return _data + _col * _cols;
    }

    ColumnIterator& operator++() {
      _col += 1;
      return *this;
    }

    ColumnIterator operator++(int) {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    ColumnIterator& operator--() {
      _col--;
      return *this;
    }

    ColumnIterator operator--(int) {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    ColumnIterator operator+(difference_type n) const {
      ColumnIterator iter(*this);
      iter._col += n;
      return iter;
    }

    ColumnIterator operator-(difference_type n) const {
      ColumnIterator iter(*this);
      iter._col -= n;
      return iter;
    }

    ColumnIterator& operator+=(difference_type n) {
      _col += n;
      return *this;
    }

    ColumnIterator& operator-=(difference_type n) {
      _col -= n;
      return *this;
    }

    friend auto operator<=>(const ColumnIterator& lhs, const ColumnIterator& other) {
      return lhs._col <=> other._col;
    }

    reference operator[](difference_type n) const {
      return *(_data + (_col + n) * _cols);
    }

    template <typename V>
    difference_type operator-(const ColumnIterator<V>& iter) const {
      return (_col - iter._col);
    }

    friend bool operator==(const ColumnIterator& lhs, const ColumnIterator& rhs) {
      return lhs._data == rhs._data && lhs._col == rhs._col;
    }

    friend bool operator!=(const ColumnIterator& lhs, const ColumnIterator& rhs) {
      return !(lhs == rhs);
    }

    friend ColumnIterator operator+(difference_type n, const ColumnIterator& iter) {
      return iter + n;
    }

  private:
    ColumnIterator(pointer data_, size_t cur, size_t cnt)
        : _data(data_)
        , _col(cur)
        , _cols(cnt) {}

    U* _data;
    difference_type _col;
    difference_type _cols;
  };

  template <typename Iter>
  struct View {
  private:
    Iter begin_, end_;

  public:
    View(const View& other)
        : begin_(other.begin_)
        , end_(other.end_) {}

    const View& operator*=(T n) const {
      for (auto it = begin_; it != end_; it++) {
        *it *= n;
      }
      return *this;
    }

    View(Iter _begin, Iter _end)
        : begin_(_begin)
        , end_(_end) {}

    Iter begin() const {
      return begin_;
    }

    Iter end() const {
      return end_;
    }
  };

  using ValueType = T;

  using Reference = T&;
  using ConstReference = const T&;

  using Pointer = T*;
  using ConstPointer = const T*;

  using Iterator = Pointer;
  using ConstIterator = ConstPointer;

  using RowIterator = Pointer;
  using ConstRowIterator = ConstPointer;

  using ColIterator = ColumnIterator<T>;
  using ConstColIterator = ColumnIterator<const T>;

  using RowView = View<RowIterator>;
  using ConstRowView = View<ConstRowIterator>;

  using ColView = View<ColIterator>;
  using ConstColView = View<ConstColIterator>;

  template <typename U>
  friend struct ColumnIterator;

  Matrix()
      : rows_(0)
      , cols_(0)
      , data_(nullptr) {}

  Matrix(size_t rows, size_t cols)
      : Matrix() {
    rows_ = rows;
    cols_ = cols; // A delegating constructor cannot contain member initializers, поэтому присваиваю тут
    if (empty()) {
      rows_ = 0;
      cols_ = 0;
    }
    data_ = empty() ? nullptr : new T[rows * cols]();
  }

  template <size_t ROWS, size_t COLS>
  Matrix(const T (&init)[ROWS][COLS])
      : rows_(ROWS)
      , cols_(COLS) {
    if (ROWS * COLS != 0) {
      data_ = new T[ROWS * COLS];
    } else {
      data_ = nullptr;
    }
    for (size_t i = 0; i < ROWS; i++) {
      std::copy_n(init[i], COLS, data_ + i * COLS);
    }
  }

  Matrix(const Matrix& other)
      : rows_(other.rows_)
      , cols_(other.cols_) {
    if (!empty()) {
      data_ = new T[rows_ * cols_];
    } else {
      data_ = nullptr;
    }
    std::copy_n(other.data_, rows_ * cols_, data_);
  }

  Matrix& operator=(const Matrix& other) {
    if (&other != this) {
      Matrix tmp(other);
      swap(tmp);
    }
    return *this;
  }

  ~Matrix() {
    delete[] data_;
  }

  // Iterators

  Iterator begin() {
    return data();
  }

  ConstIterator begin() const {
    return data();
  }

  Iterator end() {
    return begin() + size();
  }

  ConstIterator end() const {
    return begin() + size();
  }

  RowIterator row_begin(size_t row) {
    return data() + row * cols();
  }

  ConstRowIterator row_begin(size_t row) const {
    return data() + row * cols();
  }

  RowIterator row_end(size_t row) {
    return row_begin(row) + cols();
  }

  ConstRowIterator row_end(size_t row) const {
    return row_begin(row) + cols();
  }

  ColIterator col_begin(size_t col) {
    return ColIterator(data() + col, 0, cols());
  }

  ConstColIterator col_begin(size_t col) const {
    return ConstColIterator(data_ + col, 0, cols());
  }

  ColIterator col_end(size_t col) {
    return ColIterator(data() + col, rows(), cols());
  }

  ConstColIterator col_end(size_t col) const {
    return ConstColIterator(data_ + col, rows(), cols());
  }

  // Views

  RowView row(size_t row) {
    return RowView(row_begin(row), row_end(row));
  }

  ConstRowView row(size_t row) const {
    return ConstRowView(row_begin(row), row_end(row));
  }

  ColView col(size_t col) {
    return ColView(col_begin(col), col_end(col));
  }

  ConstColView col(size_t col) const {
    return ConstColView(col_begin(col), col_end(col));
  }

  // Size

  size_t rows() const {
    return rows_;
  }

  size_t cols() const {
    return cols_;
  }

  size_t size() const {
    return rows() * cols();
  }

  bool empty() const {
    return size() == 0;
  }

  // Elements access

  Reference operator()(size_t row, size_t col) {
    return data_[row * cols() + col];
  }

  ConstReference operator()(size_t row, size_t col) const {
    return data_[row * cols() + col];
  }

  Pointer data() {
    return size() == 0 ? nullptr : data_;
  }

  ConstPointer data() const {
    return size() == 0 ? nullptr : data_;
  }

  // Comparison

  friend bool operator==(const Matrix& left, const Matrix& right) {
    return left.rows_ == right.rows_ && left.cols_ == right.cols_ &&
           std::equal(left.begin(), left.end(), right.begin());
  }

  friend bool operator!=(const Matrix& left, const Matrix& right) {
    return !(left == right);
  }

  // Arithmetic operations

  Matrix& operator+=(const Matrix& other) {
    std::transform(begin(), end(), other.begin(), begin(), std::plus<>{});
    return *this;
  }

  Matrix& operator-=(const Matrix& other) {
    std::transform(begin(), end(), other.begin(), begin(), std::minus<>{});
    return *this;
  }

  Matrix& operator*=(const Matrix& other) {
    Matrix tmp(rows(), other.cols());
    for (size_t i = 0; i < rows(); i++) {
      for (size_t j = 0; j < other.cols(); j++) {
        tmp(i, j) = std::inner_product((*this).row_begin(i), (*this).row_end(i), other.col_begin(j), T());
      }
    }
    swap(tmp);
    return *this;
  }

  Matrix& operator*=(ConstReference factor) {
    View view(begin(), end());
    view *= factor;
    return *this;
  }

  void swap(Matrix& other) {
    using std::swap;
    swap(other.rows_, rows_);
    swap(other.cols_, cols_);
    swap(other.data_, data_);
  }

  friend Matrix operator+(const Matrix& left, const Matrix& right) {
    Matrix tmp(left);
    tmp += right;
    return tmp;
  }

  friend Matrix operator-(const Matrix& left, const Matrix& right) {
    Matrix tmp(left);
    tmp -= right;
    return tmp;
  }

  friend Matrix operator*(const Matrix& left, const Matrix& right) {
    Matrix tmp(left.rows(), right.cols());
    for (size_t i = 0; i < left.rows(); i++) {
      for (size_t j = 0; j < right.cols(); j++) {
        tmp(i, j) = std::inner_product(left.row_begin(i), left.row_end(i), right.col_begin(j), T());
      }
    }
    return tmp;
  }

  friend Matrix operator*(const Matrix& left, ConstReference right) {
    Matrix tmp(left);
    tmp *= right;
    return tmp;
  }

  friend Matrix operator*(ConstReference left, const Matrix& right) {
    Matrix tmp(right);
    tmp *= left;
    return tmp;
  }

private:
  size_t rows_;
  size_t cols_;
  T* data_;
};

} // namespace ct
