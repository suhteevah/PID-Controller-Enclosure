#pragma once

#define FW_VERSION_MAJOR    2
#define FW_VERSION_MINOR    0
#define FW_VERSION_PATCH    0
#define FW_VERSION_TAG      "alpha"

#define FW_VERSION_STRING   "2.0.0-alpha"
#define FW_BUILD_DATE       __DATE__
#define FW_BUILD_TIME       __TIME__

// OTA version check: newer version = higher number
#define FW_VERSION_NUMBER   ((FW_VERSION_MAJOR * 10000) + (FW_VERSION_MINOR * 100) + FW_VERSION_PATCH)
