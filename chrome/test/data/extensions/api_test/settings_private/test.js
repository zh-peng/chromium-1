// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This just tests the interface. It does not test for specific results, only
// that callbacks are correctly invoked, expected parameters are correct,
// and failures are detected.

var kTestPrefName = 'download.default_directory';
var kTestPrefValue = '/Downloads';
var kTestPageId = 'pageId';

function callbackResult(result) {
  if (chrome.runtime.lastError)
    chrome.test.fail(chrome.runtime.lastError.message);
  else if (result == false)
    chrome.test.fail('Failed: ' + result);
}

var availableTests = [
  function setPref() {
    chrome.settingsPrivate.setPref(
        kTestPrefName,
        kTestPrefValue,
        kTestPageId,
        function(success) {
          callbackResult(success);
          chrome.test.succeed();
        });
  },
  function getPref() {
    chrome.settingsPrivate.getPref(
        kTestPrefName,
        function(value) {
          chrome.test.assertTrue(value !== null);
          callbackResult(true);
          chrome.test.succeed();
        });
  },
  function getAllPrefs() {
    chrome.settingsPrivate.getAllPrefs(
        function(prefs) {
          chrome.test.assertTrue(prefs.length > 0);
          callbackResult(true);
          chrome.test.succeed();
        });
  },
  function onPrefsChanged() {
    chrome.settingsPrivate.onPrefsChanged.addListener(function(prefs) {
      chrome.test.assertTrue(prefs.length > 0);
      chrome.test.assertEq(kTestPrefName, prefs[0].key);
      chrome.test.assertEq(kTestPrefValue, prefs[0].value);
      callbackResult(true);
      chrome.test.succeed();
    });

    chrome.settingsPrivate.setPref(
        kTestPrefName,
        kTestPrefValue,
        kTestPageId,
        function() {});
  },
];

var testToRun = window.location.search.substring(1);
chrome.test.runTests(availableTests.filter(function(op) {
  return op.name == testToRun;
}));

