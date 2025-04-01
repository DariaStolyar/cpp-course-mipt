#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

class BigInteger {
private:
    std::vector<int> array = {0};
    size_t length = 0;
    bool positive = true;
    int kBase = static_cast<int>(1e9);

    void sum_same_sign(const BigInteger& other);
    void sum_dif_sign(const BigInteger& other, int bigger);
    void change_length();
    size_t binary_search(BigInteger other, BigInteger number);
public:
    BigInteger() {}
    BigInteger(long long number);
    BigInteger(const BigInteger& other) = default;
    BigInteger(std::string string);

    BigInteger& operator=(const BigInteger& other) = default;
    explicit operator bool() const {
      return length != 0;
    }

    bool operator==(const BigInteger& other);
    bool operator!=(const BigInteger& other) {
      return !(*this == other);
    }
    bool operator<(const BigInteger& other) const;
    bool operator>(const BigInteger& other) const {
      return other < *this;
    }
    bool operator>=(const BigInteger& other) const {
      return !(*this < other);
    }
    bool operator<=(const BigInteger& other) const {
      return !(other < *this);
    }

    BigInteger& operator+=(const BigInteger& other);
    BigInteger& operator-=(const BigInteger& other);

    BigInteger& operator*=(const BigInteger& other);
    BigInteger operator*(const BigInteger& second) const;
    BigInteger& operator/=(const BigInteger& other);
    BigInteger operator/(const BigInteger& second) const;
    BigInteger& operator%=(const BigInteger& other);
    BigInteger operator%(const BigInteger& second) const;

    std::string toString() const;
    BigInteger operator-();
    BigInteger& operator++() {
      *this += 1;
      return *this;
    }
    BigInteger& operator--() {
      *this -= 1;
      return *this;
    }
    BigInteger operator++(int);
    BigInteger operator--(int);

    BigInteger operator+(const BigInteger& second) const;
    BigInteger operator-(const BigInteger& second) const;

    ~BigInteger() {}
};

void BigInteger::sum_same_sign(const BigInteger& other) {
  int change = 0;
  int new_number = 0;
  for (size_t i = 0; i < length; ++i) {
    if (i < other.length) {
      new_number = array[i] + other.array[i] + change;
    } else {
      new_number = array[i] + change;
    }
    array[i] = new_number % kBase;
    change = new_number / kBase;
  }
  size_t index = length;
  while (change != 0 or index < other.length) {
    if (array.size() <= index) {
      array.push_back(0);
    }
    if (index < other.length) {
      new_number = other.array[index] + change;
    } else {
      new_number = change;
    }
    array[index] = new_number % kBase;
    change = new_number / kBase;
    index += 1;
  }
  change_length();
}

void BigInteger::sum_dif_sign(const BigInteger& other, int bigger) {
  int new_number = 0;
  BigInteger big_array;
  BigInteger small_array;
  if (bigger == 0) {
    big_array = other;
    small_array = *this;
  } else {
    big_array = *this;
    small_array = other;
  }
  for (size_t i = 0; i < big_array.length; ++i) {
    if (i >= array.size()) {
      array.push_back(0);
    }
    if (i >= small_array.length) {
      new_number = big_array.array[i];
      array[i] = new_number % kBase;
      continue;
    }
    if ((big_array.array[i] - small_array.array[i]) < 0) {
      size_t index = i + 1;
      while (!big_array.array[index]) {
        big_array.array[index] = kBase - 1;
        ++index;
      }
      big_array.array[index] -= 1;
      big_array.array[i] += kBase;
    }
    new_number = big_array.array[i] - small_array.array[i];
    array[i] = new_number % kBase;
  }
  change_length();
}

void BigInteger::change_length() {
  length = 0;
  for (size_t i = 0; i < array.size(); ++i) {
    if (array[i] != 0) {
      length = i + 1;
    }
  }
}

