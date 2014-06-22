﻿#pragma once

// We use ostream for logging purposes.
#include <iostream>  // NOLINT(readability/streams)
#include <string>

#include "geometry/r3_element.hpp"
#include "quantities/quantities.hpp"

namespace principia {
namespace geometry {

// A multivector of rank |rank| on a three-dimensional real inner product
// space bearing the dimensionality of |Scalar|, i.e., an element of
// ⋀ⁿ Scalar³. Do not use this type for |rank == 0| (scalars), use the |Scalar|
// type directly instead.
template<typename Scalar, typename Frame, unsigned int rank>
struct Multivector;

template<typename Scalar, typename Frame>
class Multivector<Scalar, Frame, 1> {
 public:
  Multivector() = default;
  explicit Multivector(R3Element<Scalar> const& coordinates);
  ~Multivector() = default;

  R3Element<Scalar> coordinates() const;
  Scalar Norm() const;

 private:
  R3Element<Scalar> coordinates_;
};

template<typename Scalar, typename Frame>
struct Multivector<Scalar, Frame, 2> {
  Multivector() = default;
  explicit Multivector(R3Element<Scalar> const& coordinates);
  ~Multivector() = default;

  R3Element<Scalar> coordinates() const;
  Scalar Norm() const;

 private:
  R3Element<Scalar> coordinates_;
};

template<typename Scalar, typename Frame>
struct Multivector<Scalar, Frame, 3> {
  Multivector() = default;
  explicit Multivector(Scalar const& coordinates);
  ~Multivector() = default;

  Scalar coordinates() const;
  Scalar Norm() const;

