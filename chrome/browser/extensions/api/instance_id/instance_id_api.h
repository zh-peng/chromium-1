// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_API_INSTANCE_ID_INSTANCE_ID_API_H_
#define CHROME_BROWSER_EXTENSIONS_API_INSTANCE_ID_INSTANCE_ID_API_H_

#include "extensions/browser/extension_function.h"

class Profile;

namespace extensions {

class InstanceIDGetIDFunction : public UIThreadExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("instanceID.getID", INSTANCEID_GETID);

  InstanceIDGetIDFunction();

 protected:
  ~InstanceIDGetIDFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;
};

class InstanceIDGetCreationTimeFunction : public UIThreadExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("instanceID.getCreationTime",
                             INSTANCEID_GETCREATIONTIME);

  InstanceIDGetCreationTimeFunction();

 protected:
  ~InstanceIDGetCreationTimeFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;
};

class InstanceIDGetTokenFunction : public UIThreadExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("instanceID.getToken", INSTANCEID_GETTOKEN);

  InstanceIDGetTokenFunction();

 protected:
  ~InstanceIDGetTokenFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;
};

class InstanceIDDeleteTokenFunction : public UIThreadExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("instanceID.DeleteToken", INSTANCEID_DELETETOKEN);

  InstanceIDDeleteTokenFunction();

 protected:
  ~InstanceIDDeleteTokenFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;
};

class InstanceIDDeleteIDFunction : public UIThreadExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("instanceID.deleteID",
                             INSTANCEID_DELETEID);

  InstanceIDDeleteIDFunction();

 protected:
  ~InstanceIDDeleteIDFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_API_INSTANCE_ID_INSTANCE_ID_API_H_
