﻿
#pragma once

#include "ksp_plugin/part.hpp"

#include <list>
#include <string>

#include "base/array.hpp"
#include "base/hexadecimal.hpp"
#include "base/not_null.hpp"

namespace principia {
namespace ksp_plugin {
namespace internal_part {

using base::Array;
using base::HexadecimalEncode;
using base::make_not_null_unique;
using base::UniqueBytes;

Part::Part(
    PartId const part_id,
    std::string const& name,
    Mass const& mass,
    DegreesOfFreedom<Barycentric> const& degrees_of_freedom,
    std::function<void()> deletion_callback)
    : part_id_(part_id),
      name_(name),
      mass_(mass),
      degrees_of_freedom_(degrees_of_freedom),
      prehistory_(make_not_null_unique<DiscreteTrajectory<Barycentric>>()),
      subset_node_(make_not_null_unique<Subset<Part>::Node>()),
      deletion_callback_(std::move(deletion_callback)) {
  CHECK_GT(mass_, Mass{}) << ShortDebugString();
  prehistory_->Append(astronomy::InfinitePast,
                      {Barycentric::origin, Velocity<Barycentric>()});
  history_ = prehistory_->NewForkAtLast();
}

Part::~Part() {
  LOG(INFO) << "Destroying part " << ShortDebugString();
  CHECK(!is_piled_up());
  if (deletion_callback_ != nullptr) {
    deletion_callback_();
  }
}

PartId Part::part_id() const {
  return part_id_;
}

void Part::set_mass(Mass const& mass) {
  CHECK_GT(mass, Mass{}) << ShortDebugString();
  mass_ = mass;
}

Mass const& Part::mass() const {
  return mass_;
}

void Part::clear_intrinsic_force() {
  intrinsic_force_ = Vector<Force, Barycentric>{};
}

void Part::increment_intrinsic_force(
    Vector<Force, Barycentric> const& intrinsic_force) {
  intrinsic_force_ += intrinsic_force;
}

Vector<Force, Barycentric> const& Part::intrinsic_force() const {
  return intrinsic_force_;
}

void Part::set_degrees_of_freedom(
    DegreesOfFreedom<Barycentric> const& degrees_of_freedom) {
  degrees_of_freedom_ = degrees_of_freedom;
}

DegreesOfFreedom<Barycentric> const&
Part::degrees_of_freedom() const {
  return degrees_of_freedom_;
}

DiscreteTrajectory<Barycentric>::Iterator Part::history_begin() {
  // Make sure that we skip the point of the prehistory.
  auto it = history_->Fork();
  return ++it;
}

DiscreteTrajectory<Barycentric>::Iterator Part::history_end() {
  return history_->End();
}

DiscreteTrajectory<Barycentric>::Iterator Part::psychohistory_begin() {
  // Make sure that we skip the fork, which may be the point of the prehistory.
  auto it = psychohistory_->Fork();
  return ++it;
}

DiscreteTrajectory<Barycentric>::Iterator Part::psychohistory_end() {
  return psychohistory_->End();
}

void Part::AppendToHistory(
    Instant const& time,
    DegreesOfFreedom<Barycentric> const& degrees_of_freedom) {
  if (psychohistory_ != nullptr) {
    history_->DeleteFork(psychohistory_);
  }
  history_->Append(time, degrees_of_freedom);
}

void Part::AppendToPsychohistory(
    Instant const& time,
    DegreesOfFreedom<Barycentric> const& degrees_of_freedom) {
  if (psychohistory_ == nullptr) {
    psychohistory_ = history_->NewForkAtLast();
  }
  psychohistory_->Append(time, degrees_of_freedom);
}

void Part::ClearHistory() {
  if (psychohistory_ != nullptr) {
    history_->DeleteFork(psychohistory_);
  }
  prehistory_->DeleteFork(history_);
  history_ = prehistory_->NewForkAtLast();
}

void Part::set_containing_pile_up(IteratorOn<std::list<PileUp>> const pile_up) {
  CHECK(!is_piled_up());
  LOG(INFO) << "Adding part " << ShortDebugString() << " to the pile up at "
            << &*pile_up.iterator();
  containing_pile_up_ = pile_up;
}

std::experimental::optional<IteratorOn<std::list<PileUp>>>
Part::containing_pile_up() const {
  return containing_pile_up_;
}

bool Part::is_piled_up() const {
  // TODO(egg): |has_value()| once we have a standard |optional|.
  return static_cast<bool>(containing_pile_up_);
}

void Part::clear_pile_up() {
  if (is_piled_up()) {
    IteratorOn<std::list<PileUp>> pile_up = *containing_pile_up_;
    for (not_null<Part*> const part : pile_up.iterator()->parts()) {
      LOG(INFO) << "Removing part " << part->ShortDebugString()
                << " from its pile up at " << &*pile_up.iterator();
      part->containing_pile_up_ = std::experimental::nullopt;
    }
    CHECK(!is_piled_up());
    pile_up.Erase();
  }
}

void Part::WriteToMessage(not_null<serialization::Part*> const message) const {
  message->set_part_id(part_id_);
  message->set_name(name_);
  mass_.WriteToMessage(message->mutable_mass());
  intrinsic_force_.WriteToMessage(message->mutable_intrinsic_force());
  if (containing_pile_up_) {
    message->set_containing_pile_up(containing_pile_up_->distance_from_begin());
  }
  degrees_of_freedom_.WriteToMessage(message->mutable_degrees_of_freedom());
  prehistory_->WriteToMessage(message->mutable_prehistory(),
                              /*forks=*/{history_, psychohistory_});
}

not_null<std::unique_ptr<Part>> Part::ReadFromMessage(
    serialization::Part const& message,
    std::function<void()> deletion_callback) {
  bool const is_pre_cesàro = message.has_tail_is_authoritative();
  not_null<std::unique_ptr<Part>> part =
      make_not_null_unique<Part>(message.part_id(),
                                 message.name(),
                                 Mass::ReadFromMessage(message.mass()),
                                 DegreesOfFreedom<Barycentric>::ReadFromMessage(
                                     message.degrees_of_freedom()),
                                 std::move(deletion_callback));
  part->increment_intrinsic_force(
      Vector<Force, Barycentric>::ReadFromMessage(message.intrinsic_force()));
  if (is_pre_cesàro) {
    auto tail = DiscreteTrajectory<Barycentric>::ReadFromMessage(
        message.prehistory(),
        /*forks=*/{});
    // The |prehistory_| and |history_| have been created by the constructor
    // above.  Construct the various trajectories from the |tail|.
    for (auto it = tail->Begin(); it != tail->End(); ++it) {
      if (it == tail->last() && !message.tail_is_authoritative()) {
        part->AppendToPsychohistory(it.time(), it.degrees_of_freedom());
      } else {
        part->AppendToHistory(it.time(), it.degrees_of_freedom());
      }
    }
  } else {
    part->history_ = nullptr;
    part->prehistory_ = DiscreteTrajectory<Barycentric>::ReadFromMessage(
        message.prehistory(),
        /*forks=*/{&part->history_, &part->psychohistory_});
  }
  return part;
}

void Part::FillContainingPileUpFromMessage(
    serialization::Part const& message,
    not_null<std::list<PileUp>*> const pile_ups) {
  if (message.has_containing_pile_up()) {
    auto it = pile_ups->begin();
    std::advance(it, message.containing_pile_up());
    containing_pile_up_ = IteratorOn<std::list<PileUp>>(pile_ups, it);
  }
}

std::string Part::ShortDebugString() const {
  UniqueBytes hex_id(sizeof(part_id_) * 2 + 1);
  Array<std::uint8_t const> id_bytes(
      reinterpret_cast<std::uint8_t const*>(&part_id_), sizeof(part_id_));
  HexadecimalEncode(id_bytes, hex_id.get());
  hex_id.data[sizeof(part_id_) * 2] = '\0';
  return name_ + " (" + reinterpret_cast<char const*>(hex_id.data.get()) + ")";
}

std::ostream& operator<<(std::ostream& out, Part const& part) {
  return out << "{"
             << part.part_id() << ", "
             << part.mass() << "}";
}

}  // namespace internal_part
}  // namespace ksp_plugin
}  // namespace principia
