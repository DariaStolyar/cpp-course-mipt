#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

namespace detail
{
  bool CompareDouble(double first, double second) {
    return std::fabs(first - second) < 1e-5;
  }

  bool CompareVector(std::vector<double> first, std::vector<double> second) {
    if (first.size() != second.size()) {
      return false;
    }
    for (size_t i = 0; i < first.size(); ++i) {
      if (!CompareDouble(first[i], second[i])) {
        return false;
      }
    }
    return true;
  }
}
using namespace detail;

struct Point {
    double x;
    double y;

    Point() {
      x = 0;
      y = 0;
    }
    Point(double new_x, double new_y): x(new_x),  y(new_y) {};

    bool operator==(Point point) const {
      return CompareDouble(x, point.x) and CompareDouble(y, point.y);
    }
    bool operator!=(Point point) const {
      return !(*this == point);
    }

    void rotate(const Point& center, double angle);
    void reflect(const Point& center);
};

void Point::rotate(const Point& center, double angle) {
  double angle_cos = std::cos(angle * M_PI / 180.0);
  double angle_sin = std::sin(angle * M_PI / 180.0);
  double delta_x = x - center.x;
  double delta_y = y - center.y;
  x = delta_x * angle_cos - delta_y * angle_sin + center.x;
  y =  delta_x * angle_sin + delta_y * angle_cos + center.y;
}

void Point::reflect(const Point& center) {
  x = 2 * (center.x - x) + x;
  y = 2 * (center.y - y) + y;
}

class Line {
public:
    Point point1;
    Point point2;

    Line() = default;
    Line(Point first, Point second): point1(first), point2(second) {};
    Line(double k, double b): point1(Point(0, b)), point2(Point(1, k + b)) {};
    Line(Point point, double k): point1(point), point2(Point(point.x + 1, point.y + k)) {};

    std::vector<double> Convert() const;
    std::vector<Point> Points() const {
      return {point1, point2};
    }

    bool operator==(Line line) const;
    bool operator!=(Line line) const {
      return !(*this == line);
    }
};

std::vector<double> Line::Convert() const {
  if (CompareDouble(point1.x,  point2.x)) {
    return {-1, 0, point1.x};
  } else {
    double k = (point2.y - point1.y) / (point2.x - point1.x);
    return {1, k, point1.y - k * point1.x};
  }
}

bool Line::operator==(Line line) const {
  std::vector<double> convert1 = Convert();
  std::vector<double> convert2 = line.Convert();
  if (CompareVector(convert1, convert2)) {
    return true;
  }
  return false;
}

struct Vector {
    double x;
    double y;

    Vector(): x(0.0), y(0.0) {};
    Vector(const Point first): x(first.x), y(first.y) {};
    Vector(const Point first, const Point second): x(second.x - first.x), y(second.y - first.y) {};

    double length() const {
      return std::pow(x * x + y * y, 0.5);
    }
    double length2() const {
      return x * x + y * y;
    }
    double cos(Vector vector) const {
      return (x * vector.x + y * vector.y) / (length() * vector.length());
    }
    void operator*=(double number);
    double scalar_product(Vector vector) const;
    double angle(Vector vector) const;

};
void Vector::operator*=(double number) {
  x *= number;
  y *= number;
}

double Vector::scalar_product(Vector vector) const {
  return x * vector.x + y * vector.y;
}

double Vector::angle(Vector vector) const {
  return std::acos(scalar_product(vector) / (length() * vector.length()));
}

class Shape {
public:
    Shape() = default;

    virtual double area() const = 0;
    virtual double perimeter() const = 0;
    virtual bool operator==(const Shape& another) const = 0;
    virtual bool operator!=(const Shape& another) const = 0;
    virtual bool isCongruentTo(const Shape& another) const = 0;
    virtual bool isSimilarTo(const Shape& another) const = 0;
    virtual bool containsPoint(const Point& point) const = 0;

    virtual void rotate(const Point& center, double angle) = 0;
    virtual void reflect(const Point& center) = 0;
    virtual void reflect(const Line& axis) = 0;
    virtual void scale(const Point& center, double coefficient) = 0;

