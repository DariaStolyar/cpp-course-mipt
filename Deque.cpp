#include <algorithm>
#include <vector>

struct Index {
    Index() = default;
    explicit Index(const std::pair<long long, long long>& index): first(index.first), second(index.second) {};

    Index& operator+=(size_t number);
    Index& operator--();
    Index& operator-=(long long number);
    long long operator-(const Index& other) const;

    bool operator==(const Index& other) const;
    bool operator<(const Index& other) const;
    bool operator<=(const Index& other) const;

    static const long long size_block = 16;
    long long first;
    long long second;
};

Index& Index::operator+=(size_t number) {
  if (static_cast<size_t>(second) + number < static_cast<size_t>(size_block)) {
    second = number + second;
    return *this;
  }
  first = first + 1;
  number -= (size_block - second);
  first += number / size_block;
  second = number % size_block;
  return *this;
}

Index& Index::operator--() {
  if (second - 1 >= 0) {
    second -= 1;
    return *this;
  }
  first -= 1;
  second = size_block - 1;
  return *this;
}

Index& Index::operator-=(long long number) {
  if (second - number >= 0) {
    second -= number;
    return *this;
  }
  first -= 1;
  number -= second + 1;
  first -= number / size_block;
  second = size_block - number % size_block - 1;
  return *this;
}

long long Index::operator-(const Index& other) const {
  long long size = 0;
  if (first == other.first) {
    return second - other.second;
  }
  if (first - other.first - 1 > 0) {
    size = (first - other.first - 1) * size_block;
  }
  size += second + (size_block - other.second);
  return size;
}

bool Index::operator==(const Index& other) const {
  return second == other.second and first == other.first;
}

bool Index::operator<(const Index& other) const {
  return first < other.first or (first == other.first and second < other.second);
}

bool Index::operator<=(const Index& other) const {
  return !(other < *this);
}

template <typename T>
class Deque {
public:
    Deque(): index_first_({0, 0}), index_after_last_({0, 0}) {
      outer_array_.push_back(reinterpret_cast<T*>(new char[size_block_ * sizeof(T)]));
      change_count_blocks();
    }

    Deque(const Deque& other);
    explicit Deque(long long count);
    Deque(long long count, const T& element);

    void swap(Deque& other);
    Deque& operator=(const Deque& other);

    size_t size() const { return index_after_last_ - index_first_; }

    T& operator[](size_t index);
    const T& operator[](size_t index) const;
    T& at(size_t index);
    const T& at(size_t index) const;

    void push_back(const T& element);
    void push_front(const T& element);

    void pop_back() {
      --index_after_last_;
      (outer_array_[index_after_last_.first] + index_after_last_.second)->~T();
    }

    void pop_front() {
      (outer_array_[index_first_.first] + index_first_.second)->~T();
      index_first_ += 1;
    }

    template <bool isConst>
    struct common_iterator {
    public:
        common_iterator(std::conditional_t<isConst, const T*const*, T**> ptr_b,
                        long long index_i): ptr_block_(ptr_b), index_(index_i), ptr_(&((*ptr_block_)[index_])) {};

        operator common_iterator<true>() const { return common_iterator<true>(ptr_block_, index_); };

        std::conditional_t<isConst, const T&, T&> operator*() const {
          return *ptr_;
        }

        std::conditional_t<isConst, const T*, T*> operator->() const {
          return ptr_;
        }

        common_iterator& operator++() {
          if (index_ < size_block_ - 1) {
            ++index_;
            ptr_ = *ptr_block_ + index_;
            return *this;
          }
          ++ptr_block_;
          index_ = 0;
          ptr_ = *ptr_block_ + index_;
          return *this;
        }

        common_iterator& operator--() {
          if (index_ >= 1) {
            --index_;
            ptr_ = *ptr_block_ + index_;
            return *this;
          }
          --ptr_block_;
          index_ = size_block_ - 1;
          ptr_ = *ptr_block_ + index_;
          return *this;
        }

        common_iterator operator++(int) {
          common_iterator iter(*this);
          *this += 1;
          return iter;
        }

