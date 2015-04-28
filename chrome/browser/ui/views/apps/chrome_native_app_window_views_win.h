// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_APPS_CHROME_NATIVE_APP_WINDOW_VIEWS_WIN_H_
#define CHROME_BROWSER_UI_VIEWS_APPS_CHROME_NATIVE_APP_WINDOW_VIEWS_WIN_H_

#include "chrome/browser/ui/views/apps/chrome_native_app_window_views_aura.h"

namespace web_app {
struct ShortcutInfo;
}

class GlassAppWindowFrameViewWin;

// Windows-specific parts of the views-backed native shell window implementation
// for packaged apps.
class ChromeNativeAppWindowViewsWin : public ChromeNativeAppWindowViewsAura {
 public:
  ChromeNativeAppWindowViewsWin();

  GlassAppWindowFrameViewWin* glass_frame_view() {
    return glass_frame_view_;
  }

 private:
  void ActivateParentDesktopIfNecessary();

  void OnShortcutInfoLoaded(
      const web_app::ShortcutInfo& shortcut_info);

  HWND GetNativeAppWindowHWND() const;
  bool IsRunningInAsh();
  void EnsureCaptionStyleSet();

  // Overridden from ChromeNativeAppWindowViews:
  void OnBeforeWidgetInit(
      const extensions::AppWindow::CreateParams& create_params,
      views::Widget::InitParams* init_params,
      views::Widget* widget) override;
  void InitializeDefaultWindow(
      const extensions::AppWindow::CreateParams& create_params) override;
  views::NonClientFrameView* CreateStandardDesktopAppFrame() override;

  // Overridden from ui::BaseWindow:
  void Show() override;
  void Activate() override;

  // Overridden from views::WidgetDelegate:
  bool CanMinimize() const override;

  // Overridden from extensions::NativeAppWindow:
  void UpdateShelfMenu() override;

  // Populated if there is a glass frame, which provides special information
  // to the native widget implementation. This will be NULL if there is no
  // glass frame. Note, this can change from NULL to non-NULL and back again
  // throughout the life of a window, e.g. if DWM is enabled and disabled.
  GlassAppWindowFrameViewWin* glass_frame_view_;

  // The Windows Application User Model ID identifying the app.
  // Not set for windows running inside Ash.
  base::string16 app_model_id_;

  // Whether the InitParams indicated that this window should be translucent.
  bool is_translucent_;

  base::WeakPtrFactory<ChromeNativeAppWindowViewsWin> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(ChromeNativeAppWindowViewsWin);
};

#endif  // CHROME_BROWSER_UI_VIEWS_APPS_CHROME_NATIVE_APP_WINDOW_VIEWS_WIN_H_
