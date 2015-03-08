﻿#pragma once

#include <algorithm>
#include <cmath>
#include <ctime>
#include <vector>

#include "quantities/quantities.hpp"

// Mixed assemblies are not supported by Unity/Mono.
#include "glog/logging.h"

#ifdef ADVANCE_ΔQSTAGE
#error ADVANCE_ΔQSTAGE already defined
#else
#define ADVANCE_ΔQSTAGE(step)                                    \
  do {                                                           \
    compute_velocity(p_stage, &v);                               \
    for (int k = 0; k < dimension; ++k) {                        \
      Position const Δq = (*Δqstage_previous)[k] + step * v[k];  \
      q_stage[k] = q_last[k].value + Δq;                         \
      (*Δqstage_current)[k] = Δq;                                \
    }                                                            \
  } while (false)
#endif

#ifdef ADVANCE_ΔPSTAGE
#error ADVANCE_ΔPSTAGE already defined
#else
#define ADVANCE_ΔPSTAGE(step, q_clock)                           \
  do {                                                           \
    compute_force(q_clock, q_stage, &f);                         \
    for (int k = 0; k < dimension; ++k) {                        \
      Momentum const Δp = (*Δpstage_previous)[k] + step * f[k];  \
      p_stage[k] = p_last[k].value + Δp;                         \
      (*Δpstage_current)[k] = Δp;                                \
    }                                                            \
  } while (false)
#endif

