// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SESSIONS_TAB_LOADER_H_
#define CHROME_BROWSER_SESSIONS_TAB_LOADER_H_

#include <list>
#include <set>

#include "base/memory/memory_pressure_listener.h"
#include "base/memory/scoped_ptr.h"
#include "base/timer/timer.h"
#include "chrome/browser/sessions/tab_loader_delegate.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"

namespace content {
class NavigationController;
class RenderWidgetHost;
}

// TabLoader is responsible for loading tabs after session restore has finished
// creating all the tabs. Tabs are loaded after a previously tab finishes
// loading or a timeout is reached. If the timeout is reached before a tab
// finishes loading the timeout delay is doubled.
//
// TabLoader keeps a reference to itself when it's loading. When it has finished
// loading, it drops the reference. If another profile is restored while the
// TabLoader is loading, it will schedule its tabs to get loaded by the same
// TabLoader. When doing the scheduling, it holds a reference to the TabLoader.
//
// This is not part of SessionRestoreImpl so that synchronous destruction
// of SessionRestoreImpl doesn't have timing problems.
class TabLoader : public content::NotificationObserver,
                  public base::RefCounted<TabLoader>,
                  public TabLoaderCallback {
 public:
  // Retrieves a pointer to the TabLoader instance shared between profiles, or
  // creates a new TabLoader if it doesn't exist. If a TabLoader is created, its
  // starting timestamp is set to |restore_started|.
  static TabLoader* GetTabLoader(base::TimeTicks restore_started);

  // Schedules a tab for loading.
  void ScheduleLoad(content::NavigationController* controller);

  // Notifies the loader that a tab has been scheduled for loading through
  // some other mechanism.
  void TabIsLoading(content::NavigationController* controller);

  // Invokes |LoadNextTab| to load a tab.
  //
  // This must be invoked once to start loading.
  void StartLoading();

  // TabLoaderCallback:
  void SetTabLoadingEnabled(bool enable_tab_loading) override;

 private:
  friend class base::RefCounted<TabLoader>;

  typedef std::set<content::NavigationController*> TabsLoading;
  typedef std::list<content::NavigationController*> TabsToLoad;
  typedef std::set<content::RenderWidgetHost*> RenderWidgetHostSet;

  explicit TabLoader(base::TimeTicks restore_started);
  ~TabLoader() override;

  // Loads the next tab. If there are no more tabs to load this deletes itself,
  // otherwise |force_load_timer_| is restarted.
  void LoadNextTab();

  // Starts a timer to load load the next tab once expired before the current
  // tab loading is finished.
  void StartTimer();

  // NotificationObserver method. Removes the specified tab and loads the next
  // tab.
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) override;

  // Removes the listeners from the specified tab and removes the tab from
  // the set of tabs to load and list of tabs we're waiting to get a load
  // from.
  void RemoveTab(content::NavigationController* tab);

  // Invoked from |force_load_timer_|. Doubles |force_load_delay_multiplier_|
  // and invokes |LoadNextTab| to load the next tab
  void ForceLoadTimerFired();

  // Returns the RenderWidgetHost associated with a tab if there is one,
  // NULL otherwise.
  static content::RenderWidgetHost* GetRenderWidgetHost(
      content::NavigationController* tab);

  // Register for necessary notifications on a tab navigation controller.
  void RegisterForNotifications(content::NavigationController* controller);

  // Called when a tab goes away or a load completes.
  void HandleTabClosedOrLoaded(content::NavigationController* controller);

  // TODO(sky): remove. For debugging 368236.
  void CheckNotObserving(content::NavigationController* controller);

  // React to memory pressure by stopping to load any more tabs.
  void OnMemoryPressure(
      base::MemoryPressureListener::MemoryPressureLevel memory_pressure_level);

  scoped_ptr<TabLoaderDelegate> delegate_;

  // Listens for system under memory pressure notifications and stops loading
  // of tabs when we start running out of memory.
  base::MemoryPressureListener memory_pressure_listener_;

  content::NotificationRegistrar registrar_;

  // The delay timer multiplier. See class description for details.
  size_t force_load_delay_multiplier_;

  // True if the tab loading is enabled.
  bool loading_enabled_;

  // Have we recorded the times for a foreground tab load?
  bool got_first_foreground_load_;

  // Have we recorded the times for a foreground tab paint?
  bool got_first_paint_;

  // The set of tabs we've initiated loading on. This does NOT include the
  // selected tabs.
  TabsLoading tabs_loading_;

  // The tabs we need to load.
  TabsToLoad tabs_to_load_;

  // The renderers we have started loading into.
  RenderWidgetHostSet render_widget_hosts_loading_;

  // The renderers we have loaded and are waiting on to paint.
  RenderWidgetHostSet render_widget_hosts_to_paint_;

  // The number of tabs that have been restored.
  int tab_count_;

  base::OneShotTimer<TabLoader> force_load_timer_;

  // The time the restore process started.
  base::TimeTicks restore_started_;

  // Max number of tabs that were loaded in parallel (for metrics).
  size_t max_parallel_tab_loads_;

  // For keeping TabLoader alive while it's loading even if no
  // SessionRestoreImpls reference it.
  scoped_refptr<TabLoader> this_retainer_;

  static TabLoader* shared_tab_loader_;

  DISALLOW_COPY_AND_ASSIGN(TabLoader);
};

#endif  // CHROME_BROWSER_SESSIONS_TAB_LOADER_H_