        common_iterator operator--(int) {
          common_iterator iter(*this);
          *this -= 1;
          return iter;
        }

        common_iterator& operator+=(long long number) {
          Index new_index({0, index_});
          if (number >= 0) {
            new_index += number;
          } else {
            new_index -= (-number);
          }
          ptr_block_ += new_index.first;
          index_ = new_index.second;
          ptr_ = *ptr_block_ + index_;
          return *this;
        }

        common_iterator& operator-=(long long number) {
          *this += (-number);
          return *this;
        }

        bool operator==(const common_iterator& other) const {
          return ptr_block_ == other.ptr_block_ and index_ == other.index_;
        }

        bool operator<(const common_iterator& other) const {
          return (*this - other) < 0;
        }

        bool operator<=(const common_iterator& other) const {
          return !(other < *this);
        }

        bool operator>(const common_iterator& other) const {
          return other < *this;
        }

        bool operator>=(const common_iterator& other) const {
          return !(*this < other);
        }

        bool operator!=(const common_iterator& other) const {
          return !(other == *this);
        }

        long long operator-(const common_iterator& other) const {
          return (ptr_block_ - other.ptr_block_) * size_block_ + index_ - other.index_;
        }

        common_iterator operator+(long long number) const {
          common_iterator result = *this;
          result += number;
          return result;
        }

        common_iterator operator-(long long number) const {
          common_iterator result = *this;
          result -= number;
          return result;
        }

        using value_type = std::conditional_t<isConst, const T, T>;
        using pointer = std::conditional_t<isConst, const T*, T*>;
        using reference = std::conditional_t<isConst, const T&, T&>;
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = long long;
    private:
        std::conditional_t<isConst, const T*const*, T**>  ptr_block_;
        long long index_;
        std::conditional_t<isConst, const T*, T*>  ptr_;
    };

    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<common_iterator<false>>;
    using const_reverse_iterator = std::reverse_iterator<common_iterator<true>>;

    iterator begin() {
      return iterator(&outer_array_[index_first_.first], index_first_.second);
    }

    const_iterator begin() const {
      return const_iterator(&outer_array_[index_first_.first], index_first_.second);
    }

    iterator end() {
      return iterator(&outer_array_[index_after_last_.first], index_after_last_.second);
    }

    const_iterator end() const {
      return const_iterator(&outer_array_[index_after_last_.first], index_after_last_.second);
    }

    const_iterator cbegin() const {
      return const_iterator(&outer_array_[index_first_.first], index_first_.second);
    }

    const_iterator cend() const {
      return const_iterator(&outer_array_[index_after_last_.first], index_after_last_.second);
    }

    reverse_iterator rbegin() {
      return reverse_iterator(iterator(&outer_array_[index_after_last_.first], index_after_last_.second));
    }

    const_reverse_iterator rbegin() const {
      return const_reverse_iterator(const_iterator(&outer_array_[index_after_last_.first], index_after_last_.second));
    }

    reverse_iterator rend() {
      return reverse_iterator(iterator(&outer_array_[index_first_.first], index_first_.second));
    }

    const_reverse_iterator rend() const {
      return const_reverse_iterator(const_iterator(&outer_array_[index_first_.first], index_first_.second));
    }

    const_reverse_iterator crbegin() const {
      return const_reverse_iterator(const_iterator(&outer_array_[index_after_last_.first], index_after_last_.second));
    }

    const_reverse_iterator crend() const {
      return const_reverse_iterator(const_iterator(&outer_array_[index_first_.first], index_first_.second));
    }

    void insert(iterator iter, const T& element);

    void erase(iterator iter);

    ~Deque();
private:
    static const long long size_block_ = 16;
    std::vector<T*> outer_array_;
    Index index_first_;
    Index index_after_last_;

    void change_count_blocks();
    void delete_all(long long index1, long long index2, bool create);
};

