// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_GCM_DRIVER_GCM_DRIVER_ANDROID_H
#define COMPONENTS_GCM_DRIVER_GCM_DRIVER_ANDROID_H

#include <jni.h>

#include "base/android/scoped_java_ref.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "components/gcm_driver/gcm_driver.h"

namespace gcm {

// GCMDriver implementation for Android, using Android GCM APIs.
class GCMDriverAndroid : public GCMDriver {
 public:
  GCMDriverAndroid();
  ~GCMDriverAndroid() override;

  // Methods called from Java via JNI:
  void OnRegisterFinished(JNIEnv* env,
                          jobject obj,
                          jstring app_id,
                          jstring registration_id,
                          jboolean success);
  void OnUnregisterFinished(JNIEnv* env,
                           jobject obj,
                           jstring app_id,
                           jboolean success);
  void OnMessageReceived(JNIEnv* env,
                         jobject obj,
                         jstring app_id,
                         jstring sender_id,
                         jstring collapse_key,
                         jobjectArray data_keys_and_values);
  void OnMessagesDeleted(JNIEnv* env,
                         jobject obj,
                         jstring app_id);

  // Register JNI methods.
  static bool RegisterBindings(JNIEnv* env);

  // GCMDriver implementation:
  void OnSignedIn() override;
  void OnSignedOut() override;
  void Enable() override;
  void AddConnectionObserver(GCMConnectionObserver* observer) override;
  void RemoveConnectionObserver(GCMConnectionObserver* observer) override;
  void Disable() override;
  GCMClient* GetGCMClientForTesting() const override;
  bool IsStarted() const override;
  bool IsConnected() const override;
  void GetGCMStatistics(const GetGCMStatisticsCallback& callback,
                        bool clear_logs) override;
  void SetGCMRecording(const GetGCMStatisticsCallback& callback,
                       bool recording) override;
  void SetAccountTokens(
      const std::vector<GCMClient::AccountTokenInfo>& account_tokens) override;
  void UpdateAccountMapping(const AccountMapping& account_mapping) override;
  void RemoveAccountMapping(const std::string& account_id) override;
  base::Time GetLastTokenFetchTime() override;
  void SetLastTokenFetchTime(const base::Time& time) override;
  void WakeFromSuspendForHeartbeat(bool wake) override;

 protected:
  // GCMDriver implementation:
  GCMClient::Result EnsureStarted(GCMClient::StartMode start_mode) override;
  void RegisterImpl(const std::string& app_id,
                    const std::vector<std::string>& sender_ids) override;
  void UnregisterImpl(const std::string& app_id) override;
  void UnregisterWithSenderIdImpl(const std::string& app_id,
                                  const std::string& sender_id) override;
  void SendImpl(const std::string& app_id,
                const std::string& receiver_id,
                const GCMClient::OutgoingMessage& message) override;

 private:
  base::android::ScopedJavaGlobalRef<jobject> java_ref_;

  DISALLOW_COPY_AND_ASSIGN(GCMDriverAndroid);
};

}  // namespace gcm

#endif  // COMPONENTS_GCM_DRIVER_GCM_DRIVER_ANDROID_H
