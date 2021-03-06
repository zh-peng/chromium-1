// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_RESOURCES_TILING_SET_RASTER_QUEUE_ALL_H_
#define CC_RESOURCES_TILING_SET_RASTER_QUEUE_ALL_H_

#include "base/containers/stack_container.h"
#include "cc/base/cc_export.h"
#include "cc/resources/picture_layer_tiling_set.h"
#include "cc/resources/tile.h"
#include "cc/resources/tile_priority.h"

namespace cc {

// This queue returns all tiles required to be rasterized from HIGH_RESOLUTION
// and LOW_RESOLUTION tilings.
class CC_EXPORT TilingSetRasterQueueAll {
 public:
  TilingSetRasterQueueAll(PictureLayerTilingSet* tiling_set,
                          bool prioritize_low_res);
  ~TilingSetRasterQueueAll();

  Tile* Top();
  const Tile* Top() const;
  void Pop();
  bool IsEmpty() const;

 private:
  // Helper base class for individual region iterators.
  class OnePriorityRectIterator {
   public:
    OnePriorityRectIterator();
    OnePriorityRectIterator(PictureLayerTiling* tiling,
                            TilingData* tiling_data);

    bool done() const { return !tile_; }
    Tile* operator*() const { return tile_; }

   protected:
    ~OnePriorityRectIterator() = default;
    bool TileNeedsRaster(Tile* tile) const {
      return tile->NeedsRaster() && !tiling_->IsTileOccluded(tile);
    }

    template <typename TilingIteratorType>
    void AdvanceToNextTile(TilingIteratorType* iterator);
    template <typename TilingIteratorType>
    bool GetFirstTileAndCheckIfValid(TilingIteratorType* iterator);

    Tile* tile_;
    PictureLayerTiling* tiling_;
    TilingData* tiling_data_;
  };

  // Iterates over visible rect only, left to right top to bottom order.
  class VisibleTilingIterator : public OnePriorityRectIterator {
   public:
    VisibleTilingIterator() = default;
    VisibleTilingIterator(PictureLayerTiling* tiling, TilingData* tiling_data);

    VisibleTilingIterator& operator++();

   private:
    TilingData::Iterator iterator_;
  };

  class PendingVisibleTilingIterator : public OnePriorityRectIterator {
   public:
    PendingVisibleTilingIterator() = default;
    PendingVisibleTilingIterator(PictureLayerTiling* tiling,
                                 TilingData* tiling_data);

    PendingVisibleTilingIterator& operator++();

   private:
    TilingData::DifferenceIterator iterator_;
  };

  // Iterates over skewport only, spiral around the visible rect.
  class SkewportTilingIterator : public OnePriorityRectIterator {
   public:
    SkewportTilingIterator() = default;
    SkewportTilingIterator(PictureLayerTiling* tiling, TilingData* tiling_data);

    SkewportTilingIterator& operator++();

   private:
    TilingData::SpiralDifferenceIterator iterator_;
    gfx::Rect pending_visible_rect_;
  };

  // Iterates over soon border only, spiral around the visible rect.
  class SoonBorderTilingIterator : public OnePriorityRectIterator {
   public:
    SoonBorderTilingIterator() = default;
    SoonBorderTilingIterator(PictureLayerTiling* tiling,
                             TilingData* tiling_data);

    SoonBorderTilingIterator& operator++();

   private:
    TilingData::SpiralDifferenceIterator iterator_;
    gfx::Rect pending_visible_rect_;
  };

  // Iterates over eventually rect only, spiral around the soon rect.
  class EventuallyTilingIterator : public OnePriorityRectIterator {
   public:
    EventuallyTilingIterator() = default;
    EventuallyTilingIterator(PictureLayerTiling* tiling,
                             TilingData* tiling_data);

    EventuallyTilingIterator& operator++();

   private:
    TilingData::SpiralDifferenceIterator iterator_;
    gfx::Rect pending_visible_rect_;
  };

  // Iterates over all of the above phases in the following order: visible,
  // skewport, soon border, eventually.
  class TilingIterator {
   public:
    TilingIterator();
    explicit TilingIterator(PictureLayerTiling* tiling,
                            TilingData* tiling_data);
    ~TilingIterator();

    bool done() const { return current_tile_ == nullptr; }
    const Tile* operator*() const { return current_tile_; }
    Tile* operator*() { return current_tile_; }
    TilePriority::PriorityBin type() const {
      switch (phase_) {
        case VISIBLE_RECT:
          return TilePriority::NOW;
        case PENDING_VISIBLE_RECT:
        case SKEWPORT_RECT:
        case SOON_BORDER_RECT:
          return TilePriority::SOON;
        case EVENTUALLY_RECT:
          return TilePriority::EVENTUALLY;
      }
      NOTREACHED();
      return TilePriority::EVENTUALLY;
    }

    TilingIterator& operator++();

   private:
    // PENDING VISIBLE RECT refers to the visible rect that will become current
    // upon activation (ie, the pending tree's visible rect). Tiles in this
    // region that are not part of the current visible rect are all handled
    // here. Note that when processing a pending tree, this rect is the same as
    // the visible rect so no tiles are processed in this case.
    enum Phase {
      VISIBLE_RECT,
      PENDING_VISIBLE_RECT,
      SKEWPORT_RECT,
      SOON_BORDER_RECT,
      EVENTUALLY_RECT
    };

    void AdvancePhase();

    PictureLayerTiling* tiling_;
    TilingData* tiling_data_;

    Phase phase_;

    Tile* current_tile_;
    VisibleTilingIterator visible_iterator_;
    PendingVisibleTilingIterator pending_visible_iterator_;
    SkewportTilingIterator skewport_iterator_;
    SoonBorderTilingIterator soon_border_iterator_;
    EventuallyTilingIterator eventually_iterator_;
  };

  enum IteratorType {
    LOW_RES,
    HIGH_RES,
    ACTIVE_NON_IDEAL_PENDING_HIGH_RES,
    NUM_ITERATORS
  };

  void AdvanceToNextStage();

  PictureLayerTilingSet* tiling_set_;

  struct IterationStage {
    IterationStage(IteratorType type, TilePriority::PriorityBin bin);
    IteratorType iterator_type;
    TilePriority::PriorityBin tile_type;
  };

  size_t current_stage_;

  // The max number of stages is 6: 1 low res, 3 high res, and 2 active non
  // ideal pending high res.
  base::StackVector<IterationStage, 6> stages_;
  TilingIterator iterators_[NUM_ITERATORS];
};

}  // namespace cc

#endif  // CC_RESOURCES_TILING_SET_RASTER_QUEUE_ALL_H_
