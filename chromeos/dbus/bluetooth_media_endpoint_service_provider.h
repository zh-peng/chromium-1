// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_DBUS_BLUETOOTH_MEDIA_ENDPOINT_SERVICE_PROVIDER_H_
#define CHROMEOS_DBUS_BLUETOOTH_MEDIA_ENDPOINT_SERVICE_PROVIDER_H_

#include <vector>

#include "base/callback.h"
#include "chromeos/chromeos_export.h"
#include "dbus/bus.h"
#include "dbus/message.h"
#include "dbus/object_path.h"

namespace chromeos {

// BluetoothMediaEndpointServiceProvider is used to provide a D-Bus object that
// the Bluetooth daemon can commuicate with to serve as a media source/sink.
//
// Instantiate with a chosen D-Bus object path and a delegate object, and pass
// the D-Bus object path as |endpoint_path| argument to the
// chromeos::BluetoothMediaClient::RegisterEndoint() method.
//
// After initiating a connection between an audio source and an audio sink, the
// Bluetooth daemon will make calls to this endpoint object and they will be
// passed to user's Delegate object for handling. For SelectConfiguration method
// the response is returned using the SelectConfiguration callback.
class CHROMEOS_EXPORT BluetoothMediaEndpointServiceProvider {
 public:
  // Delegate is the interface for reacting to endpoint requests. User
  // applications will implement this interface to handle either A2DP Sink or
  // Source.
  class Delegate {
   public:
    virtual ~Delegate() {}

    // SelectConfigurationCallback is used for the SelectConfiguration() method,
    // it should be called with two arguements, the |configuration| which is
    // agreed by the application and the |length| of |configuration|.
    typedef base::Callback<void(const std::vector<uint8_t>&)>
        SelectConfigurationCallback;

    // This method will be called after an Audio Source receives the agreed
    // capabilities from the Audio Sink to set the configuration for the
    // media transport object. |transport_path| is the path to the
    // MediaTransport object, and |properties| are the properties for that
    // MediaTransport object.
    virtual void SetConfiguration(const dbus::ObjectPath& transport_path,
                                  const dbus::MessageReader& properties) = 0;

    // This method will be called when an Audio Source connects to an Audio Sink
    // and asks it to decide the configuration to be used during the oncoming
    // streaming. Audio Sources provide |capabilities| as a reference, where
    // a user application can use these |capabilities| to figure out
    // a well-matched configuration and return it to the Audio Source via
    // |callback|.
    virtual void SelectConfiguration(
        const std::vector<uint8_t>& capabilities,
        const SelectConfigurationCallback& callback) = 0;

    // This method will be called when an Audio Source disconnects from an Audio
    // Sink. A user application is supposed to clear any of its resources which
    // it keeps for that particular connection. |transport_path| is the Media
    // Transport object which has been kept by an endpoint during the
    // connection.
    virtual void ClearConfiguration(const dbus::ObjectPath& transport_path) = 0;

    // This method will be called when the Bluetooth daemon unregisters the
    // Media Endpoint. Media Endpoint objects can use this method to clean up
    // tasks. There is no need to unregister the endpoint, since when this
    // method gets called, that endpoint has been unregistered.
    virtual void Release() = 0;
  };

  virtual ~BluetoothMediaEndpointServiceProvider();

  // Creates the instance where |bus| is the D-Bus bus connection to export the
  // object onto, |object_path| is the object path that it should have and
  // |delegate| is the object to which all method calls will be passed and
  // responses generated from.
  static BluetoothMediaEndpointServiceProvider* Create(
      dbus::Bus* bus,
      const dbus::ObjectPath& object_path,
      Delegate* delegate);

 protected:
  BluetoothMediaEndpointServiceProvider();

 private:
  DISALLOW_COPY_AND_ASSIGN(BluetoothMediaEndpointServiceProvider);
};

}  // namespace chromeos

#endif  // CHROMEOS_DBUS_BLUETOOTH_MEDIA_ENDPOINT_SERVICE_PROVIDER_H_