    virtual ~Shape() = default;
};

class Polygon: public Shape {
public:
    std::vector<Point> points;

    Polygon(std::vector<Point>& new_points) {
      points = new_points;
    };
    Polygon(Polygon& polygon) {
      points = polygon.points;
    }
    template<typename... point>
    Polygon(point&&... all_points) {
      (points.push_back(std::forward<point>(all_points)), ...);
    }

    void swap(Polygon& other) {
      std::swap(points, other.points);
    }
    Polygon& operator=(Polygon other) {
      swap(other);
      return *this;
    }

    long long verticesCount() const {
      return points.size();
    }
    const std::vector<Point> getVertices() const {
      return points;
    }
    bool isConvex() const;
    double perimeter() const override;
    double area() const override;

    bool operator==(const Shape& another) const override;
    bool operator!=(const Shape& another) const override {
      return !(*this == another);
    }

    bool ComparePolygons(std::vector<double> first_sides, std::vector<double> first_corners,
                         std::vector<double> second_sides, std::vector<double> second_corners,
                         int index, double ratio) const;
    bool isCongruentTo(const Shape& another) const override;
    bool isSimilarTo(const Shape& another) const override;
    bool containsPoint(const Point& point) const override;

    void rotate(const Point& center, double angle) override;
    void reflect(const Point& center) override;
    void reflect(const Line& axis) override;
    void scale(const Point& center, double coefficient) override;
};

bool Polygon::isConvex() const {
  int convex = 0;
  for (size_t i = 0; i < points.size(); ++i) {
    Vector first_side(points[i], points[(i + 1) % points.size()]);
    Vector second_side(points[(i + 1) % points.size()], points[(i + 2) % points.size()]);
    int convex_i = first_side.x * second_side.y - first_side.y * second_side.x;
    if (convex == 0) {
      convex = convex_i;
    } else if (convex < 0 and convex_i > 0) {
      return false;
    } else if (convex > 0 and convex_i < 0) {
      return false;
    }
  }
  return true;
}

double Polygon::perimeter() const {
  double perimeter = 0;
  for (size_t i = 0; i < points.size(); ++i) {
    Vector side(points[i], points[(i + 1) % points.size()]);
    perimeter += side.length();
  }
  return perimeter;
}

double Polygon::area() const {
  double area = 0;
  for (size_t i = 0; i < points.size(); ++i) {
    Point second_point = points[(i + 1) % points.size()];
    area += points[i].x * second_point.y - second_point.x * points[i].y;
  }
  return 0.5 * std::fabs(area);
}

bool Polygon::operator==(const Shape& another) const {
  const auto* polygon = dynamic_cast<const Polygon*>(&another);
  if (polygon == nullptr) {
    return false;
  }

  if (points.size() != polygon->points.size()) {
    return false;
  }

  int size = static_cast<int>(points.size());
  int index_same;
  bool same = false;
  for (int i = 0; i < size; ++i) {
    if (points[0] == polygon->points[i]) {
      index_same = i;
      same = true;
      break;
    }
  }

  if (!same) {
    return false;
  }

  if (points[1] == polygon->points[(index_same + 1) % size]) {
    for (int i = 0; i < size; ++i) {
      if (points[i] != polygon->points[(i + index_same) % size]) {
        return false;
      }
    }
    return true;
  } else if (points[1] == polygon->points[(index_same - 1 + size) % size]) {
    for (int i = 0; i < size; ++i) {
      if (points[i] != polygon->points[(index_same - i + size) % size]) {
        return false;
      }
    }
    return true;
  }
  return false;
}