template <typename T>
Deque<T>::Deque(const Deque& other): index_first_(other.index_first_),
                                     index_after_last_(other.index_after_last_) {
  outer_array_.resize(other.outer_array_.size());
  long long i;
  Index j({index_first_.first, index_first_.second});
  try {
    i = 0;
    for (; i < static_cast<long long>(other.outer_array_.size()); ++i) {
      outer_array_[i] = reinterpret_cast<T*>(new char[size_block_ * sizeof(T)]);
    }
    for (; j < index_after_last_; j += 1) {
      new (outer_array_[j.first] + j.second) T(other.outer_array_[j.first][j.second]);
    }
  } catch(...) {
    Index new_index({index_first_.first, index_first_.second});
    for (; new_index < j; new_index += 1) {
      (outer_array_[new_index.first] + new_index.second)->~T();
    }
    for (int k = 0; k < i; ++k) {
      delete[] reinterpret_cast<char*>(outer_array_[k]);
    }
    throw;
  }
};

template <typename T>
Deque<T>::Deque(long long count): index_first_({0, 0}), index_after_last_({count / size_block_,
                                                                         count % size_block_})  {
  long long index1 = 0;
  long long index2 = 0;
  bool create;
  try {
    outer_array_.resize((count / size_block_) + 2);
    for (; index1 < (count / size_block_) + 2; ++index1) {
      create = false;
      outer_array_[index1] = reinterpret_cast<T*>(new char[size_block_ * sizeof(T)]);
      create = true;
      if (index1 < index_after_last_.first) {
        index2 = 0;
        for (; index2 < size_block_; ++index2) {
          new (outer_array_[index1] + index2) T();
        }
      } else if (index1 == index_after_last_.first) {
        index2 = 0;
        for (; index2 < index_after_last_.second; ++index2) {
          new (outer_array_[index1] + index2) T();
        }
      }
    }
  } catch(...) {
    delete_all(index1, index2, create);
    throw;
  }
  change_count_blocks();
}

template <typename T>
Deque<T>::Deque(long long count, const T& element): index_first_({0, 0}),
                                                    index_after_last_({count / size_block_,
                                                            count % size_block_}) {
  long long index1 = 0;
  long long index2 = 0;
  bool create;
  try {
    outer_array_.resize((count / size_block_) + 2);
    for (; index1 < (count / size_block_) + 2; ++index1) {
      create = false;
      outer_array_[index1] = reinterpret_cast<T*>(new char[size_block_ * sizeof(T)]);
      create = true;
      if (index1 < index_after_last_.first) {
        index2 = 0;
        for (; index2 < size_block_; ++index2) {
          new (outer_array_[index1] + index2) T(element);
        }
      } else if (index1 == index_after_last_.first) {
        index2 = 0;
        for (; index2 < index_after_last_.second; ++index2) {
          new (outer_array_[index1] + index2) T(element);
        }
      }
    }
  } catch(...) {
    delete_all(index1, index2, create);
    throw;
  }
  change_count_blocks();
}

template <typename T>
void Deque<T>::swap(Deque& other) {
  std::swap(outer_array_, other.outer_array_);
  std::swap(index_after_last_.first, other.index_after_last_.first);
  std::swap(index_after_last_.second, other.index_after_last_.second);
  std::swap(index_first_.first, other.index_first_.first);
  std::swap(index_first_.second, other.index_first_.second);
}

template <typename T>
Deque<T>& Deque<T>::operator=(const Deque& other) {
  if (this != &other) {
    Deque deque(other);
    swap(deque);
  }
  return *this;
}

template <typename T>
T& Deque<T>::operator[](size_t index) {
  Index ind_of_elem(index_first_);
  ind_of_elem += index;
  return outer_array_[ind_of_elem.first][ind_of_elem.second];
}

template <typename T>
const T& Deque<T>::operator[](size_t index) const {
  Index ind_of_elem(index_first_);
  ind_of_elem += index;
  return outer_array_[ind_of_elem.first][ind_of_elem.second];
}

template <typename T>
T& Deque<T>::at(size_t index) {
  Index ind_of_elem(index_first_);
  ind_of_elem += index;
  if (index_after_last_ <= ind_of_elem) {
    throw std::out_of_range("Error");
  }
  return outer_array_[ind_of_elem.first][ind_of_elem.second];
}

