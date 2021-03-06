// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/web/web_state/js/credential_util.h"

#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "ios/web/public/web_state/credential.h"
#include "url/gurl.h"

namespace {

// "type" value for a DictionaryValue representation of LocalCredential.
const char* kLocalCredentialType = "LocalCredential";

// "type" value for a DictionaryValue representation of FederatedCredential.
const char* kFederatedCredentialType = "FederatedCredential";

}  // namespace

namespace web {

bool DictionaryValueToCredential(const base::DictionaryValue& value,
                                 Credential* credential) {
  DCHECK(credential);

  base::string16 type;
  if (!value.GetString("type", &type))
    return false;
  CredentialType credential_type;
  if (type == base::ASCIIToUTF16(kLocalCredentialType))
    credential_type = CredentialType::CREDENTIAL_TYPE_LOCAL;
  else if (type == base::ASCIIToUTF16(kFederatedCredentialType))
    credential_type = CredentialType::CREDENTIAL_TYPE_FEDERATED;
  else
    return false;

  base::string16 id;
  if (!value.GetString("id", &id))
    return false;

  base::string16 name;
  value.GetString("name", &name);

  base::string16 avatar;
  GURL avatar_url;
  if (value.GetString("avatarURL", &avatar)) {
    avatar_url = GURL(avatar);
    if (!avatar_url.is_valid())
      return false;
  }

  base::string16 password;
  if (credential_type == CredentialType::CREDENTIAL_TYPE_LOCAL &&
      !value.GetString("password", &password)) {
    return false;
  }

  base::string16 federation;
  GURL federation_url;
  if (credential_type == CredentialType::CREDENTIAL_TYPE_FEDERATED) {
    if (!value.GetString("federation", &federation))
      return false;
    federation_url = GURL(federation);
    if (!federation_url.is_valid())
      return false;
  }

  credential->type = credential_type;
  credential->id = id;
  credential->name = name;
  credential->avatar_url = avatar_url;
  credential->password = password;
  credential->federation_url = federation_url;
  return true;
}

void CredentialToDictionaryValue(const Credential& credential,
                                 base::DictionaryValue* value) {
  switch (credential.type) {
    case CredentialType::CREDENTIAL_TYPE_LOCAL:
      value->SetString("type", kLocalCredentialType);
      value->SetString("password", credential.password);
      break;
    case CredentialType::CREDENTIAL_TYPE_FEDERATED:
      value->SetString("type", kFederatedCredentialType);
      value->SetString("federation", credential.federation_url.spec());
      break;
    default:
      NOTREACHED();
  }
  value->SetString("id", credential.id);
  value->SetString("name", credential.name);
  value->SetString("avatarURL", credential.avatar_url.spec());
}

}  // web