bool Polygon::ComparePolygons(std::vector<double> first_sides, std::vector<double> first_corners,
                              std::vector<double> second_sides, std::vector<double> second_corners,
                              int index, double ratio) const {
  int size = static_cast<int>(points.size());
  if (CompareDouble(first_sides[1], second_sides[(index + 1) % size] * ratio)) {
    for (int j = 0; j < size; ++j) {
      if (!CompareDouble(first_sides[j], second_sides[(j + index) % size] * ratio)) {
        return false;
      }
      if (!CompareDouble(first_corners[j], second_corners[(j + index - 1) % size])) {
        return false;
      }
    }
    return true;
  } else if (CompareDouble(first_sides[1], second_sides[(points.size() + index - 1) % points.size()] * ratio)) {
    for (int j = 0; j < size; ++j) {
      if (!CompareDouble(first_sides[j], second_sides[(points.size() + index - j) % points.size()] * ratio)) {
        return false;
      }
      if (!CompareDouble(first_corners[(j)  % points.size()], second_corners[(points.size() + index - j - 1) % points.size()])) {
        return false;
      }
    }
    return true;
  }
  return false;
}
bool Polygon::isCongruentTo(const Shape& another) const {
  const auto* polygon = dynamic_cast<const Polygon*>(&another);
  if (polygon == nullptr) {
    return false;
  }

  if (points.size() != polygon->points.size()) {
    return false;
  }

  int size = static_cast<int>(points.size());
  std::vector<double> first_sides;
  std::vector<double> first_corners;
  std::vector<double> second_sides;
  std::vector<double> second_corners;

  for (size_t i = 0; i < points.size(); ++i) {
    Vector side1_first(points[i], points[(i + 1) % points.size()]);
    Vector side2_first(points[(i + 1) % points.size()], points[(i + 2) % points.size()]);
    first_sides.push_back(side1_first.length());
    first_corners.push_back(side1_first.cos(side2_first));
    Vector side1_second(polygon->points[i], polygon->points[(i + 1) % points.size()]);
    Vector side2_second(polygon->points[(i + 1) % points.size()], polygon->points[(i + 2) % points.size()]);
    second_sides.push_back(side1_second.length());
    second_corners.push_back(side1_second.cos(side2_second));
  }

  for (int i = 0; i < size; ++i) {
    if (CompareDouble(first_sides[0], second_sides[i])) {
      if (ComparePolygons(first_sides, first_corners, second_sides, second_corners, i, 1)) {
        return true;
      }
    }
  }
  return false;
}
bool Polygon::isSimilarTo(const Shape& another) const {
  const auto* polygon = dynamic_cast<const Polygon*>(&another);
  if (polygon == nullptr) {
    return false;
  }

  if (points.size() != polygon->points.size()) {
    return false;
  }

  int size = static_cast<int>(points.size());
  std::vector<double> first_sides;
  std::vector<double> first_corners;
  std::vector<double> second_sides;
  std::vector<double> second_corners;

  for (size_t i = 0; i < points.size(); ++i) {
    Vector side1_first(points[i], points[(i + 1) % points.size()]);
    Vector side2_first(points[(i + 1) % points.size()], points[(i + 2) % points.size()]);
    first_sides.push_back(side1_first.length());
    first_corners.push_back(side1_first.cos(side2_first));
    Vector side1_second(polygon->points[i], polygon->points[(i + 1) % points.size()]);
    Vector side2_second(polygon->points[(i + 1) % points.size()], polygon->points[(i + 2) % points.size()]);
    second_sides.push_back(side1_second.length());
    second_corners.push_back(side1_second.cos(side2_second));
  }

  for (int i = 0; i < size; ++i) {
    double ratio = first_sides[0] / second_sides[i];
    if (ComparePolygons(first_sides, first_corners, second_sides, second_corners, i, ratio)) {
      return true;
    }
  }
  return false;
}

bool Polygon::containsPoint(const Point& point) const {
  double angle = 0;
  for (int i = 0; i < static_cast<int>(points.size()); ++i) {
    Vector side1(point, points[i]);
    Vector side2(point, points[(i + 1) % points.size()]);
    angle += side1.angle(side2);
  }
  std::cerr << "point" << point.x << " " << point.y << "\n";
  for (auto x: points) std::cerr << x.x << " " << x.y << "\n";
  return CompareDouble(angle, 2 * M_PI);
}