size_t BigInteger::binary_search(BigInteger other, BigInteger number) {
  size_t start = 0;
  auto finish = static_cast<size_t>(1e18 + 1);
  while (start + 1 < finish) {
    size_t middle = start + ((finish - start) / 2);
    BigInteger hhh(static_cast<long long>(middle));
    if (hhh * other > number) {
      finish = middle;
    } else {
      start = middle;
    }
  }
  return start;
}

BigInteger::BigInteger(long long int number) {
  if (number < 0) {
    positive = false;
    number = -number;
  }
  array = {};
  if (number == 0) {
    array = {0};
    length = 0;
  }
  while (number != 0) {
    array.push_back(number % kBase);
    number /= kBase;
    length = array.size();
  }
}

BigInteger::BigInteger(std::string string) {
  int index = 0;
  if (string.size() == 0) {
    array = {0};
    length = 0;
    positive = true;
    return;
  }
  positive = true;
  if (string[0] == '-') {
    positive = false;
    index = 1;
  }
  std::string block;
  array = {};
  for (int i = string.size() - 1; i >= index; --i) {
    if ((string.size() - i - 1) % 9 == 0 and i != static_cast<int>(string.size()) - 1) {
      array.push_back(stoi(block));
      block = "";
    }
    block = string[i] + block;
  }
  if (block != "") {
    array.push_back(stoi(block));
  }
  length = array.size();
}

bool BigInteger::operator==(const BigInteger& other) {
  if (length != other.length) {
    return false;
  }
  for (size_t i = 0; i < length; ++i) {
    if (array[i] != other.array[i]) {
      return false;
    }
  }
  return true;
}

bool BigInteger::operator<(const BigInteger& other) const {
  if (positive and !other.positive) {
    return false;
  }
  if (!positive and other.positive) {
    return true;
  }
  if (length < other.length) {
    return positive;
  }
  if (length > other.length) {
    return !positive;
  }
  if (positive and other.positive) {
    for (int i = static_cast<int>(length) - 1; i >= 0; --i) {
      if (array[i] < other.array[i]) {
        return true;
      }
      if (array[i] > other.array[i]) {
        return false;
      }
    }
    return false;
  }
  for (int i = static_cast<int>(length) - 1; i >= 0; --i) {
    if (array[i] > other.array[i]) {
      return true;
    }
    if (array[i] < other.array[i]) {
      return false;
    }
  }
  return false;
}

BigInteger& BigInteger::operator+=(const BigInteger& other) {
  if ((positive and other.positive) or (!positive and !other.positive)) {
    sum_same_sign(other);
    return *this;
  }
  int bigger;
  if (positive) {
    positive = false;
    if (*this < other) {
      positive = true;
    }
    bigger = positive;
  } else {
    positive = true;
    if (*this > other) {
      positive = false;
    }
    bigger = !positive;
  }
  sum_dif_sign(other, bigger);
  return *this;
}

BigInteger& BigInteger::operator-=(const BigInteger& other) {
  if ((positive and other.positive) or (!positive and !other.positive)) {
    int bigger;
    if (positive) {
      bigger = *this > other;
    } else {
      bigger = !(*this > other);
    }
    if (!bigger) {
      positive = !positive;
    }
    sum_dif_sign(other, bigger);
  } else {
    sum_same_sign(other);
  }
  return *this;
}