namespace principia {

using quantities::Quotient;

namespace integrators {

template<typename Position, typename Momentum>
SPRKIntegrator<Position, Momentum>::SPRKIntegrator() : stages_(0) {}

template<typename Position, typename Momentum>
typename SPRKIntegrator<Position, Momentum>::Scheme const&
SPRKIntegrator<Position, Momentum>::Leapfrog() const {
  static Scheme const scheme = {{ 0.5, 0.5}, { 0.0, 1.0}};
  return scheme;
}

template<typename Position, typename Momentum>
typename SPRKIntegrator<Position, Momentum>::Scheme const&
SPRKIntegrator<Position, Momentum>::PseudoLeapfrog() const {
  static Scheme const scheme = {{ 1.0, 0.0}, { 0.5, 0.5}};
  return scheme;
}

template<typename Position, typename Momentum>
typename SPRKIntegrator<Position, Momentum>::Scheme const&
SPRKIntegrator<Position, Momentum>::McLachlanAtela1992Order2Optimal() const {
  static Scheme const scheme = {
      { 0.7071067811865475244,
        0.2928932188134524756},
      { 0.2928932188134524756,
        0.7071067811865475244}};
  return scheme;
}


template<typename Position, typename Momentum>
typename SPRKIntegrator<Position, Momentum>::Scheme const&
SPRKIntegrator<Position, Momentum>::Ruth1983() const {
  static Scheme const scheme = {{ 2. / 3., -2. / 3., 1.},
                                { 7. / 24., 3. / 4., -1. / 24.}};
  return scheme;
}

template<typename Position, typename Momentum>
typename SPRKIntegrator<Position, Momentum>::Scheme const&
SPRKIntegrator<Position, Momentum>::McLachlanAtela1992Order3Optimal() const {
  static Scheme const scheme = {
      { 0.9196615230173998571,
       -0.1879916187991597820,
        0.2683300957817599250},
      { 0.2683300957817599250,
       -0.1879916187991597820,
        0.9196615230173998571}};
  return scheme;
}

template<typename Position, typename Momentum>
typename SPRKIntegrator<Position, Momentum>::Scheme const&
SPRKIntegrator<Position, Momentum>::
    CandyRozmus1991ForestRuth1990SynchronousMomenta() const {
  static Scheme const scheme = {
      { 0.6756035959798288170,
       -0.1756035959798288170,
       -0.1756035959798288170,
        0.6756035959798288170},
      { 0.0,
        1.351207191959657634,
       -1.702414383919315268,
        1.351207191959657634}};
  return scheme;
}

template<typename Position, typename Momentum>
typename SPRKIntegrator<Position, Momentum>::Scheme const&
SPRKIntegrator<Position, Momentum>::
    CandyRozmus1991ForestRuth1990SynchronousPositions() const {
  static Scheme const scheme = {
      { 1.3512071919596576340,
       -1.7024143839193152681,
        1.3512071919596576340,
        0.0},
      { 0.6756035959798288170,
       -0.1756035959798288170,
       -0.1756035959798288170,
        0.6756035959798288170}};
  return scheme;
}

template<typename Position, typename Momentum>
typename SPRKIntegrator<Position, Momentum>::Scheme const&
SPRKIntegrator<Position, Momentum>::McLachlanAtela1992Order4Optimal() const {
  static Scheme const scheme = {
      { 0.5153528374311229364,
       -0.085782019412973646,
        0.4415830236164665242,
        0.1288461583653841854},
      { 0.1344961992774310892,
       -0.2248198030794208058,
        0.7563200005156682911,
        0.3340036032863214255}};
  return scheme;
}

template<typename Position, typename Momentum>
typename SPRKIntegrator<Position, Momentum>::Scheme const&
SPRKIntegrator<Position, Momentum>::McLachlanAtela1992Order5Optimal() const {
  static Scheme const scheme = {
      { 0.339839625839110000,
       -0.088601336903027329,
        0.5858564768259621188,
       -0.603039356536491888,
        0.3235807965546976394,
        0.4423637942197494587},
      { 0.1193900292875672758,
        0.6989273703824752308,
       -0.1713123582716007754,
        0.4012695022513534480,
        0.0107050818482359840,
       -0.0589796254980311632}};
  return scheme;
}

template<typename Position, typename Momentum>
typename SPRKIntegrator<Position, Momentum>::Scheme const&
SPRKIntegrator<Position, Momentum>::Yoshida1990Order6A() const {
  static Scheme const scheme = {
      { 0.78451361047755726382,
        0.23557321335935813369,
       -1.17767998417887100695,
        1.31518632068391121889,
       -1.17767998417887100695,
        0.23557321335935813369,
        0.78451361047755726382,
        0.0},
      { 0.392256805238778631910,
        0.51004341191845769875,
       -0.47105338540975643663,
        0.06875316825252010597,
        0.06875316825252010597,
       -0.47105338540975643663,
        0.51004341191845769875,
        0.392256805238778631910}};
  return scheme;
}

template<typename Position, typename Momentum>
typename SPRKIntegrator<Position, Momentum>::Scheme const&
SPRKIntegrator<Position, Momentum>::Yoshida1990Order6B() const {
  static Scheme const scheme = {
      { 1.43984816797678309102,
        0.00426068187079201616,
       -2.13228522200145152088,
        2.37635274430775282740,
       -2.13228522200145152088,
        0.00426068187079201616,
        1.43984816797678309102,
        0.0},
      { 0.71992408398839154551,
        0.72205442492378755359,
       -1.06401227006532975236,
        0.12203376115315065326,
        0.12203376115315065326,
       -1.06401227006532975236,
        0.72205442492378755359,
        0.71992408398839154551}};
  return scheme;
}

template<typename Position, typename Momentum>
typename SPRKIntegrator<Position, Momentum>::Scheme const&
SPRKIntegrator<Position, Momentum>::Yoshida1990Order6C() const {
  static Scheme const scheme = {
      { 1.44778256239929793290,
       -2.14403531630538931060,
        0.00152886228424927025338,
        2.38944778324368421490,
        0.00152886228424927025338,
       -2.14403531630538931060,
        1.44778256239929793290,
        0.0},
      { 0.72389128119964896645,
       -0.34812637695304568885,
       -1.07125322701057002017,
        1.19548832276396674257,
        1.19548832276396674257,
       -1.07125322701057002017,
       -0.34812637695304568885,
        0.72389128119964896645}};
  return scheme;
}

template<typename Position, typename Momentum>
typename SPRKIntegrator<Position, Momentum>::Scheme const&
SPRKIntegrator<Position, Momentum>::Yoshida1990Order8A() const {
  static Scheme const scheme = {
      { 1.04242620869970426435,
        1.82020630970698006933,
        0.157739928123708321343,
        2.44002732616634406382,
       -0.00716989419709533209981,
       -2.44699182370424588929,
       -1.61582374150065378479,
       -1.7808286265894834253,
       -1.61582374150065378479,
       -2.44699182370424588929,
       -0.00716989419709533209981,
        2.44002732616634406382,
        0.157739928123708321343,
        1.82020630970698006933,
        1.04242620869970426435,
        0.0},
      { 0.521213104349852132174,
        1.43131625920334216684,
        0.988973118915344195337,
        1.29888362714502619258,
        1.21642871598462436586,
       -1.22708085895067061070,
       -2.03140778260244983704,
       -1.6983261840450686051,
       -1.6983261840450686051,
       -2.03140778260244983704,
       -1.22708085895067061070,
        1.21642871598462436586,
        1.29888362714502619258,
        0.988973118915344195337,
        1.43131625920334216684,
        0.521213104349852132174}};
  return scheme;
}

template<typename Position, typename Momentum>
typename SPRKIntegrator<Position, Momentum>::Scheme const&
SPRKIntegrator<Position, Momentum>::Yoshida1990Order8B() const {
  static Scheme const scheme = {
      { 1.48819229202921310080,
       -2.33864815101041943098,
        2.89105148972198900311,
       -2.89688250330423987105,
        0.00378039588362668223674,
        2.89195744315817391244,
       -0.00169248587771706559145,
       -3.0755169612012526619,
       -0.00169248587771706559145,
        2.89195744315817391244,
        0.00378039588362668223674,
       -2.89688250330423987105,
        2.89105148972198900311,
       -2.33864815101041943098,
        1.48819229202921310080,
        0.0},
      { 0.744096146014606550401,
       -0.42522792949060316509,
        0.27620166935578478606,
       -0.00291550679112543397,
       -1.44655105371030659441,
        1.44786891952090029734,
        1.44513247864022842343,
       -1.5386047235394848638,
       -1.5386047235394848638,
        1.44513247864022842343,
        1.44786891952090029734,
       -1.44655105371030659441,
       -0.00291550679112543397,
        0.27620166935578478606,
       -0.42522792949060316509,
        0.744096146014606550401}};
  return scheme;
}

template<typename Position, typename Momentum>
typename SPRKIntegrator<Position, Momentum>::Scheme const&
SPRKIntegrator<Position, Momentum>::Yoshida1990Order8C() const {
  static Scheme const scheme = {
      { 0.629030650210427818049,
        1.36934946416874222370,
       -1.06458714789183904181,
        1.66335809963311356298,
       -1.67896928259637402925,
       -1.55946803821449795876,
        0.311790812418431890510,
        1.6589908845439910692,
        0.311790812418431890510,
       -1.55946803821449795876,
       -1.67896928259637402925,
        1.66335809963311356298,
       -1.06458714789183904181,
        1.36934946416874222370,
        0.629030650210427818049,
        0.0},
      { 0.314515325105213909024,
        0.999190057189585020872,
        0.15238115813845159094,
        0.29938547587063726059,
       -0.00780559148163023314,
       -1.61921866040543599400,
       -0.623838612898033034124,
        0.98539084848121147984,
        0.98539084848121147984,
       -0.623838612898033034124,
       -1.61921866040543599400,
       -0.00780559148163023314,
        0.29938547587063726059,
        0.15238115813845159094,
        0.999190057189585020872,
        0.314515325105213909024}};
  return scheme;
}

template<typename Position, typename Momentum>
typename SPRKIntegrator<Position, Momentum>::Scheme const&
SPRKIntegrator<Position, Momentum>::Yoshida1990Order8D() const {
  static Scheme const scheme = {
      { 0.914844246229642658287,
        0.253693336566286009974,
       -1.44485223686030647660,
       -0.158240635368502468458,
        1.93813913762291232471,
       -1.96061023297558163691,
        0.102799849392219431139,
        1.7084530707866603157,
        0.102799849392219431139,
       -1.96061023297558163691,
        1.93813913762291232471,
       -0.158240635368502468458,
       -1.44485223686030647660,
        0.253693336566286009974,
        0.914844246229642658287,
        0.0},
      { 0.457422123114821329143,
        0.584268791397964334130,
       -0.595579450147010233314,
       -0.801546436114404472530,
        0.88994925112720492813,
       -0.01123554767633465610,
       -0.92890519179168110289,
        0.90562646008943987343,
        0.90562646008943987343,
       -0.92890519179168110289,
       -0.01123554767633465610,
        0.88994925112720492813,
       -0.801546436114404472530,
       -0.595579450147010233314,
        0.584268791397964334130,
        0.457422123114821329143}};
  return scheme;
}

template<typename Position, typename Momentum>
typename SPRKIntegrator<Position, Momentum>::Scheme const&
SPRKIntegrator<Position, Momentum>::Yoshida1990Order8E() const {
  static Scheme const scheme = {
      { 1.30300165757516838484,
        0.107990467718098279648,
       -2.04809795883490205633,
        0.00536018921375238082832,
       -0.0719180053650705075005,
        2.52778927318028339169,
        0.0227738840126312259937,
       -2.6937990149999221983,
        0.0227738840126312259937,
        2.52778927318028339169,
       -0.0719180053650705075005,
        0.00536018921375238082832,
       -2.04809795883490205633,
        0.107990467718098279648,
        1.30300165757516838484,
        0.0},
      { 0.651500828787584192418,
        0.705496062646633332241,
       -0.97005374555840188834,
       -1.02136888481057483775,
       -0.0332789080756590633361,
        1.22793563390760644210,
        1.27528157859645730884,
       -1.33551256549364548617,
       -1.33551256549364548617,
        1.27528157859645730884,
        1.22793563390760644210,
       -0.0332789080756590633361,
       -1.02136888481057483775,
       -0.97005374555840188834,
        0.705496062646633332241,
        0.651500828787584192418}};
  return scheme;
}


template<typename Position, typename Momentum>
void SPRKIntegrator<Position, Momentum>::Initialize(
    Scheme const& coefficients) {
  CHECK_EQ(2, coefficients.size());
  if (coefficients[1].front() == 0.0) {
    vanishing_coefficients_ = kFirstBVanishes;
    first_same_as_last_ = std::make_unique<FirstSameAsLast>();
    first_same_as_last_->first = coefficients[0].front();
    first_same_as_last_->last = coefficients[0].back();
    a_ = std::vector<double>(coefficients[0].begin() + 1,
                             coefficients[0].end());
    b_ = std::vector<double>(coefficients[1].begin() + 1,
                             coefficients[1].end());
    a_.back() += first_same_as_last_->first;
    stages_ = b_.size();
    CHECK_EQ(stages_, a_.size());
  } else if (coefficients[0].back() == 0.0) {
    vanishing_coefficients_ = kLastAVanishes;
    first_same_as_last_ = std::make_unique<FirstSameAsLast>();
    first_same_as_last_->first = coefficients[1].front();
    first_same_as_last_->last = coefficients[1].back();
    a_ = std::vector<double>(coefficients[0].begin(),
                             coefficients[0].end() - 1);
    b_ = std::vector<double>(coefficients[1].begin(),
                             coefficients[1].end() - 1);
    b_.front() += first_same_as_last_->last;
    stages_ = b_.size();
    CHECK_EQ(stages_, a_.size());
  } else {
    vanishing_coefficients_ = kNone;
    a_ = coefficients[0];
    b_ = coefficients[1];
    stages_ = b_.size();
    CHECK_EQ(stages_, a_.size());
  }

  // Runge-Kutta time weights.
  c_.resize(stages_);
  if (vanishing_coefficients_ == kFirstBVanishes) {
    c_[0] = first_same_as_last_->first;
  } else {
    c_[0] = 0;
  }
  for (int j = 1; j < stages_; ++j) {
    c_[j] = c_[j - 1] + a_[j - 1];
  }
}

template<typename Position, typename Momentum>
template<typename AutonomousRightHandSideComputation,
         typename RightHandSideComputation>
void SPRKIntegrator<Position, Momentum>::Solve(
      RightHandSideComputation compute_force,
      AutonomousRightHandSideComputation compute_velocity,
      Parameters const& parameters,
      not_null<std::vector<SystemState>*> const solution) const {
  switch (vanishing_coefficients_) {
    case kNone:
      SolveOptimized<kNone>(
          compute_force, compute_velocity, parameters, solution);
      break;
    case kFirstBVanishes:
      SolveOptimized<kFirstBVanishes>(
          compute_force, compute_velocity, parameters, solution);
      break;
    case kLastAVanishes:
      SolveOptimized<kLastAVanishes>(
          compute_force, compute_velocity, parameters, solution);
      break;
    default:
      LOG(FATAL) << "Invalid vanishing coefficients";
  }
}

template<typename Position, typename Momentum>
template<VanishingCoefficients vanishing_coefficients,
         typename AutonomousRightHandSideComputation,
         typename RightHandSideComputation>
void SPRKIntegrator<Position, Momentum>::SolveOptimized(
      RightHandSideComputation compute_force,
      AutonomousRightHandSideComputation compute_velocity,
      Parameters const& parameters,
      not_null<std::vector<SystemState>*> const solution) const {
  int const dimension = parameters.initial.positions.size();

  std::vector<Position> Δqstage0(dimension);
  std::vector<Position> Δqstage1(dimension);
  std::vector<Momentum> Δpstage0(dimension);
  std::vector<Momentum> Δpstage1(dimension);
  std::vector<Position>* Δqstage_current = &Δqstage1;
  std::vector<Position>* Δqstage_previous = &Δqstage0;
  std::vector<Momentum>* Δpstage_current = &Δpstage1;
  std::vector<Momentum>* Δpstage_previous = &Δpstage0;

  // Dimension the result.
  int const capacity = parameters.sampling_period == 0 ?
    1 :
    static_cast<int>(
        ceil((((parameters.tmax - parameters.initial.time.value) /
                    parameters.Δt) + 1) /
                parameters.sampling_period)) + 1;
  solution->clear();
  solution->reserve(capacity);

  std::vector<DoublePrecision<Position>> q_last(parameters.initial.positions);
  std::vector<DoublePrecision<Momentum>> p_last(parameters.initial.momenta);
  int sampling_phase = 0;

  std::vector<Position> q_stage(dimension);
  std::vector<Momentum> p_stage(dimension);
  std::vector<Quotient<Momentum, Time>> f(dimension);  // Current forces.
  std::vector<Quotient<Position, Time>> v(dimension);  // Current velocities.

  // The following quantity is generally equal to |Δt|, but during the last
  // iteration, if |tmax_is_exact|, it may differ significantly from |Δt|.
  Time h = parameters.Δt;  // Constant for now.

  // During one iteration of the outer loop below we process the time interval
  // [|tn|, |tn| + |h|[.  |tn| is computed using compensated summation to make
  // sure that we don't have drifts.
  DoublePrecision<Time> tn = parameters.initial.time;

  // Whether position and momentum are synchronized between steps, relevant for
  // first-same-as-last (FSAL) integrators. Time is always synchronous with
  // position.
  bool q_and_p_are_synchronized = true;
  bool should_synchronize = false;

  // Integration.  For details see Wolfram Reference,
  // http://reference.wolfram.com/mathematica/tutorial/NDSolveSPRK.html#74387056
  bool at_end = !parameters.tmax_is_exact && parameters.tmax < tn.value + h;
  while (!at_end) {
    // Check if this is the last interval and if so process it appropriately.
    if (parameters.tmax_is_exact) {
      // If |tn| is getting close to |tmax|, use |tmax| as the upper bound of
      // the interval and update |h| accordingly.  The bound chosen here for
      // |tmax| ensures that we don't end up with a ridiculously small last
      // interval: we'd rather make the last interval a bit bigger.  More
      // precisely, the last interval generally has a length between 0.5 Δt and
      // 1.5 Δt, unless it is also the first interval.
      // NOTE(phl): This may lead to convergence as bad as (1.5 Δt)^5 rather
      // than Δt^5.
      if (parameters.tmax <= tn.value + 3 * h / 2) {
        at_end = true;
        h = (parameters.tmax - tn.value) - tn.error;
      }
    } else if (parameters.tmax < tn.value + 2 * h) {
      // If the next interval would overshoot, make this the last interval but
      // stick to the same step.
      at_end = true;
    }
    // Here |h| is the length of the current time interval and |tn| is its
    // start.

    // Increment SPRK step from "'SymplecticPartitionedRungeKutta' Method
    // for NDSolve", algorithm 3.
    for (int k = 0; k < dimension; ++k) {
      (*Δqstage_current)[k] = Position();
      (*Δpstage_current)[k] = Momentum();
      q_stage[k] = q_last[k].value;
    }


    if (vanishing_coefficients_ != kNone) {
      should_synchronize = at_end ||
                           (parameters.sampling_period != 0 &&
                            sampling_phase % parameters.sampling_period == 0);
    }

    if (vanishing_coefficients_ == kFirstBVanishes &&
        q_and_p_are_synchronized) {
      // Desynchronize.
      std::swap(Δqstage_current, Δqstage_previous);
      for (int k = 0; k < dimension; ++k) {
        p_stage[k] = p_last[k].value;
      }
      ADVANCE_ΔQSTAGE(first_same_as_last_->first * h);
      q_and_p_are_synchronized = false;
    }
    for (int i = 0; i < stages_; ++i) {
      std::swap(Δqstage_current, Δqstage_previous);
      std::swap(Δpstage_current, Δpstage_previous);

      // Beware, the p/q order matters here, the two computations depend on one
      // another.

      // By using |tn.error| below we get a time value which is possibly a wee
      // bit more precise.
      if (vanishing_coefficients_ == kLastAVanishes &&
          q_and_p_are_synchronized && i == 0) {
        ADVANCE_ΔPSTAGE(first_same_as_last_->first * h,
                        tn.value);
        q_and_p_are_synchronized = false;
      } else {
        ADVANCE_ΔPSTAGE(b_[i] * h, tn.value + (tn.error + c_[i] * h));
      }

      if (vanishing_coefficients_ == kFirstBVanishes &&
          should_synchronize && i == stages_ - 1) {
        ADVANCE_ΔQSTAGE(first_same_as_last_->last * h);
        q_and_p_are_synchronized = true;
      } else {
        ADVANCE_ΔQSTAGE(a_[i] * h);
      }
    }
    if (vanishing_coefficients_ == kLastAVanishes && should_synchronize) {
      std::swap(Δpstage_current, Δpstage_previous);
      // TODO(egg): the second parameter below is really just tn.value + h.
      ADVANCE_ΔPSTAGE(first_same_as_last_->last * h,
                      tn.value + h);
      q_and_p_are_synchronized = true;
    }
    // Compensated summation from "'SymplecticPartitionedRungeKutta' Method
    // for NDSolve", algorithm 2.
    for (int k = 0; k < dimension; ++k) {
      q_last[k].Increment((*Δqstage_current)[k]);
      p_last[k].Increment((*Δpstage_current)[k]);
      q_stage[k] = q_last[k].value;
      p_stage[k] = p_last[k].value;
    }
    tn.Increment(h);

    if (parameters.sampling_period != 0) {
      if (sampling_phase % parameters.sampling_period == 0) {
        solution->emplace_back();
        SystemState* state = &solution->back();
        state->time = tn;
        state->positions.reserve(dimension);
        state->momenta.reserve(dimension);
        for (int k = 0; k < dimension; ++k) {
          state->positions.emplace_back(q_last[k]);
          state->momenta.emplace_back(p_last[k]);
        }
      }
      ++sampling_phase;
    }
  }

  if (parameters.sampling_period == 0) {
    solution->emplace_back();
    SystemState* state = &solution->back();
    state->time = tn;
    state->positions.reserve(dimension);
    state->momenta.reserve(dimension);
    for (int k = 0; k < dimension; ++k) {
      state->positions.emplace_back(q_last[k]);
      state->momenta.emplace_back(p_last[k]);
    }
  }
}

}  // namespace integrators
}  // namespace principia

#undef ADVANCE_ΔQSTAGE
#undef ADVANCE_ΔPSTAGE