void Polygon::rotate(const Point& center, double angle) {
  for (auto& point : points) {
    point.rotate(center, angle);
  }
}

void Polygon::reflect(const Point& center) {
  for (auto& point : points) {
    point.reflect(center);
  }
}

void Polygon::reflect(const Line& axis) {
  Vector normal;
  normal.x = axis.point1.y - axis.point2.y;
  normal.y = axis.point2.x - axis.point1.x;
  double length_normal = normal.length();
  normal.x /= length_normal;
  normal.y /= length_normal;
  for (auto& point : points) {
    double scalar = 2.0 * (normal.x * point.x + normal.y * point.y);
    point.x -= scalar * normal.x;
    point.y -= scalar * normal.y;
  }
}

void Polygon::scale(const Point& center, double coefficient) {
  for (auto& point : points) {
    Vector vector(center, point);
    point.x = center.x + vector.x * coefficient;
    point.y = center.y + vector.y * coefficient;
  }
}

class Ellipse: public Shape {
protected:
    Point focus1;
    Point focus2;
    double distance;
public:
    Ellipse(): focus1(), focus2(), distance(0) {};
    Ellipse(Point focus1, Point focus2, double distance):
            focus1(focus1), focus2(focus2), distance(distance) {};
    Ellipse(Ellipse& other):
            focus1(other.focus1), focus2(other.focus2), distance(other.distance) {};

    void swap(Ellipse& other) {
      std::swap(focus1, other.focus1);
      std::swap(focus2, other.focus2);
      std::swap(distance, other.distance);
    }
    Ellipse& operator=(Ellipse other) {
      swap(other);
      return *this;
    }

    const std::pair<Point,Point> focuses() const {
      return {focus1, focus2};
    }
    double focus_distance() const {
      return sqrt((focus1.x - focus2.x) * (focus1.x - focus2.x)
                  + (focus1.y - focus2.y) * (focus1.y - focus2.y));
    }
    double small_axis() const {
      return sqrt(distance * distance - focus_distance() * focus_distance()) / 2.0;
    }
    double big_axis() const {
      return distance / 2.0;
    }
    Point center() const {
      return Point((focus1.x + focus2.x) / 2.0, (focus1.y + focus2.y) / 2.0);
    }
    double eccentricity() const {
      return focus_distance() / (distance);
    }

    std::pair<Line, Line> directrices() const;
    double perimeter() const override {
      return 4.0 * big_axis() * std::comp_ellint_2(eccentricity());
    }
    double area() const override {
      return M_PI * small_axis() * big_axis();
    }

    bool operator==(const Shape& another) const override;
    bool operator!=(const Shape& another) const override {
      return !(*this == another);
    }

    bool isCongruentTo(const Shape& another) const override;
    bool isSimilarTo(const Shape& another) const override;
    bool containsPoint(const Point& point) const override;

    void rotate(const Point& center, double angle) override;
    void reflect(const Point& center) override;
    void reflect(const Line& axis) override;
    void scale(const Point& center, double coefficient) override;
};

bool Ellipse::operator==(const Shape& another) const {
  const auto* ellipse = dynamic_cast<const Ellipse*>(&another);
  if (ellipse == nullptr) {
    return false;
  }
  return ((focus1 == ellipse->focus1 and focus2 == ellipse->focus2) or
          (focus1 == ellipse->focus2 and focus2 == ellipse->focus1)) and
         CompareDouble(distance, ellipse->distance);
}

std::pair<Line, Line> Ellipse::directrices() const {
  Vector vector(center(), focus1);
  vector *= (1 / vector.length());
  vector *= (big_axis() / eccentricity());
  Point first_point(vector.x + center().x, vector.y + center().y);
  Vector normal;
  normal.x = focus1.y - focus2.y;
  normal.y = focus2.x - focus1.x;
  double length_normal = normal.length();
  normal.x /= length_normal;
  normal.y /= length_normal;
  Point second_point(first_point.x + normal.x, first_point.y + normal.y);
  Line directrice1(first_point, second_point);
  first_point.x = center().x - first_point.x;
  first_point.y = center().y - first_point.y;
  second_point.x = center().x - second_point.x;
  second_point.y = center().y - second_point.y;
  Line directrice2(first_point, second_point);
  return {directrice1, directrice2};
}

