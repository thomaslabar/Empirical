//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A simple class to track value pairs of any kind, optimized for points in 2D Space
//
//  Note: For maximal efficiency, prefer SquareMagnitude() and SquareDistance()
//        over Magnitude() and Distance() as the latter require a square-root.


#ifndef EMP_POINT_2D_H
#define EMP_POINT_2D_H

#include <cmath>
#include <iostream>

namespace emp {

  template <typename TYPE=double> class Point {
  private:
    TYPE x;
    TYPE y;

  public:
    constexpr Point() : x(0.0), y(0.0) { ; }                      // Default = 0,0
    constexpr Point(const Point & _in) : x(_in.x), y(_in.y) { ; } // Copy constructor
    constexpr Point(TYPE _x, TYPE _y) : x(_x), y(_y) { ; }        // Two ints -> x and y
    constexpr Point(const Point & _in, TYPE new_magnitude)
      : x(_in.x * ((double) new_magnitude) / ((double) _in.Magnitude()))
      , y(_in.y * ((double) new_magnitude) / ((double) _in.Magnitude()))
     { ; }
    // ~Point() { ; }

    const Point & operator=(const Point & _in) { x = _in.x; y = _in.y; return *this; }
    constexpr bool operator==(const Point & _in) const { return (x == _in.x) && (y == _in.y); }
    constexpr bool operator!=(const Point & _in) const { return (x != _in.x) || (y != _in.y); }

    constexpr inline TYPE GetX() const { return x; }
    constexpr inline TYPE GetY() const { return y; }
    inline Point & SetX(TYPE in_x) { x = in_x; return *this; }
    inline Point & SetY(TYPE in_y) { y = in_y; return *this; }
    Point & Set(TYPE _x, TYPE _y) { x=_x; y=_y; return *this; }

    constexpr double SquareMagnitude() const { return x*x + y*y; }
    constexpr double Magnitude() const { return sqrt( x*x + y*y ); }
    constexpr bool AtOrigin() const { return x == 0 && y == 0; }
    constexpr bool NonZero() const { return x != 0 || y != 0; }

    // Determine a new point, but don't change this one
    constexpr Point GetMidpoint(const Point & p2) const { return Point((x+p2.x)/2, (y+p2.y)/2); }
    constexpr Point GetRot90() const { return Point(y, -x); }
    constexpr Point GetRot180() const { return Point(-x, -y); }
    constexpr Point GetRot270() const { return Point(-y, x); }

    constexpr Point operator+(const Point & _in) const { return Point(x + _in.x , y + _in.y); }
    constexpr Point operator-(const Point & _in) const { return Point(x - _in.x , y - _in.y); }
    constexpr Point operator*(double mult) const { return Point(((double) x) * mult, ((double) y) * mult); }
    constexpr Point operator/(double div) const { return Point(((double) x) / div, ((double) y) / div); }
    constexpr Point operator*(int mult) const { return Point(x * mult, y * mult); }
    constexpr Point operator/(int div) const { return Point(x / div, y / div); }
    constexpr Point operator-() const { return Point(-x, -y); } // Unary minus

    constexpr Point Abs() const { return Point(std::abs(x), std::abs(y)); } // Absolute value

    // Modify this point.
    Point & Translate(TYPE shift_x, TYPE shift_y) { x += shift_x; y += shift_y; return *this; }
    Point & TranslateX(TYPE shift) { x += shift; return *this; }
    Point & TranslateY(TYPE shift) { y += shift; return *this; }
    Point & ToOrigin() { x = 0; y = 0; return *this; }
    Point & NegateX() { x = -x; return *this; }
    Point & NegateY() { y = -y; return *this; }

    Point & operator+=(const Point & _in) { x += _in.x; y += _in.y; return *this; }
    Point & operator-=(const Point & _in) { x -= _in.x; y -= _in.y; return *this; }
    Point & operator*=(double mult) { x *= mult; y *= mult; return *this; }
    Point & operator/=(double val) { if (val != 0.0) { x /= val;  y /= val; }; return *this; }
    Point & operator*=(int mult) { x *= mult; y *= mult; return *this; }
    Point & operator/=(int val) { if (val != 0.0) { x /= val;  y /= val; }; return *this; }

    Point & Rot90() { return Set(y, -x); }
    Point & Rot180() { return Set(-x, -y); }
    Point & Rot270() { return Set(-y, x); }

    // Square-roots are slow to calculate; if we're just doing comparisons, square-distance
    // is usualy going to be sufficient.
    TYPE SquareDistance(const Point & _in) const {
      const TYPE x_dist = x - _in.x;
      const TYPE y_dist = y - _in.y;
      return x_dist*x_dist + y_dist*y_dist;
    }

    TYPE Distance(const Point & _in) const { return sqrt( SquareDistance(_in) ); }
  };

}

// Overload ostream to work with points.
template <typename TYPE> std::ostream & operator<<(std::ostream & os,
                                                   const emp::Point<TYPE> & point) {
  return os << "(" << point.GetX() << "," << point.GetY() << ")";
}

#endif
