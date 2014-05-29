// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_METRICS_PERF_PROVIDER_CHROMEOS_H_
#define CHROME_BROWSER_METRICS_PERF_PROVIDER_CHROMEOS_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/threading/non_thread_safe.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "chromeos/login/login_state.h"
#include "components/metrics/proto/perf_data.pb.h"

namespace metrics {

class WindowedIncognitoObserver;

// Provides access to ChromeOS perf data. perf aka "perf events" is a
// performance profiling infrastructure built into the linux kernel. For more
// information, see: https://perf.wiki.kernel.org/index.php/Main_Page.
class PerfProvider : public base::NonThreadSafe {
 public:
  PerfProvider();
  ~PerfProvider();

  // Writes collected perf data protobufs to |perf_data|. Clears all the stored
  // perf data. Returns true if it wrote to |perf_data|.
  bool GetPerfData(std::vector<PerfDataProto>* perf_data);

 private:
  // Class that listens for changes to the login state. When a normal user logs
  // in, it updates PerfProvider to start collecting data.
  class LoginObserver : public chromeos::LoginState::Observer {
   public:
    explicit LoginObserver(PerfProvider* perf_provider);

    // Called when either the login state or the logged in user type changes.
    // Activates |perf_provider_| to start collecting.
    virtual void LoggedInStateChanged() OVERRIDE;

   private:
    // Points to a PerfProvider instance that can be turned on or off based on
    // the login state.
    PerfProvider* perf_provider_;
  };

  // Turns on perf collection. Resets the timer that's used to schedule
  // collections.
  void Activate();

  // Turns off perf collection. Does not delete any data that was already
  // collected and stored in |cached_perf_data_|.
  void Deactivate();

  // Starts an internal timer to start collecting perf data. The timer is set to
  // trigger |interval| after this function call.
  void ScheduleCollection(const base::TimeDelta& interval);

  // Collects perf data if it has not been consumed by calling GetPerfData()
  // (see above).
  void CollectIfNecessary();

  // Collects perf data by calling CollectIfNecessary() and reschedules it to be
  // collected again.
  void CollectIfNecessaryAndReschedule();

  // Parses a protobuf from the |data| passed in only if the
  // |incognito_observer| indicates that no incognito window had been opened
  // during the profile collection period.
  void ParseProtoIfValid(
      scoped_ptr<WindowedIncognitoObserver> incognito_observer,
      const std::vector<uint8>& data);

  // Vector of perf data protobufs.
  std::vector<PerfDataProto> cached_perf_data_;

  // For scheduling collection of perf data.
  base::OneShotTimer<PerfProvider> timer_;

  // For detecting when changes to the login state.
  LoginObserver login_observer_;

  // To pass around the "this" pointer across threads safely.
  base::WeakPtrFactory<PerfProvider> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(PerfProvider);
};

}  // namespace metrics

#endif  // CHROME_BROWSER_METRICS_PERF_PROVIDER_CHROMEOS_H_
