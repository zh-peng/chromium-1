// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "cc/resources/tiling_set_eviction_queue.h"

namespace cc {
namespace {

bool IsSharedOutOfOrderTile(WhichTree tree, const Tile* tile) {
  if (!tile->is_shared())
    return false;

  // The priority for tile priority of a shared tile will be a combined
  // priority thus return shared tiles from a higher priority tree as
  // it is out of order for a lower priority tree.
  WhichTree twin_tree = tree == ACTIVE_TREE ? PENDING_TREE : ACTIVE_TREE;
  const TilePriority& priority = tile->priority(tree);
  const TilePriority& twin_priority = tile->priority(twin_tree);
  if (priority.priority_bin != twin_priority.priority_bin)
    return priority.priority_bin > twin_priority.priority_bin;
  const bool occluded = tile->is_occluded(tree);
  const bool twin_occluded = tile->is_occluded(twin_tree);
  if (occluded != twin_occluded)
    return occluded;
  if (priority.distance_to_visible != twin_priority.distance_to_visible)
    return priority.distance_to_visible > twin_priority.distance_to_visible;

  // If priorities are the same, it does not matter which tree returns
  // the tile. Let's pick the pending tree.
  return tree != PENDING_TREE;
}

}  // namespace

TilingSetEvictionQueue::TilingSetEvictionQueue(
    PictureLayerTilingSet* tiling_set,
    bool skip_shared_out_of_order_tiles)
    : tree_(tiling_set->client()->GetTree()),
      skip_shared_out_of_order_tiles_(skip_shared_out_of_order_tiles),
      phase_(EVENTUALLY_RECT),
      current_tile_(nullptr) {
  // Early out if the layer has no tilings.
  if (!tiling_set->num_tilings())
    return;
  GenerateTilingOrder(tiling_set);
  eventually_iterator_ = EventuallyTilingIterator(
      &tilings_, tree_, skip_shared_out_of_order_tiles_);
  if (eventually_iterator_.done()) {
    AdvancePhase();
    return;
  }
  current_tile_ = *eventually_iterator_;
}

TilingSetEvictionQueue::~TilingSetEvictionQueue() {
}

void TilingSetEvictionQueue::GenerateTilingOrder(
    PictureLayerTilingSet* tiling_set) {
  tilings_.reserve(tiling_set->num_tilings());
  // Generate all of the tilings in the order described in the header comment
  // for this class.
  auto range =
      tiling_set->GetTilingRange(PictureLayerTilingSet::HIGHER_THAN_HIGH_RES);
  for (int i = range.start; i < range.end; ++i)
    tilings_.push_back(tiling_set->tiling_at(i));

  range = tiling_set->GetTilingRange(PictureLayerTilingSet::LOWER_THAN_LOW_RES);
  for (int i = range.end - 1; i >= range.start; --i)
    tilings_.push_back(tiling_set->tiling_at(i));

  range = tiling_set->GetTilingRange(
      PictureLayerTilingSet::BETWEEN_HIGH_AND_LOW_RES);
  for (int i = range.end - 1; i >= range.start; --i)
    tilings_.push_back(tiling_set->tiling_at(i));

  range = tiling_set->GetTilingRange(PictureLayerTilingSet::LOW_RES);
  for (int i = range.start; i < range.end; ++i)
    tilings_.push_back(tiling_set->tiling_at(i));

  range = tiling_set->GetTilingRange(PictureLayerTilingSet::HIGH_RES);
  for (int i = range.start; i < range.end; ++i)
    tilings_.push_back(tiling_set->tiling_at(i));
  DCHECK_EQ(tiling_set->num_tilings(), tilings_.size());
}

void TilingSetEvictionQueue::AdvancePhase() {
  current_tile_ = nullptr;
  while (!current_tile_ &&
         phase_ != VISIBLE_RECT_REQUIRED_FOR_ACTIVATION_UNOCCLUDED) {
    phase_ = static_cast<Phase>(phase_ + 1);
    switch (phase_) {
      case EVENTUALLY_RECT:
        NOTREACHED();
        break;
      case SOON_BORDER_RECT:
        soon_iterator_ = SoonBorderTilingIterator(
            &tilings_, tree_, skip_shared_out_of_order_tiles_);
        if (!soon_iterator_.done())
          current_tile_ = *soon_iterator_;
        break;
      case SKEWPORT_RECT:
        skewport_iterator_ = SkewportTilingIterator(
            &tilings_, tree_, skip_shared_out_of_order_tiles_);
        if (!skewport_iterator_.done())
          current_tile_ = *skewport_iterator_;
        break;
      case VISIBLE_RECT_OCCLUDED:
        visible_iterator_ = VisibleTilingIterator(
            &tilings_, tree_, skip_shared_out_of_order_tiles_,
            true /* return occluded tiles */,
            false /* return required for activation tiles */);
        if (!visible_iterator_.done())
          current_tile_ = *visible_iterator_;
        break;
      case VISIBLE_RECT_UNOCCLUDED:
        visible_iterator_ = VisibleTilingIterator(
            &tilings_, tree_, skip_shared_out_of_order_tiles_,
            false /* return occluded tiles */,
            false /* return required for activation tiles */);
        if (!visible_iterator_.done())
          current_tile_ = *visible_iterator_;
        break;
      case VISIBLE_RECT_REQUIRED_FOR_ACTIVATION_OCCLUDED:
        visible_iterator_ = VisibleTilingIterator(
            &tilings_, tree_, skip_shared_out_of_order_tiles_,
            true /* return occluded tiles */,
            true /* return required for activation tiles */);
        if (!visible_iterator_.done())
          current_tile_ = *visible_iterator_;
        break;
      case VISIBLE_RECT_REQUIRED_FOR_ACTIVATION_UNOCCLUDED:
        visible_iterator_ = VisibleTilingIterator(
            &tilings_, tree_, skip_shared_out_of_order_tiles_,
            false /* return occluded tiles */,
            true /* return required for activation tiles */);
        if (!visible_iterator_.done())
          current_tile_ = *visible_iterator_;
        break;
    }
  }
}

bool TilingSetEvictionQueue::IsEmpty() const {
  return !current_tile_;
}

Tile* TilingSetEvictionQueue::Top() {
  DCHECK(!IsEmpty());
  return current_tile_;
}

const Tile* TilingSetEvictionQueue::Top() const {
  DCHECK(!IsEmpty());
  return current_tile_;
}

void TilingSetEvictionQueue::Pop() {
  DCHECK(!IsEmpty());
  current_tile_ = nullptr;
  switch (phase_) {
    case EVENTUALLY_RECT:
      ++eventually_iterator_;
      if (!eventually_iterator_.done())
        current_tile_ = *eventually_iterator_;
      break;
    case SOON_BORDER_RECT:
      ++soon_iterator_;
      if (!soon_iterator_.done())
        current_tile_ = *soon_iterator_;
      break;
    case SKEWPORT_RECT:
      ++skewport_iterator_;
      if (!skewport_iterator_.done())
        current_tile_ = *skewport_iterator_;
      break;
    case VISIBLE_RECT_OCCLUDED:
    case VISIBLE_RECT_UNOCCLUDED:
    case VISIBLE_RECT_REQUIRED_FOR_ACTIVATION_OCCLUDED:
    case VISIBLE_RECT_REQUIRED_FOR_ACTIVATION_UNOCCLUDED:
      ++visible_iterator_;
      if (!visible_iterator_.done())
        current_tile_ = *visible_iterator_;
      break;
  }
  if (!current_tile_)
    AdvancePhase();
}

// EvictionRectIterator
TilingSetEvictionQueue::EvictionRectIterator::EvictionRectIterator()
    : tile_(nullptr),
      tilings_(nullptr),
      tree_(ACTIVE_TREE),
      skip_shared_out_of_order_tiles_(false),
      tiling_index_(0) {
}

TilingSetEvictionQueue::EvictionRectIterator::EvictionRectIterator(
    std::vector<PictureLayerTiling*>* tilings,
    WhichTree tree,
    bool skip_shared_out_of_order_tiles)
    : tile_(nullptr),
      tilings_(tilings),
      tree_(tree),
      skip_shared_out_of_order_tiles_(skip_shared_out_of_order_tiles),
      tiling_index_(0) {
}

template <typename TilingIteratorType>
bool TilingSetEvictionQueue::EvictionRectIterator::AdvanceToNextTile(
    TilingIteratorType* iterator) {
  bool found_tile = false;
  while (!found_tile) {
    ++(*iterator);
    if (!(*iterator)) {
      tile_ = nullptr;
      break;
    }
    found_tile = GetFirstTileAndCheckIfValid(iterator);
  }
  return found_tile;
}

template <typename TilingIteratorType>
bool TilingSetEvictionQueue::EvictionRectIterator::GetFirstTileAndCheckIfValid(
    TilingIteratorType* iterator) {
  tile_ = (*tilings_)[tiling_index_]->TileAt(iterator->index_x(),
                                             iterator->index_y());
  // If there's nothing to evict, return false.
  if (!tile_ || !tile_->HasResource())
    return false;
  (*tilings_)[tiling_index_]->UpdateTileAndTwinPriority(tile_);
  // If the tile is out of order, return false.
  if (skip_shared_out_of_order_tiles_ && IsSharedOutOfOrderTile(tree_, tile_))
    return false;
  // In other cases, the tile we got is a viable candidate, return true.
  return true;
}

// EventuallyTilingIterator
TilingSetEvictionQueue::EventuallyTilingIterator::EventuallyTilingIterator(
    std::vector<PictureLayerTiling*>* tilings,
    WhichTree tree,
    bool skip_shared_out_of_order_tiles)
    : EvictionRectIterator(tilings, tree, skip_shared_out_of_order_tiles) {
  // Find the first tiling with a tile.
  while (tiling_index_ < tilings_->size()) {
    if (!((*tilings_))[tiling_index_]->has_eventually_rect_tiles()) {
      ++tiling_index_;
      continue;
    }
    iterator_ = TilingData::ReverseSpiralDifferenceIterator(
        ((*tilings_))[tiling_index_]->tiling_data(),
        ((*tilings_))[tiling_index_]->current_eventually_rect(),
        ((*tilings_))[tiling_index_]->current_skewport_rect(),
        ((*tilings_))[tiling_index_]->current_soon_border_rect());
    if (!iterator_) {
      ++tiling_index_;
      continue;
    }
    break;
  }
  if (tiling_index_ >= tilings_->size())
    return;
  if (!GetFirstTileAndCheckIfValid(&iterator_))
    ++(*this);
}

TilingSetEvictionQueue::EventuallyTilingIterator&
    TilingSetEvictionQueue::EventuallyTilingIterator::
    operator++() {
  bool found_tile = AdvanceToNextTile(&iterator_);
  while (!found_tile && (tiling_index_ + 1) < tilings_->size()) {
    ++tiling_index_;
    if (!((*tilings_))[tiling_index_]->has_eventually_rect_tiles())
      continue;
    iterator_ = TilingData::ReverseSpiralDifferenceIterator(
        ((*tilings_))[tiling_index_]->tiling_data(),
        ((*tilings_))[tiling_index_]->current_eventually_rect(),
        ((*tilings_))[tiling_index_]->current_skewport_rect(),
        ((*tilings_))[tiling_index_]->current_soon_border_rect());
    if (!iterator_)
      continue;
    found_tile = GetFirstTileAndCheckIfValid(&iterator_);
    if (!found_tile)
      found_tile = AdvanceToNextTile(&iterator_);
  }
  return *this;
}

// SoonBorderTilingIterator
TilingSetEvictionQueue::SoonBorderTilingIterator::SoonBorderTilingIterator(
    std::vector<PictureLayerTiling*>* tilings,
    WhichTree tree,
    bool skip_shared_out_of_order_tiles)
    : EvictionRectIterator(tilings, tree, skip_shared_out_of_order_tiles) {
  // Find the first tiling with a tile.
  while (tiling_index_ < tilings_->size()) {
    if (!((*tilings_))[tiling_index_]->has_soon_border_rect_tiles()) {
      ++tiling_index_;
      continue;
    }
    iterator_ = TilingData::ReverseSpiralDifferenceIterator(
        ((*tilings_))[tiling_index_]->tiling_data(),
        ((*tilings_))[tiling_index_]->current_soon_border_rect(),
        ((*tilings_))[tiling_index_]->current_skewport_rect(),
        ((*tilings_))[tiling_index_]->current_visible_rect());
    if (!iterator_) {
      ++tiling_index_;
      continue;
    }
    break;
  }
  if (tiling_index_ >= tilings_->size())
    return;
  if (!GetFirstTileAndCheckIfValid(&iterator_))
    ++(*this);
}

TilingSetEvictionQueue::SoonBorderTilingIterator&
    TilingSetEvictionQueue::SoonBorderTilingIterator::
    operator++() {
  bool found_tile = AdvanceToNextTile(&iterator_);
  while (!found_tile && (tiling_index_ + 1) < tilings_->size()) {
    ++tiling_index_;
    if (!((*tilings_))[tiling_index_]->has_soon_border_rect_tiles())
      continue;
    iterator_ = TilingData::ReverseSpiralDifferenceIterator(
        ((*tilings_))[tiling_index_]->tiling_data(),
        ((*tilings_))[tiling_index_]->current_soon_border_rect(),
        ((*tilings_))[tiling_index_]->current_skewport_rect(),
        ((*tilings_))[tiling_index_]->current_visible_rect());
    if (!iterator_)
      continue;
    found_tile = GetFirstTileAndCheckIfValid(&iterator_);
    if (!found_tile)
      found_tile = AdvanceToNextTile(&iterator_);
  }
  return *this;
}

// SkewportTilingIterator
TilingSetEvictionQueue::SkewportTilingIterator::SkewportTilingIterator(
    std::vector<PictureLayerTiling*>* tilings,
    WhichTree tree,
    bool skip_shared_out_of_order_tiles)
    : EvictionRectIterator(tilings, tree, skip_shared_out_of_order_tiles) {
  // Find the first tiling with a tile.
  while (tiling_index_ < tilings_->size()) {
    if (!((*tilings_))[tiling_index_]->has_skewport_rect_tiles()) {
      ++tiling_index_;
      continue;
    }
    iterator_ = TilingData::ReverseSpiralDifferenceIterator(
        ((*tilings_))[tiling_index_]->tiling_data(),
        ((*tilings_))[tiling_index_]->current_skewport_rect(),
        ((*tilings_))[tiling_index_]->current_visible_rect(),
        ((*tilings_))[tiling_index_]->current_visible_rect());
    if (!iterator_) {
      ++tiling_index_;
      continue;
    }
    break;
  }
  if (tiling_index_ >= tilings_->size())
    return;
  if (!GetFirstTileAndCheckIfValid(&iterator_))
    ++(*this);
}

TilingSetEvictionQueue::SkewportTilingIterator&
    TilingSetEvictionQueue::SkewportTilingIterator::
    operator++() {
  bool found_tile = AdvanceToNextTile(&iterator_);
  while (!found_tile && (tiling_index_ + 1) < tilings_->size()) {
    ++tiling_index_;
    if (!((*tilings_))[tiling_index_]->has_skewport_rect_tiles())
      continue;
    iterator_ = TilingData::ReverseSpiralDifferenceIterator(
        ((*tilings_))[tiling_index_]->tiling_data(),
        ((*tilings_))[tiling_index_]->current_skewport_rect(),
        ((*tilings_))[tiling_index_]->current_visible_rect(),
        ((*tilings_))[tiling_index_]->current_visible_rect());
    if (!iterator_)
      continue;
    found_tile = GetFirstTileAndCheckIfValid(&iterator_);
    if (!found_tile)
      found_tile = AdvanceToNextTile(&iterator_);
  }
  return *this;
}

// VisibleTilingIterator
TilingSetEvictionQueue::VisibleTilingIterator::VisibleTilingIterator(
    std::vector<PictureLayerTiling*>* tilings,
    WhichTree tree,
    bool skip_shared_out_of_order_tiles,
    bool return_occluded_tiles,
    bool return_required_for_activation_tiles)
    : EvictionRectIterator(tilings, tree, skip_shared_out_of_order_tiles),
      return_occluded_tiles_(return_occluded_tiles),
      return_required_for_activation_tiles_(
          return_required_for_activation_tiles) {
  // Find the first tiling with a tile.
  while (tiling_index_ < tilings_->size()) {
    if (!((*tilings_))[tiling_index_]->has_visible_rect_tiles()) {
      ++tiling_index_;
      continue;
    }
    iterator_ = TilingData::Iterator(
        ((*tilings_))[tiling_index_]->tiling_data(),
        ((*tilings_))[tiling_index_]->current_visible_rect(), false);
    if (!iterator_) {
      ++tiling_index_;
      continue;
    }
    break;
  }
  if (tiling_index_ >= tilings_->size())
    return;
  if (!GetFirstTileAndCheckIfValid(&iterator_)) {
    ++(*this);
    return;
  }
  if (!TileMatchesRequiredFlags(tile_)) {
    ++(*this);
    return;
  }
}

TilingSetEvictionQueue::VisibleTilingIterator&
    TilingSetEvictionQueue::VisibleTilingIterator::
    operator++() {
  bool found_tile = AdvanceToNextTile(&iterator_);
  while (found_tile && !TileMatchesRequiredFlags(tile_))
    found_tile = AdvanceToNextTile(&iterator_);

  while (!found_tile && (tiling_index_ + 1) < tilings_->size()) {
    ++tiling_index_;
    if (!((*tilings_))[tiling_index_]->has_visible_rect_tiles())
      continue;
    iterator_ = TilingData::Iterator(
        ((*tilings_))[tiling_index_]->tiling_data(),
        ((*tilings_))[tiling_index_]->current_visible_rect(), false);
    if (!iterator_)
      continue;
    found_tile = GetFirstTileAndCheckIfValid(&iterator_);
    if (!found_tile)
      found_tile = AdvanceToNextTile(&iterator_);
    while (found_tile && !TileMatchesRequiredFlags(tile_))
      found_tile = AdvanceToNextTile(&iterator_);
  }
  return *this;
}

bool TilingSetEvictionQueue::VisibleTilingIterator::TileMatchesRequiredFlags(
    const Tile* tile) const {
  bool activation_flag_matches =
      tile->required_for_activation() == return_required_for_activation_tiles_;
  bool occluded_flag_matches =
      tile->is_occluded(tree_) == return_occluded_tiles_;
  return activation_flag_matches && occluded_flag_matches;
}

}  // namespace cc