BigInteger& BigInteger::operator*=(const BigInteger& other) {
  BigInteger result;
  for (size_t i = 0; i < length; ++i) {
    long long change = 0;
    long long new_number = 0;
    BigInteger result_i;
    for (size_t j = 0; j < other.length; ++j) {
      new_number = static_cast<long long>(array[i]) * static_cast<long long>(other.array[j]) + change;
      if (j >= result_i.array.size()) {
        result_i.array.push_back(0);
      }
      result_i.array[j] = static_cast<int>(new_number % kBase);
      change = new_number / kBase;
    }
    size_t index = other.length;
    while (change != 0) {
      if (result_i.array.size() <= index) {
        result_i.array.push_back(0);
      }
      new_number = change;
      result_i.array[index] = new_number % kBase;
      change = new_number / kBase;
      index += 1;
    }
    BigInteger result_i_with_zeros;
    result_i_with_zeros.array = {};
    for (size_t j = 0; j < i; ++j) {
      if (j >= result_i_with_zeros.array.size()) {
        result_i_with_zeros.array.push_back(0);
      }
    }
    for (size_t j = 0; j < result_i.array.size(); ++j) {
      result_i_with_zeros.array.push_back(result_i.array[j]);
    }
    result_i_with_zeros.length = result_i_with_zeros.array.size();
    result += result_i_with_zeros;
  }
  result.length = result.array.size();
  if ((positive and !other.positive) or (!positive and other.positive)) {
    result.positive = false;
  }
  *this = result;
  change_length();
  return *this;
}

BigInteger BigInteger::operator*(const BigInteger& second) const {
  BigInteger result(*this);
  result *= second;
  return result;
}

BigInteger& BigInteger::operator/=(const BigInteger& other) {
  std::vector<int> result;
  BigInteger number;
  BigInteger module_other(other);
  module_other.positive = true;
  bool new_positive = true;
  if ((positive and !other.positive) or (!positive and other.positive)) {
    new_positive = false;
  }
  positive = true;
  number.array.resize(std::min(other.length + 1, length));
  int index = length - 1;
  for (int i = std::min(other.length + 1, length) - 1; i >= 0; --i) {
    number.array[i] = array[index];
    --index;
  }
  number.length = number.array.size();
  bool stop = false;
  while (true) {
    while (number < module_other) {
      if (index >= 0) {
        result.push_back(0);
        --index;
      } else {
        if (number == 0) {
          result.push_back(0);
        }
        stop = true;
        break;
      }
    }
    if (stop) {
      break;
    }
    number.positive = true;
    long long half = binary_search(module_other, number);
    number -= static_cast<BigInteger>(half) * module_other;
    if (half >= kBase) {
      result.push_back(half / kBase);
    }
    result.push_back(half % kBase);
    if (index < 0) {
      break;
    }
    BigInteger new_number;
    new_number.array.resize(number.array.size() + 1);
    new_number.array[0] = array[index];
    --index;
    for (size_t i = 0; i < number.length; ++i) {
      new_number.array[i + 1] = number.array[i];
    }
    number = new_number;
    number.length = 0;
    for (size_t i = 0; i < number.array.size(); ++i) {
      if (number.array[i] != 0) {
        number.length = i + 1;
      }
    }
  }
  std::reverse(result.begin(), result.end());
  array = result;
  positive = new_positive;
  change_length();
  return *this;
}

BigInteger BigInteger::operator/(const BigInteger& second) const {
  BigInteger result(*this);
  result /= second;
  return result;
}

BigInteger& BigInteger::operator%=(const BigInteger& other) {
  BigInteger division = *this / other;
  division *= other;
  *this = *this - division;
  return *this;
}

BigInteger BigInteger::operator%(const BigInteger& second) const {
  BigInteger result(*this);
  result %= second;
  return result;
}

std::string BigInteger::toString() const {
  std::string BigInt;
  bool zero = true;
  for (size_t i = 0; i < length; ++i) {
    if (array[i] != 0) {
      zero = false;
    }
  }
  if (zero) {
    return "0";
  }
  if (!positive) {
    BigInt = "-";
  }
  int index = static_cast<int>(length) - 1;
  BigInt += std::to_string(array[index]);
  for (int i = index - 1; i >= 0; --i) {
    size_t len = 9 - std::to_string(array[i]).size();
    std::string zeros(len, '0');
    BigInt += zeros + std::to_string(array[i]);
  }
  return BigInt;
}