bool Ellipse::isCongruentTo(const Shape& another) const {
  const auto* ellipse = dynamic_cast<const Ellipse*>(&another);
  if (ellipse == nullptr) {
    return false;
  }
  return CompareDouble(big_axis(), ellipse->big_axis()) and CompareDouble(small_axis(), ellipse->small_axis());
}

bool Ellipse::isSimilarTo(const Shape& another) const {
  const auto* ellipse = dynamic_cast<const Ellipse*>(&another);
  if (ellipse == nullptr) {
    return false;
  }
  double coefficient = big_axis() / ellipse->big_axis();
  return CompareDouble(small_axis(), ellipse->small_axis() * coefficient);
}

bool Ellipse::containsPoint(const Point& point) const {
  Vector focus_distance1(point, focus1);
  Vector focus_distance2(point, focus2);
  double sum_distance = focus_distance1.length() + focus_distance2.length();
  return sum_distance < distance or CompareDouble(distance, sum_distance);
}

void Ellipse::rotate(const Point& center, double angle) {
  focus1.rotate(center, angle);
  focus2.rotate(center, angle);
}

void Ellipse::reflect(const Point& center) {
  focus1.reflect(center);
  focus2.reflect(center);
}

void Ellipse::reflect(const Line& axis) {
  Vector normal;
  normal.x = axis.point1.y - axis.point2.y;
  normal.y = axis.point2.x - axis.point1.x;
  double length_normal = normal.length();
  normal.x /= length_normal;
  normal.y /= length_normal;

  double scalar1 = 2.0 * (normal.x * focus1.x + normal.y * focus1.y);
  focus1.x -= scalar1 * normal.x;
  focus1.y -= scalar1 * normal.y;
  double scalar2 = 2.0 * (normal.x * focus2.x + normal.y * focus2.y);
  focus2.x -= scalar2 * normal.x;
  focus2.y -= scalar2 * normal.y;
}

void Ellipse::scale(const Point& center, double coefficient) {
  Vector vector1(center, focus1);
  focus1.x = center.x + vector1.x * coefficient;
  focus1.y = center.y + vector1.y * coefficient;
  Vector vector2(center, focus2);
  focus2.x = center.x + vector2.x * coefficient;
  focus2.y = center.y + vector2.y * coefficient;
  distance *= coefficient;
}

class Circle: public Ellipse {
public:
    Circle(Point center, double radius);
    double radius() {
      return distance / 2.0;
    }
};
Circle::Circle(Point center, double radius) {
  focus1 = center;
  focus2 = center;
  distance = 2.0 * radius;
}

class Rectangle: public Polygon {
public:
    Rectangle() = default;
    Rectangle(Point point1, Point point3, double ratio);

    void swap(Rectangle& other) {
      std::swap(points, other.points);
    }
    Rectangle& operator=(Rectangle other) {
      swap(other);
      return *this;
    }

    Point center() const {
      return Point((points[0].x + points[2].x) / 2.0,  (points[0].y + points[2].y) / 2.0);
    }
    std::pair<Line, Line> diagonals() const;
};

std::pair<Line, Line> Rectangle::diagonals() const {
  return {Line(points[0], points[2]), Line(points[1], points[3])};
}

Rectangle::Rectangle(Point point1, Point point3, double ratio) {
  if (ratio < 1.0) {
    ratio = 1.0 / ratio;
  }
  Point point2(point3);
  point2.rotate(point1, atan(ratio) * 180.0 / M_PI);
  double coefficient = 1 / sqrt(ratio * ratio + 1);
  Vector vector(point1, point2);
  point2.x = point1.x + vector.x * coefficient;
  point2.y = point1.y + vector.y * coefficient;
  points = {point1, point2, point3};
  point2.reflect(Point((point1.x + point3.x) / 2, (point1.y + point3.y) / 2));
  points.push_back(point2);
}

