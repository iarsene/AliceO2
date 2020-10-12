// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// @file   IntervalXY.h
/// @author ruben.shahoyan@cern.ch, michael.lettrich@cern.ch
/// @since  Oct 10, 2020
/// @brief

#ifndef MATHUTILS_INCLUDE_MATHUTILS_DETAIL_INTERVALXY_H_
#define MATHUTILS_INCLUDE_MATHUTILS_DETAIL_INTERVALXY_H_

#include "GPUCommonRtypes.h"
#include <cmath>
#include <tuple>

#include "MathUtils/detail/CircleXY.h"

namespace o2
{
namespace math_utils
{
namespace detail
{

template <typename T>
class IntervalXY
{
 public:
  using value_t = T;

  ///< 2D interval in lab frame defined by its one edge and signed projection lengths on X,Y axes
  IntervalXY(T x = T(), T y = T(), T dx = T(), T dy = T());
  T getX0() const;
  T getY0() const;
  T& getX0();
  T& getY0();
  T getX1() const;
  T getY1() const;
  T getDX() const;
  T getDY() const;
  T& getDX();
  T& getDY();

  void setX0(T x0);
  void setY0(T y0);
  void setX1(T x1);
  void setY1(T y1);
  void setDX(T dx);
  void setDY(T dy);
  void setEdges(T x0, T y0, T x1, T y1);

  void eval(T t, T& x, T& y) const;
  std::tuple<T, T> eval(T t) const;

  void getLineCoefs(T& a, T& b, T& c) const;

  /** check if XY interval is seen by the circle.
  * The tolerance parameter eps is interpreted as a fraction of the interval
  * lenght to be added to the edges along the interval (i.e. while the points
  * within the interval are spanning
  * x = xc + dx*t
  * y = yc + dy*t
  * with 0<t<1., we acctually check the interval for -eps<t<1+eps
  */
  bool seenByCircle(const CircleXY<T>& circle, T eps) const;

  bool circleCrossParam(const CircleXY<T>& circle, T& t) const;

  /**
  * check if XY interval is seen by the line defined by other interval
  * The tolerance parameter eps is interpreted as a fraction of the interval
  * lenght to be added to the edges along the interval (i.e. while the points
  * within the interval are spanning
  * x = xc + dx*t
  * y = yc + dy*t
  * with 0<t<1., we acctually check the interval for -eps<t<1+eps
  */
  bool seenByLine(const IntervalXY<T>& other, T eps) const;

  /**
   * get crossing parameter of 2 intervals
   */
  bool lineCrossParam(const IntervalXY<T>& other, T& t) const;

 private:
  T mX, mY;   ///< one of edges
  T mDx, mDY; ///< other edge minus provided

