// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/cocoa/floating_bar_backing_view.h"

#import "chrome/browser/cocoa/browser_frame_view.h"

@implementation FloatingBarBackingView

- (void)drawRect:(NSRect)rect {
  NSWindow* window = [self window];
  BOOL isMainWindow = [window isMainWindow];

  if (isMainWindow)
    [[NSColor windowFrameColor] set];
  else
    [[NSColor windowBackgroundColor] set];
  NSRectFill(rect);

  [BrowserFrameView drawWindowThemeInDirtyRect:rect
                                       forView:self
                                        bounds:[self bounds]];
}

// Eat all mouse events (and do *not* pass them on to the next responder!).
- (void)mouseDown:(NSEvent*)event {}
- (void)rightMouseDown:(NSEvent*)event {}
- (void)otherMouseDown:(NSEvent*)event {}
- (void)rightMouseUp:(NSEvent*)event {}
- (void)otherMouseUp:(NSEvent*)event {}
- (void)mouseMoved:(NSEvent*)event {}
- (void)mouseDragged:(NSEvent*)event {}
- (void)rightMouseDragged:(NSEvent*)event {}
- (void)otherMouseDragged:(NSEvent*)event {}

// Eat this too, except that ...
- (void)mouseUp:(NSEvent*)event {
  // a double-click in the blank area should minimize.
  if ([event clickCount] == 2)
    [[self window] performMiniaturize:self];
}

@end  // @implementation FloatingBarBackingView