class Square: public Rectangle {
public:
    Square(Point point1, Point point3) : Rectangle(point1, point3, 1.0) {}

    void swap(Square& other) {
      std::swap(points, other.points);
    }
    Square& operator=(Square other) {
      swap(other);
      return *this;
    }

    Circle circumscribedCircle() const {
      return Circle(center(), Vector(points[0], center()).length());
    }
    Circle inscribedCircle() const {
      return Circle(center(), Vector(points[0], points[1]).length());
    }
};

class Triangle: public Polygon {
public:
    Triangle(Point point1, Point point2, Point point3): Polygon(point1, point2, point3) {}

    void swap(Triangle& other) {
      std::swap(points, other.points);
    }
    Triangle& operator=(Triangle other) {
      swap(other);
      return *this;
    }

    Circle circumscribedCircle() const;
    Circle inscribedCircle() const;

    Point centroid() const;
    Point orthocenter() const;

    Line EulerLine() const;
    Circle ninePointsCircle() const;
};

Circle Triangle::circumscribedCircle() const {
  Vector site12(points[1], points[0]);
  Vector site23(points[2], points[1]);
  Vector site31(points[0], points[2]);

  double length_normal = 2.0 * (site12.x * site31.y - site12.y * site31.x);

  Vector point1(points[0]);
  Vector point2(points[1]);
  Vector point3(points[2]);

  double x_coefficient = site12.y * point3.length2() + site23.y * point1.length2() + site31.y * point2.length2();
  double y_coefficient = site12.x * point3.length2() + site23.x * point1.length2() + site31.x * point2.length2();
  Point center(-x_coefficient / length_normal, y_coefficient / length_normal);

  double radius = Vector(center, points[0]).length();
  return Circle(center, radius);
}

Circle Triangle::inscribedCircle() const {
  Vector site12(points[1], points[0]);
  Vector site23(points[2], points[1]);
  Vector site31(points[0], points[2]);
  Point center;

  center.x = (points[0].x * site23.length() + points[1].x * site31.length() + points[2].x * site12.length()) / Polygon::perimeter();
  center.y = (points[0].y * site23.length() + points[1].y * site31.length() + points[2].y * site12.length()) / Polygon::perimeter();
  double radius = 2.0 * Polygon::area() / Polygon::perimeter();
  return Circle(center, radius);
}

Point Triangle::centroid() const {
  Vector point1(points[0]);
  Vector point2(points[1]);
  Vector point3(points[2]);

  Point centroid;
  centroid.x = (points[0].x + points[1].x + points[2].x) / 3;
  centroid.y = (points[0].y + points[1].y + points[2].y) / 3;
  return centroid;
}

Point Triangle::orthocenter() const {
  Vector site23(points[1], points[2]);
  Vector site31(points[0], points[2]);

  Point first_point(site23.x, site31.x);
  Point second_point(site23.y, site31.y);
  Point third_point(points[0].x * first_point.x + points[0].y * second_point.x,
                    points[1].x * first_point.y + points[1].y * second_point.y);

  double scalar12 = first_point.x * second_point.y - first_point.y * second_point.x;
  double scalar23 = third_point.x * second_point.y - third_point.y * second_point.x;
  double scalar31 = first_point.x * third_point.y - first_point.y * third_point.x;
  return Point(scalar23 / scalar12, scalar31 / scalar12);
}

Line Triangle::EulerLine() const {
  return Line(orthocenter(), circumscribedCircle().center());
}

Circle Triangle::ninePointsCircle() const {
  Point center12((points[0].x + points[1].x) / 2, (points[0].y + points[1].y) / 2);
  Point center23((points[1].x + points[2].x) / 2, (points[1].y + points[2].y) / 2);
  Point center31((points[0].x + points[2].x) / 2, (points[0].y + points[2].y) / 2);
  return Triangle(center12, center23, center31).circumscribedCircle();
}
