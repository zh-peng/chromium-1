// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/test/display_manager_test_api.h"

#include <vector>

#include "ash/display/display_info.h"
#include "ash/display/display_manager.h"
#include "ash/shell.h"
#include "base/strings/string_split.h"
#include "ui/aura/window_event_dispatcher.h"
#include "ui/gfx/display.h"

namespace ash {
namespace test {
typedef std::vector<gfx::Display> DisplayList;
typedef DisplayInfo DisplayInfo;
typedef std::vector<DisplayInfo> DisplayInfoList;

namespace {

std::vector<DisplayInfo> CreateDisplayInfoListFromString(
    const std::string specs,
    DisplayManager* display_manager) {
  std::vector<DisplayInfo> display_info_list;
  std::vector<std::string> parts;
  base::SplitString(specs, ',', &parts);
  size_t index = 0;
  for (std::vector<std::string>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter, ++index) {
    int64 id = index < display_manager->GetNumDisplays() ?
        display_manager->GetDisplayAt(index).id() :
        gfx::Display::kInvalidDisplayID;
    display_info_list.push_back(
        DisplayInfo::CreateFromSpecWithID(*iter, id));
  }
  return display_info_list;
}

}  // namespace

DisplayManagerTestApi::DisplayManagerTestApi(DisplayManager* display_manager)
    : display_manager_(display_manager) {}

DisplayManagerTestApi::~DisplayManagerTestApi() {}

void DisplayManagerTestApi::UpdateDisplay(
    const std::string& display_specs) {
  std::vector<DisplayInfo> display_info_list =
      CreateDisplayInfoListFromString(display_specs, display_manager_);
  bool is_host_origin_set = false;
  for (size_t i = 0; i < display_info_list.size(); ++i) {
    const DisplayInfo& display_info = display_info_list[i];
    if (display_info.bounds_in_native().origin() != gfx::Point(0, 0)) {
      is_host_origin_set = true;
      break;
    }
  }

  // On non-testing environment, when a secondary display is connected, a new
  // native (i.e. X) window for the display is always created below the
  // previous one for GPU performance reasons. Try to emulate the behavior
  // unless host origins are explicitly set.
  if (!is_host_origin_set) {
    // Sart from (1,1) so that windows won't overlap with native mouse cursor.
    // See |AshTestBase::SetUp()|.
    int next_y = 1;
    for (std::vector<DisplayInfo>::iterator iter = display_info_list.begin();
         iter != display_info_list.end(); ++iter) {
      gfx::Rect bounds(iter->bounds_in_native().size());
      bounds.set_x(1);
      bounds.set_y(next_y);
      next_y += bounds.height();
      iter->SetBounds(bounds);
    }
  }

  display_manager_->OnNativeDisplaysChanged(display_info_list);
  display_manager_->UpdateInternalDisplayModeListForTest();
}

int64 DisplayManagerTestApi::SetFirstDisplayAsInternalDisplay() {
  const gfx::Display& internal = display_manager_->active_display_list_[0];
  SetInternalDisplayId(internal.id());
  return gfx::Display::InternalDisplayId();
}

void DisplayManagerTestApi::SetInternalDisplayId(int64 id) {
  gfx::Display::SetInternalDisplayId(id);
  display_manager_->UpdateInternalDisplayModeListForTest();
}

void DisplayManagerTestApi::DisableChangeDisplayUponHostResize() {
  display_manager_->set_change_display_upon_host_resize(false);
}

void DisplayManagerTestApi::SetAvailableColorProfiles(
    int64 display_id,
    const std::vector<ui::ColorCalibrationProfile>& profiles) {
  display_manager_->display_info_[display_id].set_available_color_profiles(
      profiles);
}

}  // namespace test
}  // namespace ash
