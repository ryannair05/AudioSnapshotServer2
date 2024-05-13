INSTALL_TARGET_PROCESSES = mediaserverd

FINALPACKAGE = 1

THEOS_PACKAGE_SCHEME=rootless

export TARGET := iphone:clang:latest:15.0

include $(THEOS)/makefiles/common.mk

TWEAK_NAME = AudioSnapshotServer
$(TWEAK_NAME)_FILES = Tweak.c

ARCHS = arm64 arm64e

include $(THEOS_MAKE_PATH)/tweak.mk