template <typename T>
const T& Deque<T>::at(size_t index) const {
  Index ind_of_elem(index_first_);
  ind_of_elem += index;
  if (index_after_last_ <= ind_of_elem) {
    throw std::out_of_range("Error");
  }
  return outer_array_[ind_of_elem.first][ind_of_elem.second];
}

template <typename T>
void Deque<T>::change_count_blocks() {
  std::vector<T*> new_outer_array(3 * outer_array_.size());
  long long i = 0;
  try {
    for (; i <= static_cast<long long>(outer_array_.size()); ++i) {
      new_outer_array[i] = reinterpret_cast<T*>(new char[size_block_ * sizeof(T)]);
    }
    for (;i <= 2 * static_cast<long long>(outer_array_.size()); ++i) {
      new_outer_array[i] = outer_array_[i - outer_array_.size() - 1];
    }
    for (; i < 3 * static_cast<long long>(outer_array_.size()); ++i) {
      new_outer_array[i] = reinterpret_cast<T*>(new char[size_block_ * sizeof(T)]);
    }
    index_first_.first += outer_array_.size() + 1;
    index_after_last_.first += outer_array_.size() + 1;
    outer_array_ = new_outer_array;
  } catch(...) {
    for (long long j = 0; j < i; ++j) {
      delete[] reinterpret_cast<char*>(new_outer_array[j]);
    }
    throw;
  }
}

template <typename T>
void Deque<T>::delete_all(long long index1, long long index2, bool create) {
  for (long long i = 0; i < index1; ++i) {
    for (size_t j = 0; j < size_block_; ++j) {
      (outer_array_[i] + j)->~T();
    }
    delete[] reinterpret_cast<char*>(outer_array_[i]);
  }
  for (long long j = 0; j < index2; ++j) {
    (outer_array_[index1] + j)->~T();
  }
  if (create) {
    delete[] reinterpret_cast<char*>(outer_array_[index1]);
  }
}

template <typename T>
void Deque<T>::push_back(const T& element) {
  new (outer_array_[index_after_last_.first] + index_after_last_.second) T(element);
  index_after_last_ += 1;
  if (index_after_last_.first == static_cast<long long>(outer_array_.size())) {
    change_count_blocks();
  }
}

template <typename T>
void Deque<T>::push_front(const T& element) {
  Index deleted(index_first_);
  --deleted;
  new (outer_array_[deleted.first] + deleted.second) T(element);
  --index_first_;
  if (index_first_.first == 0 and index_first_.second == 0) {
    change_count_blocks();
  }
}

template <typename T>
void Deque<T>::insert(iterator iter, const T& element) {
  T element_i = element;
  for (iterator i = iter; i < end(); ++i) {
    std::swap(element_i, *i);
  }
  new (outer_array_[index_after_last_.first] + index_after_last_.second) T(element_i);
  index_after_last_ += 1;
  if (index_after_last_.first == static_cast<long long>(outer_array_.size())) {
    change_count_blocks();
  }
}

template <typename T>
void Deque<T>::erase(iterator iter) {
  for (iterator i = iter; i < end() - 1; ++i) {
    *i = *(i + 1);
  }
  index_after_last_ -= 1;
  (outer_array_[index_after_last_.first] + index_after_last_.second)->~T();
}

template<typename T>
Deque<T>::~Deque() {
  if (index_first_.first == index_after_last_.first) {
    for (long long j = index_first_.second; j < index_after_last_.second; ++j) {
      (outer_array_[index_first_.first] + j)->~T();
    }
  }
  else {
    for (long long i = index_first_.first; i <= index_after_last_.first; ++i) {
      if (i == index_first_.first) {
        for (long long j = index_first_.second; j < size_block_; ++j) {
          (outer_array_[i] + j)->~T();
        }
      } else if (i == index_after_last_.first) {
        for (long long j = 0; j < index_after_last_.second; ++j) {
          (outer_array_[i] + j)->~T();
        }
      } else {
        for (long long j = 0; j < size_block_; ++j) {
          (outer_array_[i] + j)->~T();
        }
      }
    }
  }
  for (size_t i = 0; i < outer_array_.size(); ++i) {
    delete[] reinterpret_cast<char*>(outer_array_[i]);
  }
}
