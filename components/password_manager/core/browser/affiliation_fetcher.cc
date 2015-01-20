// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/password_manager/core/browser/affiliation_fetcher.h"

#include "components/password_manager/core/browser/affiliation_api.pb.h"
#include "components/password_manager/core/browser/affiliation_utils.h"
#include "google_apis/google_api_keys.h"
#include "net/base/load_flags.h"
#include "net/base/url_util.h"
#include "net/http/http_status_code.h"
#include "net/url_request/url_fetcher.h"
#include "net/url_request/url_request_context_getter.h"
#include "url/gurl.h"

namespace password_manager {

AffiliationFetcher::AffiliationFetcher(
    net::URLRequestContextGetter* request_context_getter,
    const std::vector<FacetURI>& facet_uris,
    AffiliationFetcherDelegate* delegate)
    : request_context_getter_(request_context_getter),
      requested_facet_uris_(facet_uris),
      delegate_(delegate) {
  for (const FacetURI& uri : requested_facet_uris_) {
    DCHECK(uri.is_valid());
  }
}

AffiliationFetcher::~AffiliationFetcher() {
}

// static
AffiliationFetcher* AffiliationFetcher::Create(
    net::URLRequestContextGetter* context_getter,
    const std::vector<FacetURI>& facet_uris,
    AffiliationFetcherDelegate* delegate) {
  return new AffiliationFetcher(context_getter, facet_uris, delegate);
}

void AffiliationFetcher::StartRequest() {
  DCHECK(!fetcher_);

  fetcher_.reset(
      net::URLFetcher::Create(BuildQueryURL(), net::URLFetcher::POST, this));
  fetcher_->SetRequestContext(request_context_getter_.get());
  fetcher_->SetUploadData("application/x-protobuf", PreparePayload());
  fetcher_->SetLoadFlags(net::LOAD_DO_NOT_SAVE_COOKIES |
                         net::LOAD_DO_NOT_SEND_COOKIES |
                         net::LOAD_DO_NOT_SEND_AUTH_DATA |
                         net::LOAD_BYPASS_CACHE | net::LOAD_DISABLE_CACHE);
  fetcher_->SetAutomaticallyRetryOn5xx(false);
  fetcher_->SetAutomaticallyRetryOnNetworkChanges(0);
  fetcher_->Start();
}

GURL AffiliationFetcher::BuildQueryURL() const {
  return net::AppendQueryParameter(
      GURL("https://www.googleapis.com/affiliation/v1/affiliation:lookup"),
      "key", google_apis::GetAPIKey());
}

std::string AffiliationFetcher::PreparePayload() const {
  affiliation_pb::LookupAffiliationRequest lookup_request;
  for (const FacetURI& uri : requested_facet_uris_)
    lookup_request.add_facet(uri.canonical_spec());

  std::string serialized_request;
  bool success = lookup_request.SerializeToString(&serialized_request);
  DCHECK(success);
  return serialized_request;
}

bool AffiliationFetcher::ParseResponse(
    AffiliationFetcherDelegate::Result* result) const {
  // This function parses the response protocol buffer message for a list of
  // equivalence classes, and stores them into |results| after performing some
  // validation and sanitization steps to make sure that the contract of
  // AffiliationFetcherDelegate is fulfilled. Possible discrepancies are:
  //   * The server response will not have anything for facets that are not
  //     affiliated with any other facet, while |result| must have them.
  //   * The server response might contain future, unknown kinds of facet URIs,
  //     while |result| must contain only those that are FacetURI::is_valid().
  //   * The server response being ill-formed or self-inconsistent (in the sense
  //     that there are overlapping equivalence classes) is indicative of server
  //     side issues likely not remedied by re-fetching. Report failure in this
  //     case so the caller can be notified and it can act accordingly.
  //   * The |result| will be free of duplicate or empty equivalence classes.

  std::string serialized_response;
  if (!fetcher_->GetResponseAsString(&serialized_response)) {
    NOTREACHED();
  }

  affiliation_pb::LookupAffiliationResponse response;
  if (!response.ParseFromString(serialized_response))
    return false;

  result->reserve(requested_facet_uris_.size());

  std::map<FacetURI, size_t> facet_uri_to_class_index;
  for (int i = 0; i < response.affiliation_size(); ++i) {
    const affiliation_pb::Affiliation& equivalence_class(
        response.affiliation(i));

    AffiliatedFacets affiliated_uris;
    for (int j = 0; j < equivalence_class.facet_size(); ++j) {
      const std::string& uri_spec(equivalence_class.facet(j));
      FacetURI uri = FacetURI::FromPotentiallyInvalidSpec(uri_spec);
      // Ignore potential future kinds of facet URIs (e.g. for new platforms).
      if (!uri.is_valid())
        continue;
      affiliated_uris.push_back(uri);
    }

    // Be lenient and ignore empty (after filtering) equivalence classes.
    if (affiliated_uris.empty())
      continue;

    // Ignore equivalence classes that are duplicates of earlier ones. However,
    // fail in the case of a partial overlap, which violates the invariant that
    // affiliations must form an equivalence relation.
    for (const FacetURI& uri : affiliated_uris) {
      if (!facet_uri_to_class_index.count(uri))
        facet_uri_to_class_index[uri] = result->size();
      if (facet_uri_to_class_index[uri] !=
          facet_uri_to_class_index[affiliated_uris[0]]) {
        return false;
      }
    }

    // Filter out duplicate equivalence classes in the response.
    if (facet_uri_to_class_index[affiliated_uris[0]] == result->size())
      result->push_back(affiliated_uris);
  }

  // Synthesize an equivalence class (of size one) for each facet that did not
  // appear in the server response due to not being affiliated with any others.
  for (const FacetURI& uri : requested_facet_uris_) {
    if (!facet_uri_to_class_index.count(uri))
      result->push_back(AffiliatedFacets(1, uri));
  }

  return true;
}

void AffiliationFetcher::OnURLFetchComplete(const net::URLFetcher* source) {
  DCHECK_EQ(source, fetcher_.get());

  scoped_ptr<AffiliationFetcherDelegate::Result> result(
      new AffiliationFetcherDelegate::Result);
  if (fetcher_->GetStatus().status() == net::URLRequestStatus::SUCCESS &&
      fetcher_->GetResponseCode() == net::HTTP_OK) {
    if (ParseResponse(result.get()))
      delegate_->OnFetchSucceeded(result.Pass());
    else
      delegate_->OnMalformedResponse();
  } else {
    delegate_->OnFetchFailed();
  }
}

}  // namespace password_manager