  ClassDefNV(IntervalXY, 2);
};

template <typename T>
IntervalXY<T>::IntervalXY(T x, T y, T dx, T dy) : mX(std::move(x)), mY(std::move(y)), mDx(std::move(dx)), mDY(std::move(dy))
{
}

template <typename T>
inline T IntervalXY<T>::getX0() const
{
  return mX;
}

template <typename T>
inline T IntervalXY<T>::getY0() const
{
  return mY;
}

template <typename T>
inline T& IntervalXY<T>::getX0()
{
  return mX;
}

template <typename T>
inline T& IntervalXY<T>::getY0()
{
  return mY;
}

template <typename T>
inline T IntervalXY<T>::getX1() const
{
  return mX + mDx;
}
template <typename T>
inline T IntervalXY<T>::getY1() const
{
  return mY + mDY;
}
template <typename T>
inline T IntervalXY<T>::getDX() const
{
  return mDx;
}
template <typename T>
inline T IntervalXY<T>::getDY() const
{
  return mDY;
}

template <typename T>
inline T& IntervalXY<T>::getDX()
{
  return mDx;
}
template <typename T>
inline T& IntervalXY<T>::getDY()
{
  return mDY;
}

template <typename T>
inline void IntervalXY<T>::setX0(T x0)
{
  mX = x0;
}

template <typename T>
inline void IntervalXY<T>::setY0(T y0)
{
  mY = y0;
}
template <typename T>
inline void IntervalXY<T>::setX1(T x1)
{
  mDx = x1 - mX;
}
template <typename T>
inline void IntervalXY<T>::setY1(T y1)
{
  mDY = y1 - mY;
}
template <typename T>
inline void IntervalXY<T>::setDX(T dx)
{
  mDx = dx;
}
template <typename T>
inline void IntervalXY<T>::setDY(T dy)
{
  mDY = dy;
}

template <typename T>
inline void IntervalXY<T>::setEdges(T x0, T y0, T x1, T y1)
{
  mX = x0;
  mY = y0;
  mDx = x1 - x0;
  mDY = y1 - y0;
}

template <typename T>
inline std::tuple<T, T> IntervalXY<T>::eval(T t) const
{
  return {mX + t * mDx, mY + t * mDY};
}

template <typename T>
inline void IntervalXY<T>::eval(T t, T& x, T& y) const
{
  std::tie(x, y) = eval(t);
}

template <typename T>
void IntervalXY<T>::getLineCoefs(T& a, T& b, T& c) const
{
  // convert to line parameters in canonical form: a*x+b*y+c = 0
  c = mX * mDY - mY * mDx;
  if (c) {
    a = -mDY;
    b = mDx;
  } else if (fabs(mDx) > fabs(mDY)) {
    a = 0.;
    b = -1.;
    c = mY;
  } else {
    a = -1.;
    b = 0.;
    c = mX;
  }
}

template <typename T>
bool IntervalXY<T>::seenByCircle(const CircleXY<T>& circle, T eps) const
{
  T dx0 = mX - circle.xC;
  T dy0 = mY - circle.yC;
  T dx1 = dx0 + mDx;
  T dy1 = dy0 + mDY;
  if (eps > 0.) {
    const T dxeps = eps * mDx, dyeps = eps * mDY;
    dx0 -= dxeps;
    dx1 += dxeps;
    dy0 -= dyeps;
    dy1 += dyeps;
  }
  // distance^2 from circle center to edges
  const T d02 = dx0 * dx0 + dy0 * dy0;
  const T d12 = dx1 * dx1 + dy1 * dy1;
  const T rC2 = circle.rC * circle.rC;

  return (d02 - rC2) * (d12 - rC2) < 0;
}
template <typename T>
bool IntervalXY<T>::circleCrossParam(const CircleXY<T>& circle, T& t) const
{
  const T dx = mX - circle.xC;
  const T dy = mY - circle.yC;
  const T del2I = 1.f / (mDx * mDx + mDY * mDY);
  const T b = (dx * mDx + dy * mDY) * del2I;
  const T c = del2I * (dx * dx + dy * dy - circle.rC * circle.rC);
  T det = b * b - c;
  if (det < 0.f) {
    return false;
  }
  det = std::sqrt(det);
  const T t0 = -b - det;
  const T t1 = -b + det;
  // select the one closer to [0:1] interval
  t = (std::fabs(t0 - 0.5f) < std::fabs(t1 - 0.5f)) ? t0 : t1;
  return true;
}

template <typename T>
bool IntervalXY<T>::seenByLine(const IntervalXY<T>& other, T eps) const
{
  T a, b, c; // find equation of the line a*x+b*y+c = 0
  other.getLineCoefs(a, b, c);
  const auto [x0, y0] = eval(-eps);
  const auto [x1, y1] = eval(1.f + eps);

  return (a * x0 + b * y0 + c) * (a * x1 + b * y1 + c) < 0;
}

template <typename T>
bool IntervalXY<T>::lineCrossParam(const IntervalXY<T>& other, T& t) const
{
  // tolerance
  constexpr float eps = 1.e-9f;
  const T dx = other.getX0() - mX;
  const T dy = other.getY0() - mY;
  const T det = -mDx * other.getY0() + mDY * other.getX0();
  if (std::fabs(det) < eps) {
    return false; // parallel
  }
  t = (-dx * other.getY0() + dy * other.getX0()) / det;
  return true;
}

} // namespace detail
} // namespace math_utils
} // namespace o2

#endif /* MATHUTILS_INCLUDE_MATHUTILS_DETAIL_INTERVALXY_H_ */