 private:
  Scalar coordinates_;
};

template<typename Scalar, typename Frame>
using Vector = Multivector<Scalar, Frame, 1>;
template<typename Scalar, typename Frame>
using Bivector = Multivector<Scalar, Frame, 2>;
template<typename Scalar, typename Frame>
using Trivector = Multivector<Scalar, Frame, 3>;

template<typename LScalar, typename RScalar, typename Frame>
quantities::Product<LScalar, RScalar> InnerProduct(
    Vector<LScalar, Frame> const& left,
    Vector<RScalar, Frame> const& right);
template<typename LScalar, typename RScalar, typename Frame>
quantities::Product<LScalar, RScalar> InnerProduct(
    Bivector<LScalar, Frame> const& left,
    Bivector<RScalar, Frame> const& right);
template<typename LScalar, typename RScalar, typename Frame>
quantities::Product<LScalar, RScalar> InnerProduct(
    Trivector<LScalar, Frame> const& left,
    Trivector<RScalar, Frame> const& right);

template<typename LScalar, typename RScalar, typename Frame>
Bivector<quantities::Product<LScalar, RScalar>, Frame> Wedge(
    Vector<LScalar, Frame> const& left,
    Vector<RScalar, Frame> const& right);
template<typename LScalar, typename RScalar, typename Frame>
Trivector<quantities::Product<LScalar, RScalar>, Frame> Wedge(
    Bivector<LScalar, Frame> const& left,
    Vector<RScalar, Frame> const& right);
template<typename LScalar, typename RScalar, typename Frame>
Trivector<quantities::Product<LScalar, RScalar>, Frame> Wedge(
    Vector<LScalar, Frame> const& left,
    Bivector<RScalar, Frame> const& right);

// Lie bracket on 𝑉 ∧ 𝑉 ≅ 𝖘𝔬(𝑉).
template<typename LScalar, typename RScalar, typename Frame>
Bivector<quantities::Product<LScalar, RScalar>, Frame> Commutator(
    Bivector<LScalar, Frame> const& left,
    Bivector<RScalar, Frame> const& right);

// Left action of 𝑉 ∧ 𝑉 ≅ 𝖘𝔬(𝑉) on 𝑉.
template<typename LScalar, typename RScalar, typename Frame>
Vector<quantities::Product<LScalar, RScalar>, Frame> operator*(
    Bivector<LScalar, Frame> const& left,
    Vector<RScalar, Frame> const& right);

// Right action of 𝑉 ∧ 𝑉 ≅ 𝖘𝔬(𝑉) on 𝑉* ≅ 𝑉.
template<typename LScalar, typename RScalar, typename Frame>
Vector<quantities::Product<LScalar, RScalar>, Frame> operator*(
    Vector<LScalar, Frame> const& left,
    Bivector<RScalar, Frame> const& right);

template<typename FromFrame, typename ToFrame> class Rotation;

// Exponential map 𝑉 ∧ 𝑉 ≅ 𝖘𝔬(𝑉) → SO(𝑉).
template<typename Frame>
Rotation<Frame, Frame> Exp(Bivector<quantities::Angle, Frame> const& exponent);

template<typename LScalar, typename RScalar, typename Frame>
Vector<quantities::Product<LScalar, RScalar>, Frame> operator*(
    Bivector<LScalar, Frame> const& left,
    Trivector<RScalar, Frame> const& right);
template<typename LScalar, typename RScalar, typename Frame>
Vector<quantities::Product<LScalar, RScalar>, Frame> operator*(
    Trivector<LScalar, Frame> const& left,
    Bivector<RScalar, Frame> const& right);
template<typename LScalar, typename RScalar, typename Frame>
Bivector<quantities::Product<LScalar, RScalar>, Frame> operator*(
    Vector<LScalar, Frame> const& left,
    Trivector<RScalar, Frame> const& right);
template<typename LScalar, typename RScalar, typename Frame>
Bivector<quantities::Product<LScalar, RScalar>, Frame> operator*(
    Trivector<LScalar, Frame> const& left,
    Vector<RScalar, Frame> const& right);

template<typename Scalar, typename Frame, unsigned int rank>
Multivector<Scalar, Frame, rank> operator+(
    Multivector<Scalar, Frame, rank> const& right);
template<typename Scalar, typename Frame, unsigned int rank>
Multivector<Scalar, Frame, rank> operator-(
    Multivector<Scalar, Frame, rank> const& right);

template<typename Scalar, typename Frame, unsigned int rank>
Multivector<Scalar, Frame, rank> operator+(
    Multivector<Scalar, Frame, rank> const& left,
    Multivector<Scalar, Frame, rank> const& right);
template<typename Scalar, typename Frame, unsigned int rank>
Multivector<Scalar, Frame, rank> operator-(
    Multivector<Scalar, Frame, rank> const& left,
    Multivector<Scalar, Frame, rank> const& right);

template<typename Scalar, typename Frame, unsigned int rank>
Multivector<Scalar, Frame, rank> operator*(
    double const left,
    Multivector<Scalar, Frame, rank> const& right);
template<typename Scalar, typename Frame, unsigned int rank>
Multivector<Scalar, Frame, rank> operator*(
    Multivector<Scalar, Frame, rank> const& left,
    double const right);
template<typename Scalar, typename Frame, unsigned int rank>
Multivector<Scalar, Frame, rank> operator/(
    Multivector<Scalar, Frame, rank> const& left,
    double const right);

template<typename LDimension, typename RScalar, typename Frame,
         unsigned int rank>
Multivector<quantities::Product<quantities::Quantity<LDimension>, RScalar>,
            Frame, rank>
operator*(quantities::Quantity<LDimension> const& left,
          Multivector<RScalar, Frame, rank> const& right);

template<typename LScalar, typename RDimension, typename Frame,
         unsigned int rank>
Multivector<quantities::Product<LScalar, quantities::Quantity<RDimension>>,
            Frame, rank>
operator*(Multivector<LScalar, Frame, rank> const& left,
          quantities::Quantity<RDimension> const& right);

template<typename LScalar, typename RDimension, typename Frame,
         unsigned int rank>
Multivector<quantities::Quotient<LScalar, quantities::Quantity<RDimension>>,
            Frame, rank>
operator/(Multivector<LScalar, Frame, rank> const& left,
          quantities::Quantity<RDimension> const& right);

template<typename Scalar, typename Frame, unsigned int rank>
bool operator==(Multivector<Scalar, Frame, rank> const& left,
                Multivector<Scalar, Frame, rank> const& right);
template<typename Scalar, typename Frame, unsigned int rank>
bool operator!=(Multivector<Scalar, Frame, rank> const& left,
                Multivector<Scalar, Frame, rank> const& right);

template<typename Scalar, typename Frame, unsigned int rank>
Multivector<Scalar, Frame, rank>& operator+=(
    Multivector<Scalar, Frame, rank>& left,  // NOLINT(runtime/references)
    Multivector<Scalar, Frame, rank> const& right);
template<typename Scalar, typename Frame, unsigned int rank>
Multivector<Scalar, Frame, rank>& operator-=(
    Multivector<Scalar, Frame, rank>& left,  // NOLINT(runtime/references)
    Multivector<Scalar, Frame, rank> const& right);
template<typename Scalar, typename Frame, unsigned int rank>
Multivector<Scalar, Frame, rank>& operator*=(
    Multivector<Scalar, Frame, rank>& left,  // NOLINT(runtime/references)
    double const right);
template<typename Scalar, typename Frame, unsigned int rank>
Multivector<Scalar, Frame, rank>& operator/=(
    Multivector<Scalar, Frame, rank>& left,  // NOLINT(runtime/references)
    double const right);

template<typename Scalar, typename Frame, unsigned int rank>
std::string DebugString(Multivector<Scalar, Frame, rank> const& multivector);

template<typename Scalar, typename Frame, unsigned int rank>
std::ostream& operator<<(std::ostream& out,
                         Multivector<Scalar, Frame, rank> const& multivector);

}  // namespace geometry
}  // namespace principia

#include "geometry/grassmann_body.hpp"
