// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_LOGIN_LOCK_WEBUI_SCREEN_LOCKER_H_
#define CHROME_BROWSER_CHROMEOS_LOGIN_LOCK_WEBUI_SCREEN_LOCKER_H_

#include <string>

#include "ash/shell_delegate.h"
#include "ash/wm/lock_state_observer.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "chrome/browser/chromeos/login/lock/screen_locker_delegate.h"
#include "chrome/browser/chromeos/login/signin_screen_controller.h"
#include "chrome/browser/chromeos/login/signin_specifics.h"
#include "chrome/browser/chromeos/login/ui/lock_window.h"
#include "chrome/browser/chromeos/login/ui/login_display.h"
#include "chrome/browser/chromeos/login/ui/webui_login_view.h"
#include "chromeos/dbus/power_manager_client.h"
#include "ui/keyboard/keyboard_controller_observer.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_observer.h"

namespace content {
class WebUI;
}

namespace chromeos {

class OobeUI;
class ScreenLocker;
class WebUILoginDisplay;

namespace login {
class NetworkStateHelper;
}

namespace test {
class WebUIScreenLockerTester;
}

// This version of ScreenLockerDelegate displays a WebUI lock screen based on
// the Oobe account picker screen.
class WebUIScreenLocker : public WebUILoginView,
                          public LoginDisplay::Delegate,
                          public ScreenLockerDelegate,
                          public LockWindow::Observer,
#if !defined(USE_ATHENA)
                          public ash::LockStateObserver,
#endif
                          public views::WidgetObserver,
                          public PowerManagerClient::Observer,
#if !defined(USE_ATHENA)
                          public ash::VirtualKeyboardStateObserver,
#endif
                          public keyboard::KeyboardControllerObserver {
 public:
  explicit WebUIScreenLocker(ScreenLocker* screen_locker);

  // ScreenLockerDelegate implementation.
  void LockScreen() override;
  void ScreenLockReady() override;
  void OnAuthenticate() override;
  void SetInputEnabled(bool enabled) override;
  void ShowErrorMessage(int error_msg_id,
                        HelpAppLauncher::HelpTopic help_topic_id) override;
  void ClearErrors() override;
  void AnimateAuthenticationSuccess() override;
  gfx::NativeWindow GetNativeWindow() const override;
  content::WebUI* GetAssociatedWebUI() override;
  void OnLockWebUIReady() override;
  void OnLockBackgroundDisplayed() override;

  // LoginDisplay::Delegate: implementation
  void CancelPasswordChangedFlow() override;
  void CreateAccount() override;
  void CompleteLogin(const UserContext& user_context) override;
  base::string16 GetConnectedNetworkName() override;
  bool IsSigninInProgress() const override;
  void Login(const UserContext& user_context,
             const SigninSpecifics& specifics) override;
  void MigrateUserData(const std::string& old_password) override;
  void OnSigninScreenReady() override;
  void OnStartEnterpriseEnrollment() override;
  void OnStartEnableDebuggingScreen() override;
  void OnStartKioskEnableScreen() override;
  void OnStartKioskAutolaunchScreen() override;
  void ShowWrongHWIDScreen() override;
  void ResetPublicSessionAutoLoginTimer() override;
  void ResyncUserData() override;
  void SetDisplayEmail(const std::string& email) override;
  void Signout() override;

  // LockWindow::Observer implementation.
  void OnLockWindowReady() override;

#if !defined(USE_ATHENA)
  // LockStateObserver override.
  void OnLockStateEvent(ash::LockStateObserver::EventType event) override;
#endif

  // WidgetObserver override.
  void OnWidgetDestroying(views::Widget* widget) override;

  // PowerManagerClient::Observer overrides:
  void SuspendDone(const base::TimeDelta& sleep_duration) override;
  void LidEventReceived(bool open, const base::TimeTicks& time) override;

  // Overridden from content::WebContentsObserver:
  void RenderProcessGone(base::TerminationStatus status) override;

#if !defined(USE_ATHENA)
  // Overridden from ash::KeyboardStateObserver:
  void OnVirtualKeyboardStateChanged(bool activated) override;
#endif

  // Overridden from keyboard::KeyboardControllerObserver:
  void OnKeyboardBoundsChanging(const gfx::Rect& new_bounds) override;

  // Returns instance of the OOBE WebUI.
  OobeUI* GetOobeUI();

 private:
  friend class test::WebUIScreenLockerTester;

  ~WebUIScreenLocker() override;

  // Ensures that user pod is focused.
  void FocusUserPod();

  // The screen locker window.
  views::Widget* lock_window_;

  // Sign-in Screen controller instance (owns login screens).
  scoped_ptr<SignInScreenController> signin_screen_controller_;

  // Login UI implementation instance.
  scoped_ptr<WebUILoginDisplay> login_display_;

  // Tracks when the lock window is displayed and ready.
  bool lock_ready_;

  // Tracks when the WebUI finishes loading.
  bool webui_ready_;

  // Time when lock was initiated, required for metrics.
  base::TimeTicks lock_time_;

  scoped_ptr<login::NetworkStateHelper> network_state_helper_;

  // True is subscribed as keyboard controller observer.
  bool is_observing_keyboard_;

  // The bounds of the virtual keyboard.
  gfx::Rect keyboard_bounds_;

  base::WeakPtrFactory<WebUIScreenLocker> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(WebUIScreenLocker);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_LOGIN_LOCK_WEBUI_SCREEN_LOCKER_H_
