﻿
#pragma once

#include <experimental/optional>
#include <string>

#include "astronomy/frames.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "physics/ephemeris.hpp"
#include "physics/kepler_orbit.hpp"
#include "physics/rigid_motion.hpp"
#include "physics/solar_system.hpp"
#include "testing_utilities/numerics.hpp"
#include "testing_utilities/solar_system_factory.hpp"

namespace principia {

using geometry::AngleBetween;
using geometry::BarycentreCalculator;
using geometry::Bivector;
using geometry::Commutator;
using geometry::Rotation;
using geometry::Velocity;
using geometry::Wedge;
using physics::DegreesOfFreedom;
using physics::Ephemeris;
using physics::KeplerianElements;
using physics::KeplerOrbit;
using physics::RelativeDegreesOfFreedom;
using physics::RigidMotion;
using physics::RotatingBody;
using physics::SolarSystem;
using quantities::AngularMomentum;
using quantities::GravitationalParameter;
using quantities::si::Metre;
using quantities::si::Milli;
using quantities::si::Minute;
using quantities::si::Radian;
using testing_utilities::SolarSystemFactory;
using testing_utilities::AbsoluteError;
using ::testing::AllOf;
using ::testing::Gt;
using ::testing::Lt;

namespace astronomy {

class SolarSystemDynamicsTest : public testing::Test {
 protected:
  struct OrbitError {
    Angle separation_per_orbit;
    Angle inclination_drift_per_orbit;
    std::experimental::optional<Angle>
        longitude_of_ascending_node_drift_per_orbit;
  };

  SolarSystemDynamicsTest() {
#if 1
    google::LogToStderr();
#endif
    for (int primary = 0; primary <= SolarSystemFactory::LastBody; ++primary) {
      for (int i = 1; i <= SolarSystemFactory::LastBody; ++i) {
        if (SolarSystemFactory::parent(i) == primary) {
          bodies_orbiting_[primary].emplace_back(i);
        }
      }
    }
  }

  OrbitError CompareOrbits(
      int const index,
      Ephemeris<ICRFJ2000Equator> const& ephemeris,
      SolarSystem<ICRFJ2000Equator> const& system,
      SolarSystem<ICRFJ2000Equator> const& expected_system) {
    Instant const& epoch = expected_system.epoch();
    Time const duration = epoch - system.epoch();

    auto const name = SolarSystemFactory::name(index);
    auto const parent_name =
        SolarSystemFactory::name(SolarSystemFactory::parent(index));
    auto const body = system.massive_body(ephemeris, name);
    auto const* const parent =
        dynamic_cast<RotatingBody<ICRFJ2000Equator> const*>(
            &*system.massive_body(ephemeris, parent_name));

    BarycentreCalculator<DegreesOfFreedom<ICRFJ2000Equator>,
                         GravitationalParameter> actual_subsystem_barycentre;
    BarycentreCalculator<DegreesOfFreedom<ICRFJ2000Equator>,
                         GravitationalParameter> expected_subsystem_barycentre;

    actual_subsystem_barycentre.Add(
        ephemeris.trajectory(body)->
            EvaluateDegreesOfFreedom(epoch, /*hint=*/nullptr),
        body->gravitational_parameter());
    expected_subsystem_barycentre.Add(expected_system.initial_state(name),
                                      body->gravitational_parameter());
    for (int const moon_index : bodies_orbiting_[index]) {
      auto const moon_name = SolarSystemFactory::name(moon_index);
      auto const moon = system.massive_body(ephemeris, moon_name);
      actual_subsystem_barycentre.Add(
          ephemeris.trajectory(moon)->
              EvaluateDegreesOfFreedom(epoch, /*hint=*/nullptr),
          moon->gravitational_parameter());
      expected_subsystem_barycentre.Add(
          expected_system.initial_state(moon_name),
          moon->gravitational_parameter());
    }

    auto const actual_dof = actual_subsystem_barycentre.Get();
    auto const expected_dof = expected_subsystem_barycentre.Get();

    auto const actual_parent_dof =
        ephemeris.trajectory(parent)->
            EvaluateDegreesOfFreedom(epoch, /*hint=*/nullptr);
    auto const expected_parent_dof =
        expected_system.initial_state(parent_name);

    // We transform to a frame in which |parent| has the z-axis as its rotation
    // axis by rotating around the normal to Earth's and |parent|'s rotation
    // axes.
    // If |parent| is the Sun, we use the normal to the invariable plane instead
    // of the Sun's axis.
    // TODO(egg): perhaps rotating bodies should export a rotation to their
    // celestial reference frame, we'll use that in the plugin too.
    enum LocalFrameTag { tag };
    using ParentEquator = Frame<LocalFrameTag, tag, /*frame_is_inertial=*/true>;
    auto const z = Bivector<double, ICRFJ2000Equator>({0, 0, 1});
    std::experimental::optional<Rotation<ICRFJ2000Equator, ParentEquator>>
        rotation;

    if (SolarSystemFactory::parent(index) == SolarSystemFactory::Sun) {
      Bivector<AngularMomentum, ICRFJ2000Equator>
          solar_system_angular_momentum;
      for (int i = 1; i <= SolarSystemFactory::LastBody; ++i) {
        auto const body_name = SolarSystemFactory::name(i);
        auto const body = system.massive_body(ephemeris, body_name);
        RelativeDegreesOfFreedom<ICRFJ2000Equator> const
            from_solar_system_barycentre =
                expected_system.initial_state(body_name) -
                DegreesOfFreedom<ICRFJ2000Equator>(ICRFJ2000Equator::origin,
                                                   {});
        solar_system_angular_momentum +=
            Wedge(from_solar_system_barycentre.displacement(),
                  body->mass() * from_solar_system_barycentre.velocity()) *
            Radian;
      }
      auto const normal =
          Commutator(Normalize(solar_system_angular_momentum), z);
      // Check that we computed the invariable plane properly by computing its
      // angle to the Sun's equator.
      // The actual figure is "almost exactly 6 deg", see Bailey, Batygin, and
      // Brown, Solar Obliquity Induced by Planet Nine,
      // https://arxiv.org/pdf/1607.03963.pdf.
      EXPECT_THAT(AngleBetween(solar_system_angular_momentum,
                               parent->angular_velocity()),
                  AllOf(Gt(5.9 * Degree), Lt(6.0 * Degree)));
      auto const declination_of_invariable_plane =
          AngleBetween(solar_system_angular_momentum, z);
      rotation =
          Rotation<ICRFJ2000Equator, ParentEquator>(
              declination_of_invariable_plane,
              normal);
    } else {
      auto const ω = parent->angular_velocity();
      auto const normal = Commutator(Normalize(ω), z);
      auto const parent_axis_declination = AngleBetween(ω, z);
      rotation =
          Rotation<ICRFJ2000Equator, ParentEquator>(parent_axis_declination,
                                                    normal);
    }
    RigidMotion<ICRFJ2000Equator, ParentEquator> const
        to_parent_equator(
            {ICRFJ2000Equator::origin,
             ParentEquator::origin,
             rotation->Forget()},
            /*angular_velocity_of_to_frame=*/{},
            /*velocity_of_to_frame_origin=*/{});

    KeplerOrbit<ParentEquator> actual_osculating_orbit(
        /*primary=*/*parent,
        /*secondary=*/*body,
        /*state_vectors=*/to_parent_equator(actual_dof) -
            to_parent_equator(actual_parent_dof),
        epoch);
    KeplerOrbit<ParentEquator> expected_osculating_orbit(
        /*primary=*/*parent,
        /*secondary=*/*body,
        /*state_vectors=*/to_parent_equator(expected_dof) -
            to_parent_equator(expected_parent_dof),
        epoch);
    KeplerianElements<ParentEquator> const& actual_elements =
        actual_osculating_orbit.elements_at_epoch();
    KeplerianElements<ParentEquator> const& expected_elements =
        expected_osculating_orbit.elements_at_epoch();

    Time const period = 2 * π * Radian / *actual_elements.mean_motion;
    double const orbits = duration / period;

    OrbitError result;
    result.separation_per_orbit =
        AngleBetween(actual_dof.position() - actual_parent_dof.position(),
                     expected_dof.position() - expected_parent_dof.position()) /
        orbits;
    result.inclination_drift_per_orbit =
        AbsoluteError(expected_elements.inclination,
                      actual_elements.inclination) / orbits;
    if (actual_elements.inclination > 0.1 * Degree) {
      result.longitude_of_ascending_node_drift_per_orbit =
          AbsoluteError(expected_elements.longitude_of_ascending_node,
                        actual_elements.longitude_of_ascending_node) / orbits;
    }
    return result;
  }

