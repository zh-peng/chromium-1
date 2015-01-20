# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

LOCAL_SDK_VERSION := 21

LOCAL_SRC_FILES := $(call all-java-files-under, java/src)

# contentview and its dependencies
LOCAL_AIDL_INCLUDES := \
    $(LOCAL_PATH)/../content/public/android/java/src \
    $(LOCAL_PATH)/../net/android/java/src \
    $(LOCAL_PATH)/../third_party/eyesfree/src/android/java/src

LOCAL_SRC_FILES += \
    $(call all-java-files-under, ../content/public/android/java/src) \
    ../content/public/android/java/src/org/chromium/content/common/IChildProcessCallback.aidl \
    ../content/public/android/java/src/org/chromium/content/common/IChildProcessService.aidl \
    ../net/android/java/src/org/chromium/net/IRemoteAndroidKeyStoreCallbacks.aidl \
    ../net/android/java/src/org/chromium/net/IRemoteAndroidKeyStore.aidl \
    $(call all-java-files-under, ../base/android/java/src) \
    $(call all-java-files-under, ../device/battery/android/java/src) \
    $(call all-java-files-under, ../media/base/android/java/src) \
    $(call all-java-files-under, ../mojo/android/system/src) \
    $(call all-java-files-under, ../third_party/mojo/src/mojo/public/java/bindings/src) \
    $(call all-java-files-under, ../third_party/mojo/src/mojo/public/java/system/src) \
    $(call all-java-files-under, ../net/android/java/src) \
    $(call all-java-files-under, ../ui/android/java/src) \
    $(call all-java-files-under, ../third_party/eyesfree/src/android/java/src) \
    $(call all-Iaidl-files-under, ../third_party/eyesfree/src/android/java/src)

# browser components
LOCAL_SRC_FILES += \
    $(call all-java-files-under, \
        ../components/web_contents_delegate_android/android/java/src) \
    $(call all-java-files-under, \
        ../components/navigation_interception/android/java/src) \

# This directory includes .java files that are generated by the normal gyp build, but are checked in
# for the Android build.
# TODO(torne, cjhopman): Consider removing this.
LOCAL_SRC_FILES += \
    $(call all-java-files-under, java/generated_src)

# Java files generated from .template rules. This list should match list of java dependencies in
# android_webview/libwebviewchromium.gyp
LOCAL_GENERATED_SOURCES := \
$(call intermediates-dir-for,GYP,shared)/enums/android_resource_type_java/org/chromium/ui/resources/AndroidResourceType.java \
$(call intermediates-dir-for,GYP,shared)/enums/bitmap_format_java/org/chromium/ui/gfx/BitmapFormat.java \
$(call intermediates-dir-for,GYP,shared)/enums/cert_verify_status_android_java/org/chromium/net/CertVerifyStatusAndroid.java \
$(call intermediates-dir-for,GYP,shared)/enums/certificate_mime_types_java/org/chromium/net/CertificateMimeType.java \
$(call intermediates-dir-for,GYP,shared)/enums/network_change_notifier_types_java/org/chromium/net/ConnectionSubtype.java \
$(call intermediates-dir-for,GYP,shared)/enums/network_change_notifier_types_java/org/chromium/net/ConnectionType.java \
$(call intermediates-dir-for,GYP,shared)/enums/content_gamepad_mapping/org/chromium/content/browser/input/CanonicalAxisIndex.java \
$(call intermediates-dir-for,GYP,shared)/enums/content_gamepad_mapping/org/chromium/content/browser/input/CanonicalButtonIndex.java \
$(call intermediates-dir-for,GYP,shared)/enums/gesture_event_type_java/org/chromium/content/browser/GestureEventType.java \
$(call intermediates-dir-for,GYP,shared)/enums/navigation_controller_java/org/chromium/content_public/browser/navigation_controller/LoadURLType.java \
$(call intermediates-dir-for,GYP,shared)/enums/navigation_controller_java/org/chromium/content_public/browser/navigation_controller/UserAgentOverrideOption.java \
$(call intermediates-dir-for,GYP,shared)/enums/popup_item_type_java/org/chromium/content/browser/input/PopupItemType.java \
$(call intermediates-dir-for,GYP,shared)/enums/private_key_types_java/org/chromium/net/PrivateKeyType.java \
$(call intermediates-dir-for,GYP,shared)/enums/result_codes_java/org/chromium/content_public/common/ResultCode.java \
$(call intermediates-dir-for,GYP,shared)/enums/screen_orientation_values_java/org/chromium/content_public/common/ScreenOrientationValues.java \
$(call intermediates-dir-for,GYP,shared)/enums/selection_event_type_java/org/chromium/ui/touch_selection/SelectionEventType.java \
$(call intermediates-dir-for,GYP,shared)/enums/speech_recognition_error_java/org/chromium/content_public/common/SpeechRecognitionErrorCode.java \
$(call intermediates-dir-for,GYP,shared)/enums/top_controls_state_java/org/chromium/content_public/common/TopControlsState.java \
$(call intermediates-dir-for,GYP,shared)/enums/window_open_disposition_java/org/chromium/ui/WindowOpenDisposition.java \
$(call intermediates-dir-for,GYP,shared)/enums/base_java_application_state/org/chromium/base/ApplicationState.java \
$(call intermediates-dir-for,GYP,shared)/enums/base_java_library_load_from_apk_status_codes/org/chromium/base/library_loader/LibraryLoadFromApkStatusCodes.java \
$(call intermediates-dir-for,GYP,shared)/enums/base_java_memory_pressure_level/org/chromium/base/MemoryPressureLevel.java \
$(call intermediates-dir-for,GYP,shared)/enums/media_android_imageformat/org/chromium/media/AndroidImageFormat.java \
$(call intermediates-dir-for,GYP,shared)/enums/page_transition_types_java/org/chromium/ui/base/PageTransition.java \
$(call intermediates-dir-for,GYP,shared)/enums/system_ui_resource_type_java/org/chromium/ui/resources/SystemUIResourceType.java \
$(call intermediates-dir-for,GYP,shared)/enums/text_input_type_java/org/chromium/ui/base/ime/TextInputType.java \
$(call intermediates-dir-for,GYP,shared)/enums/touch_device_types_java/org/chromium/ui/base/HoverType.java \
$(call intermediates-dir-for,GYP,shared)/enums/touch_device_types_java/org/chromium/ui/base/PointerType.java \
$(call intermediates-dir-for,GYP,shared)/enums/web_input_event_java/org/chromium/blink_public/web/WebInputEventModifier.java \
$(call intermediates-dir-for,GYP,shared)/enums/web_input_event_java/org/chromium/blink_public/web/WebInputEventType.java \
$(call intermediates-dir-for,GYP,shared)/enums/web_text_input_type/org/chromium/blink_public/web/WebTextInputFlags.java \
$(call intermediates-dir-for,GYP,shared)/templates/net_errors_java/org/chromium/net/NetError.java \
$(call intermediates-dir-for,GYP,shared)/java_mojo/device_battery_mojo_bindings/src/org/chromium/mojom/device/BatteryMonitor.java \
$(call intermediates-dir-for,GYP,shared)/java_mojo/device_battery_mojo_bindings/src/org/chromium/mojom/device/BatteryMonitor_Internal.java \
$(call intermediates-dir-for,GYP,shared)/java_mojo/device_battery_mojo_bindings/src/org/chromium/mojom/device/BatteryStatus.java \

# content dependencies on java components that are provided by the system on
# android
LOCAL_STATIC_JAVA_LIBRARIES += jsr305 guava