BigInteger BigInteger::operator-() {
  BigInteger bigint(*this);
  bigint.positive = !bigint.positive;
  return bigint;
}

BigInteger BigInteger::operator++(int) {
  BigInteger bigint(*this);
  *this += 1;
  return bigint;
}

BigInteger BigInteger::operator--(int) {
  BigInteger bigint(*this);
  *this -= 1;
  return bigint;
}

BigInteger BigInteger::operator+(const BigInteger& second) const {
  BigInteger result(*this);
  result += second;
  return result;
}

BigInteger BigInteger::operator-(const BigInteger& second) const {
  BigInteger result(*this);
  result -= second;
  return result;
}

BigInteger operator+(const int first, const BigInteger& second) {
  BigInteger result(first);
  result += second;
  return result;
}

BigInteger operator-(const int first, const BigInteger& second) {
  BigInteger result(first);
  result -= second;
  return result;
}

BigInteger operator*(const int first, const BigInteger& second) {
  BigInteger result(first);
  result *= second;
  return result;
}

BigInteger operator/(const int first, const BigInteger& second) {
  BigInteger result(first);
  result /= second;
  return result;
}

BigInteger operator%(const int first, const BigInteger& second) {
  BigInteger result(first);
  result %= second;
  return result;
}

std::ostream& operator<<(std::ostream& out, const BigInteger& bigint) {
  out << bigint.toString();
  return out;
}

std::istream& operator>>(std::istream& in, BigInteger& bigint) {
  std::string string_of_bigint;
  in >> string_of_bigint;
  BigInteger new_bigint(string_of_bigint);
  bigint = new_bigint;
  return in;
}

BigInteger operator""_bi(const char* string) {
  BigInteger bigint(string);
  return bigint;
}

class Rational {
private:
    BigInteger numerator = 0;
    BigInteger denominator = 1;
    void reduction();
public:
    Rational() {}
    Rational(long long number);
    Rational(BigInteger number) {
      numerator = number;
      denominator = 1;
    }
    Rational(const Rational& other) = default;

    Rational& operator=(const Rational& other) = default;

    Rational operator-();

    Rational& operator+=(const Rational& other);
    Rational& operator-=(const Rational& other);
    Rational& operator*=(const Rational& other);
    Rational& operator/=(const Rational& other);

    Rational operator+(const Rational& second) const;
    Rational operator-(const Rational& second) const;
    Rational operator*(const Rational& second) const;
    Rational operator/(const Rational& second) const;

    bool operator==(const Rational& other) {
      return numerator * other.denominator == other.numerator * denominator;
    }
    bool operator!=(const Rational& other) {
      return !(*this == other);
    }
    bool operator<(const Rational& other) const;
    bool operator>(const Rational& other) const {
      return other < *this;
    }
    bool operator>=(const Rational& other) const {
      return !(*this < other);
    }
    bool operator<=(const Rational& other) const {
      return !(other < *this);
    }

    std::string asDecimal(size_t precision = 0) const;
    std::string toString();
    
    explicit operator double() const {
      std::string string = asDecimal(20);
      return std::stod(string);
    }
};

void Rational::reduction() {
  BigInteger first(numerator);
  if (numerator < 0) {
    first = -first;
  }
  BigInteger second(denominator);
  while (first > 0 and second > 0) {
    if (first > second) {
      first %= second;
    } else {
      second %= first;
    }
  }
  BigInteger gcd(first + second);
  numerator /= gcd;
  denominator /= gcd;
}

Rational::Rational(long long int number) {
  BigInteger new_number(number);
  numerator = new_number;
  denominator = 1;
}

Rational Rational::operator-() {
  Rational rational(*this);
  rational.numerator = -rational.numerator;
  return rational;
}

Rational &Rational::operator+=(const Rational &other) {
  BigInteger new_numerator = numerator * other.denominator + other.numerator * denominator;
  BigInteger new_denomirnator = denominator * other.denominator;
  numerator = new_numerator;
  denominator = new_denomirnator;
  return *this;
}