  std::map<int, std::vector<int>> bodies_orbiting_;
};

TEST_F(SolarSystemDynamicsTest, TwentyDaysFromJ2000) {
  SolarSystem<ICRFJ2000Equator> solar_system_at_j2000;
  solar_system_at_j2000.Initialize(
      SOLUTION_DIR / "astronomy" / "gravity_model.proto.txt",
      SOLUTION_DIR / "astronomy" /
          "initial_state_jd_2451545_000000000.proto.txt");

  SolarSystem<ICRFJ2000Equator> twenty_days_later;
  twenty_days_later.Initialize(
      SOLUTION_DIR / "astronomy" / "gravity_model.proto.txt",
      SOLUTION_DIR / "astronomy" /
          "initial_state_jd_2451564_587154910.proto.txt");

  auto const ephemeris =
      solar_system_at_j2000.MakeEphemeris(
          /*fitting_tolerance=*/5 * Milli(Metre),
          Ephemeris<ICRFJ2000Equator>::FixedStepParameters(
              integrators::BlanesMoan2002SRKN14A<Position<ICRFJ2000Equator>>(),
              /*step=*/10 * Minute));
  ephemeris->Prolong(twenty_days_later.epoch());

  for (int const planet_or_minor_planet :
       bodies_orbiting_[SolarSystemFactory::Sun]) {
    LOG(INFO) << "=== " << SolarSystemFactory::name(planet_or_minor_planet);
    auto const error = CompareOrbits(planet_or_minor_planet,
                                     *ephemeris,
                                     solar_system_at_j2000,
                                     twenty_days_later);
    LOG(INFO) << "separation = " << std::fixed
              << error.separation_per_orbit / ArcSecond << u8"″/orbit";
    LOG(INFO) << u8"Δi         = " << std::fixed
              << error.inclination_drift_per_orbit / ArcSecond << u8"″/orbit";
    if (error.longitude_of_ascending_node_drift_per_orbit) {
      LOG(INFO) << u8"ΔΩ         = " << std::fixed
                << *error.longitude_of_ascending_node_drift_per_orbit /
                       ArcSecond
                << u8"″/orbit";
    }
  }
}

TEST_F(SolarSystemDynamicsTest, Спутник1ToСпутник2) {
}

}  // namespace astronomy
}  // namespace principia
