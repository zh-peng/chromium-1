// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_ACCESSIBILITY_ACCESSIBILITY_EVENT_ROUTER_VIEWS_H_
#define CHROME_BROWSER_UI_VIEWS_ACCESSIBILITY_ACCESSIBILITY_EVENT_ROUTER_VIEWS_H_

#include <string>

#include "base/basictypes.h"
#include "base/gtest_prod_util.h"
#include "base/strings/string16.h"
#include "chrome/browser/accessibility/accessibility_events.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "ui/accessibility/ax_enums.h"

class Profile;

template <typename T> struct DefaultSingletonTraits;

namespace views {
class View;
}

// NOTE: This class is part of the Accessibility Extension API, which lets
// extensions receive accessibility events. It's distinct from code that
// implements platform accessibility APIs like MSAA or ATK.
//
// Singleton class that adds listeners to many views, then sends an
// accessibility notification whenever a relevant event occurs in an
// accessible view.
//
// Views are not accessible by default. When you register a root widget,
// that widget and all of its descendants will start sending accessibility
// event notifications. You can then override the default behavior for
// specific descendants using other methods.
//
// You can use Profile::PauseAccessibilityEvents to prevent a flurry
// of accessibility events when a window is being created or initialized.
class AccessibilityEventRouterViews : public content::NotificationObserver {
 public:
  // Get the single instance of this class.
  static AccessibilityEventRouterViews* GetInstance();

  // Handle an accessibility event generated by a view.
  void HandleAccessibilityEvent(
      views::View* view, ui::AXEvent event_type);

  // Handle a menu item being focused (separate because a menu item is
  // not necessarily its own view).
  void HandleMenuItemFocused(const base::string16& menu_name,
                             const base::string16& menu_item_name,
                             int item_index,
                             int item_count,
                             bool has_submenu);

  // NotificationObserver implementation.
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

 private:
  friend struct DefaultSingletonTraits<AccessibilityEventRouterViews>;

  FRIEND_TEST_ALL_PREFIXES(AccessibilityEventRouterViewsTest,
                           TestFocusNotification);
  FRIEND_TEST_ALL_PREFIXES(AccessibilityEventRouterViewsTest,
                           MenuIndexAndCountForInvisibleMenu);

  AccessibilityEventRouterViews();
  virtual ~AccessibilityEventRouterViews();

  // Call DispatchAccessibilityEvent using a view storage id.
  static void DispatchEventOnViewStorageId(
      int view_storage_id,
      ui::AXEvent event);

  // Checks the type of the view and calls one of the more specific
  // Send*Notification methods, below.
  void DispatchAccessibilityEvent(
      views::View* view,
      ui::AXEvent event);

  // Each of these methods constructs an AccessibilityControlInfo object
  // and sends a notification of a specific accessibility event.
  static void SendButtonNotification(
      views::View* view,
      ui::AXEvent event,
      Profile* profile);
  static void SendStaticTextNotification(
      views::View* view,
      ui::AXEvent event,
      Profile* profile);
  static void SendLinkNotification(
      views::View* view,
      ui::AXEvent event,
      Profile* profile);
  static void SendMenuNotification(
      views::View* view,
      ui::AXEvent event,
      Profile* profile);
  static void SendTabNotification(
      views::View* view,
      ui::AXEvent event,
      Profile* profile);
  static void SendMenuItemNotification(
      views::View* view,
      ui::AXEvent event,
      Profile* profile);
  static void SendTreeNotification(
      views::View* view,
      ui::AXEvent event,
      Profile* profile);
  static void SendTreeItemNotification(
      views::View* view,
      ui::AXEvent event,
      Profile* profile);
  static void SendTextfieldNotification(
      views::View* view,
      ui::AXEvent event,
      Profile* profile);
  static void SendComboboxNotification(
      views::View* view,
      ui::AXEvent event,
      Profile* profile);
  static void SendCheckboxNotification(
      views::View* view,
      ui::AXEvent event,
      Profile* profile);
  static void SendWindowNotification(
      views::View* view,
      ui::AXEvent event,
      Profile* profile);
  static void SendSliderNotification(
      views::View* view,
      ui::AXEvent event,
      Profile* profile);
  static void SendAlertControlNotification(
      views::View* view,
      ui::AXEvent event,
      Profile* profile);

  // Return the name of a view.
  static std::string GetViewName(views::View* view);

  // Get the context of a view - the name of the enclosing group, toolbar, etc.
  static std::string GetViewContext(views::View* view);

  // Return a descendant of this view with a given accessible role, if found.
  static views::View* FindDescendantWithAccessibleRole(
      views::View* view,
      ui::AXRole role);

  // Recursively explore all menu items of |menu| and return in |count|
  // the total number of items, and in |index| the 0-based index of
  // |item|, if found. Initialize |count| to zero before calling this
  // method. |index| will be unchanged if the item is not found, so
  // initialize it to -1 to detect this case.
  static void RecursiveGetMenuItemIndexAndCount(views::View* menu,
                                                views::View* item,
                                                int* index,
                                                int* count);

  // Recursively explore the subviews and return the text from the first
  // subview with a role of STATIC_TEXT.
  static std::string RecursiveGetStaticText(views::View* view);

  // Returns the first ancestor of |view| (including |view|) that is
  // accessible.
  static views::View* FindFirstAccessibleAncestor(views::View* view);

  // The profile associated with the most recent window event  - used to
  // figure out where to route a few events that can't be directly traced
  // to a window with a profile (like menu events).
  Profile* most_recent_profile_;

  // The most recent accessibility focusable view is stored in view storage
  // and is used to prevent multiple events from being dispatched on a
  // hoverable view from its multiple children. This is the id for the most
  // recent view we put in view storage.
  const int most_recent_view_id_;

  // Notification registrar so we can clear most_recent_profile_ when a
  // profile is destroyed.
  content::NotificationRegistrar registrar_;

  DISALLOW_COPY_AND_ASSIGN(AccessibilityEventRouterViews);
};

#endif  // CHROME_BROWSER_UI_VIEWS_ACCESSIBILITY_ACCESSIBILITY_EVENT_ROUTER_VIEWS_H_
