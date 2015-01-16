// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/file_manager/app_installer.h"

#include "chrome/common/extensions/webstore_install_result.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"

namespace file_manager {

namespace {
const char kWebContentsDestroyedError[] = "WebContents is destroyed.";
}  // namespace

class AppInstaller::WebContentsObserver : public content::WebContentsObserver {
 public:
  WebContentsObserver(content::WebContents* web_contents, AppInstaller* parent)
      : content::WebContentsObserver(web_contents),
        parent_(parent) {
  }

 protected:
  // content::WebContentsObserver implementation.
  void WebContentsDestroyed() override {
    parent_->OnWebContentsDestroyed(web_contents());
  }

 private:
  AppInstaller* parent_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebContentsObserver);
};

AppInstaller::AppInstaller(content::WebContents* web_contents,
                           const std::string& item_id,
                           Profile* profile,
                           bool silent_installation,
                           const Callback& callback)
    : extensions::WebstoreStandaloneInstaller(item_id, profile, callback),
      silent_installation_(silent_installation),
      callback_(callback),
      web_contents_(web_contents),
      web_contents_observer_(new WebContentsObserver(web_contents, this)) {
}

AppInstaller::~AppInstaller() {}

bool AppInstaller::CheckRequestorAlive() const {
  // The tab may have gone away - cancel installation in that case.
  return web_contents_ != NULL;
}

const GURL& AppInstaller::GetRequestorURL() const {
  return GURL::EmptyGURL();
}

scoped_refptr<ExtensionInstallPrompt::Prompt>
AppInstaller::CreateInstallPrompt() const {
  if (silent_installation_)
    return NULL;

  scoped_refptr<ExtensionInstallPrompt::Prompt> prompt(
      new ExtensionInstallPrompt::Prompt(
          ExtensionInstallPrompt::INLINE_INSTALL_PROMPT));

  prompt->SetWebstoreData(localized_user_count(),
                          show_user_count(),
                          average_rating(),
                          rating_count());
  return prompt;
}

bool AppInstaller::ShouldShowPostInstallUI() const {
  return false;
}

bool AppInstaller::ShouldShowAppInstalledBubble() const {
  return false;
}

content::WebContents* AppInstaller::GetWebContents() const {
  return web_contents_;
}

bool AppInstaller::CheckInlineInstallPermitted(
    const base::DictionaryValue& webstore_data,
    std::string* error) const {
  DCHECK(error != NULL);
  DCHECK(error->empty());
  return true;
}

bool AppInstaller::CheckRequestorPermitted(
    const base::DictionaryValue& webstore_data,
    std::string* error) const {
  DCHECK(error != NULL);
  DCHECK(error->empty());
  return true;
}

void AppInstaller::OnWebContentsDestroyed(
    content::WebContents* web_contents) {
  callback_.Run(false,
                kWebContentsDestroyedError,
                extensions::webstore_install::OTHER_ERROR);
  AbortInstall();
}

}  // namespace file_manager
