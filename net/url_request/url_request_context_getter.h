// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_URL_REQUEST_URL_REQUEST_CONTEXT_GETTER_H_
#define NET_URL_REQUEST_URL_REQUEST_CONTEXT_GETTER_H_

#include "base/memory/ref_counted.h"
#include "base/sequenced_task_runner_helpers.h"
#include "net/base/net_export.h"

namespace base {
class SingleThreadTaskRunner;
}  // namespace base

namespace net {
class CookieStore;
class URLRequestContext;

struct URLRequestContextGetterTraits;

// Interface for retrieving an URLRequestContext.
class NET_EXPORT URLRequestContextGetter
    : public base::RefCountedThreadSafe<URLRequestContextGetter,
                                        URLRequestContextGetterTraits> {
 public:
  virtual URLRequestContext* GetURLRequestContext() = 0;

  // Returns a SingleThreadTaskRunner corresponding to the thread on
  // which the network IO happens (the thread on which the returned
  // URLRequestContext may be used).
  virtual scoped_refptr<base::SingleThreadTaskRunner>
      GetNetworkTaskRunner() const = 0;

 protected:
  friend class base::RefCountedThreadSafe<URLRequestContextGetter,
                                          URLRequestContextGetterTraits>;
  friend class base::DeleteHelper<URLRequestContextGetter>;
  friend struct URLRequestContextGetterTraits;

  URLRequestContextGetter();
  virtual ~URLRequestContextGetter();

 private:
  // OnDestruct is meant to ensure deletion on the thread on which the request
  // IO happens.
  void OnDestruct() const;
};

struct URLRequestContextGetterTraits {
  static void Destruct(const URLRequestContextGetter* context_getter) {
    context_getter->OnDestruct();
  }
};

// For use in shimming a URLRequestContext into a URLRequestContextGetter.
class NET_EXPORT TrivialURLRequestContextGetter
    : public URLRequestContextGetter {
public:
 TrivialURLRequestContextGetter(
     URLRequestContext* context,
     const scoped_refptr<base::SingleThreadTaskRunner>& main_task_runner);

 // URLRequestContextGetter implementation:
 URLRequestContext* GetURLRequestContext() override;

  scoped_refptr<base::SingleThreadTaskRunner> GetNetworkTaskRunner()
      const override;

private:
 ~TrivialURLRequestContextGetter() override;

 URLRequestContext* context_;
  const scoped_refptr<base::SingleThreadTaskRunner> main_task_runner_;

  DISALLOW_COPY_AND_ASSIGN(TrivialURLRequestContextGetter);
};

}  // namespace net

#endif  // NET_URL_REQUEST_URL_REQUEST_CONTEXT_GETTER_H_
