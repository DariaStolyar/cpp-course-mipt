#include <algorithm>
#include <cstring>
#include <iostream>

class String {
 private:
  char* array = nullptr;
  size_t size_ = 0;
  size_t capacity_ = 0;

 public:
  String()
      : array(new char[1]),
        size_(0),
        capacity_(1) {
    array[0] = '\0';
  }
  String(size_t count, char element)
      : array(new char[count + 1]),
        size_(count),
        capacity_(count + 1) {
    std::fill(array, array + count, element);
    array[count] = '\0';
  }
  String(const char* string);
  String(char element)
      : array(new char[1 + 1]),
        size_(1),
        capacity_(1 + 1) {
    std::fill(array, array + 1, element);
    array[1] = '\0';
  }
  String(const String& other)
      : array(new char[other.capacity_]),
        size_(other.size_),
        capacity_(other.capacity_) {
    memcpy(array, other.array, capacity_);
  }
  void swap(String& other);
  String& operator=(const String& other);
  String& operator+=(const String& other);
  char& operator[](size_t index);
  const char& operator[](size_t index) const;
  void change_capacity(size_t new_capacity);
  size_t length() const;
  size_t size() const;
  size_t capacity() const;
  void push_back(char element);
  void pop_back();
  char& front();
  char& back();
  const char& front() const;
  const char& back() const;
  bool empty() const;
  void clear();
  void shrink_to_fit();
  char* data();
  const char* data() const;
  String substr(size_t start, size_t count) const;
  bool is_substring(const String& substring, int index) const;
  size_t find(const String& substring) const;
  size_t rfind(const String& substring) const;
  ~String() {
    delete[] array;
  }
};
String::String(const char* string) {
  size_ = strlen(string);
  capacity_ = size_ + 1;
  array = new char[capacity_];
  memcpy(array, string, capacity_);
}
void String::swap(String& other) {
  std::swap(array, other.array);
  std::swap(size_, other.size_);
  std::swap(capacity_, other.capacity_);
}
String& String::operator=(const String& other) {
  if (this != &other) {
    String string(other);
    swap(string);
  }
  return *this;
}
String& String::operator+=(const String& other) {
  if (size_ + other.size_ + 1 >= capacity_) {
    change_capacity(2 * (capacity_ + other.capacity_ + 1));
  }
  std::copy(other.array, other.array + other.size_ + 1, array + size_);
  size_ += other.size_;
  return *this;
}
char& String::operator[](size_t index) {
  return array[index];
}
const char& String::operator[](size_t index) const {
  return array[index];
}
void String::change_capacity(size_t new_capacity) {
  char* new_array = new char[new_capacity];
  memcpy(new_array, array, size_ + 1);
  delete[] array;
  array = new_array;
  capacity_ = new_capacity;
}
size_t String::length() const {
  return size_;
}
size_t String::size() const {
  return size_;
}
size_t String::capacity() const {
  return capacity_ - 1;
}
void String::push_back(char element) {
  if (size_ + 2 >= capacity_) {
    change_capacity(2 * (capacity_ + 1));
  }
  array[size_] = element;
  array[size_ + 1] = '\0';
  size_ += 1;
}
void String::pop_back() {
  array[size_ - 1] = '\0';
  --size_;
}
char& String::front() {
  return array[0];
}
char& String::back() {
  return array[size_ - 1];
}
const char& String::front() const {
  return array[0];
}
const char& String::back() const {
  return array[size_ - 1];
}
bool String::empty() const {
  return size_ == 0;
}
void String::clear() {
  size_ = 0;
  array[0] = '\0';
}
void String::shrink_to_fit() {
  change_capacity(size_ + 1);
}
char* String::data() {
  return array;
}
const char* String::data() const {
  return array;
}
String String::substr(size_t start, size_t count) const {
  String string(count, ' ');
  for (size_t i = start; i < start + count; ++i) {
    string.array[i - start] = array[i];
  }
  return string;
}
bool String::is_substring(const String& substring, int index) const {
  for (size_t j = 0; j < substring.size_; ++j) {
    if (substring[j] != array[index + j]) {
      return false;
    }
  }
  return true;
}
size_t String::find(const String& substring) const {
  for (size_t i = 0; i < size_ - substring.length() + 1; ++i) {
    if (is_substring(substring, i)) {
      return i;
    }
  }
  return size_;
}
size_t String::rfind(const String& substring) const {
  for (int i = size_ - substring.length(); i >= 0; --i) {
    if (is_substring(substring, i)) {
      return i;
    }
  }
  return size_;
}
String operator+(const String& first, const String& second) {
  String result = first;
  result += second;
  return result;
}
bool operator<(const String& first, const String& second) {
  size_t min_length = std::min(first.length(), second.length());
  for (size_t i = 0; i < min_length; ++i) {
    if (first[i] > second[i]) {
      return false;
    }
  }
  return first.length() <= second.length();
}
bool operator>(const String& first, const String& second) {
  return second < first;
}
bool operator>=(const String& first, const String& second) {
  return !(first < second);
}
bool operator<=(const String& first, const String& second) {
  return !(second < first);
}
bool operator==(const String& first, const String& second) {
  if (first.size() != second.length()) {
    return false;
  }
  for (size_t i = 0; i < first.size(); ++i) {
    if (first.data()[i] != second[i]) {
      return false;
    }
  }
  return true;
}
bool operator!=(const String& first, const String& second) {
  return !(first == second);
}
std::ostream& operator<<(std::ostream& out, const String& string) {
  out << string.data();
  return out;
}
std::istream& operator>>(std::istream& in, String& string) {
  String new_string;
  char element;
  while (in.peek() == '\n') {
    in.get();
  }
  while (in.peek() != '\n' and in >> element) {
    new_string += element;
  }
  string = new_string;
  return in;
}
