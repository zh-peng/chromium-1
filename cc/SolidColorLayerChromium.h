// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef SolidColorLayerChromium_h
#define SolidColorLayerChromium_h

#if USE(ACCELERATED_COMPOSITING)

#include "LayerChromium.h"

namespace cc {

// A Layer that renders a solid color. The color is specified by using
// setBackgroundColor() on the base class.
class SolidColorLayerChromium : public LayerChromium {
public:
    static scoped_refptr<SolidColorLayerChromium> create();

    virtual PassOwnPtr<CCLayerImpl> createCCLayerImpl() OVERRIDE;

protected:
    SolidColorLayerChromium();

private:
    virtual ~SolidColorLayerChromium();
};

}
#endif // USE(ACCELERATED_COMPOSITING)

#endif
