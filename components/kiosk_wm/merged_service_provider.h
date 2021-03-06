// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_KIOSK_WM_MERGED_SERVICE_PROVIDER_H_
#define COMPONENTS_KIOSK_WM_MERGED_SERVICE_PROVIDER_H_

#include "base/memory/scoped_ptr.h"
#include "mandoline/services/navigation/public/interfaces/navigation.mojom.h"
#include "third_party/mojo/src/mojo/public/cpp/application/interface_factory.h"
#include "third_party/mojo/src/mojo/public/cpp/bindings/binding.h"
#include "third_party/mojo/src/mojo/public/interfaces/application/service_provider.mojom.h"

namespace kiosk_wm {

// Used to wrap the second incoming WindowManager Embed() "exposed_services"
// parameter with a new ServiceProvider that adds the KioskWM's
// NavigatorHost service.
class MergedServiceProvider : public mojo::ServiceProvider {
 public:
  MergedServiceProvider(mojo::ServiceProviderPtr exposed_services,
                        mojo::InterfaceFactory<mojo::NavigatorHost>* factory);
  ~MergedServiceProvider() override;

  mojo::ServiceProviderPtr GetServiceProviderPtr();

 private:
  // mojo::ServiceProvider:
  void ConnectToService(const mojo::String& interface_name,
                        mojo::ScopedMessagePipeHandle pipe) override;

  mojo::ServiceProviderPtr exposed_services_;
  mojo::InterfaceFactory<mojo::NavigatorHost>* factory_;
  scoped_ptr<mojo::Binding<ServiceProvider>> binding_;

  DISALLOW_COPY_AND_ASSIGN(MergedServiceProvider);
};

}  // namespace kiosk_wm

#endif  // COMPONENTS_KIOSK_WM_MERGED_SERVICE_PROVIDER_H_
