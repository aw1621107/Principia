namespace principia {
namespace integrators {

template<typename Position, typename Momentum>
template<typename Scalar>
SymplecticIntegrator<Position, Momentum>::ValueAndError<Scalar>::ValueAndError(
    Scalar value) : value(value) {}

}  // namespace integrators
}  // namespace principia