Rational &Rational::operator-=(const Rational &other) {
  BigInteger new_numerator = numerator * other.denominator - other.numerator * denominator;
  BigInteger new_denomirnator = denominator * other.denominator;
  numerator = new_numerator;
  denominator = new_denomirnator;
  return *this;
}

Rational &Rational::operator*=(const Rational &other) {
  BigInteger new_numerator = numerator * other.numerator;
  BigInteger new_denomirnator = denominator * other.denominator;
  numerator = new_numerator;
  denominator = new_denomirnator;
  return *this;
}

Rational &Rational::operator/=(const Rational &other) {
  BigInteger new_numerator = numerator * other.denominator;
  BigInteger new_denomirnator = denominator * other.numerator;
  if (new_denomirnator < 0) {
    new_numerator = -new_numerator;
    new_denomirnator = -new_denomirnator;
  }
  numerator = new_numerator;
  denominator = new_denomirnator;
  return *this;
}

Rational Rational::operator+(const Rational &second) const {
  Rational result(*this);
  result += second;
  return result;
}

Rational Rational::operator-(const Rational &second) const {
  Rational result(*this);
  result -= second;
  return result;
}

Rational Rational::operator*(const Rational &second) const {
  Rational result(*this);
  result *= second;
  return result;
}

Rational Rational::operator/(const Rational &second) const {
  Rational result(*this);
  result /= second;
  return result;
}

bool Rational::operator<(const Rational &other) const {
  if (numerator > 0 and other.numerator < 0) {
    return false;
  }
  if (numerator < 0 and other.numerator > 0) {
    return true;
  }
  return numerator * other.denominator < other.numerator * denominator;
}

std::string Rational::asDecimal(size_t precision) const {
  BigInteger base(1e9);
  BigInteger ten(10);
  BigInteger rounded_mantissa(numerator);
  BigInteger new_numerator(numerator);
  for (size_t i = 0; i < precision; ++i) {
    new_numerator *= base;
    rounded_mantissa *= ten;
  }
  std::string string;
  BigInteger result = new_numerator / denominator;
  BigInteger result1 = numerator / denominator;
  if (precision == 0) {
    return result1.toString();
  }
  rounded_mantissa /= denominator;
  Rational res = rounded_mantissa;
  if (res == 0) {
    string = "0.";
    for (size_t i = 0; i < precision; ++i) {
      string += "0";
    }
    return string;
  }
  if (result != 0 and numerator < 0) {
    string = "-";
    result = -result;
    result1 = -result1;
  }
  if (result1 != 0) {
    for (size_t i = 0; i < result1.toString().size(); ++i) {
      string += result.toString()[i];
    }
    string += ".";
    for (size_t i = result1.toString().size(); i < std::min(result.toString().size(), precision + result1.toString().size()); ++i) {
      string += result.toString()[i];
    }
    if (result.toString().size() - result1.toString().size() < precision) {
      for (size_t i = result.toString().size() - result1.toString().size(); i < precision; ++i) {
        string += "0";
      }
    }
    return string;
  }
  BigInteger new_numerator_1(numerator);
  new_numerator_1 *= ten;
  size_t index = 0;
  string += "0.";
  while (new_numerator_1 / denominator == 0 and index < precision) {
    new_numerator_1 *= ten;
    string += "0";
    ++index;
  }
  for (size_t i = 0; i < precision - index; ++i) {
    string += result.toString()[i];
  }
  return string;
}

std::string Rational::toString() {
  std::string string;
  reduction();
  if (numerator == 0) {
    return "0";
  }
  if (denominator == 1) {
    return numerator.toString();
  }
  return numerator.toString() + "/" + denominator.toString();
}

Rational operator/(const int first, const Rational& second) {
  Rational result(first);
  result /= second;
  return result;
}
