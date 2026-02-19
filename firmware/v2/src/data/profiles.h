#pragma once

// ============================================================
// ESP-Nail v2 - Temperature Profile Manager
// Stores named temperature + PID profiles per channel in NVS
// ============================================================

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

// --- Profile Data ---
struct Profile {
    char name[PROFILE_NAME_MAX_LEN];
    float tempF;
    float kp;
    float ki;
    float kd;
    bool hasCustomPID;
};

// Number of built-in default profiles
#define NUM_DEFAULT_PROFILES 4

class ProfileManager {
public:
    ProfileManager();

    /// Initialize and load profiles from NVS
    bool begin();

    /// Get a profile for a channel. Returns true if valid.
    bool getProfile(uint8_t ch, uint8_t idx, Profile& outProfile);

    /// Set / update a profile for a channel
    bool setProfile(uint8_t ch, uint8_t idx, const Profile& profile);

    /// Get the currently active profile for a channel
    bool getActiveProfile(uint8_t ch, Profile& outProfile);

    /// Set the active profile index for a channel
    bool setActiveProfile(uint8_t ch, uint8_t idx);

    /// Get active profile index for a channel
    uint8_t getActiveProfileIndex(uint8_t ch);

    /// Get the number of non-empty profiles for a channel
    uint8_t getProfileCount(uint8_t ch);

    /// Load factory default profiles into all channels
    void loadDefaults();

private:
    // In-memory profile cache per channel
    Profile _profiles[NUM_CHANNELS][MAX_PROFILES_PER_CH];
    uint8_t _activeIndex[NUM_CHANNELS];
    uint8_t _profileCount[NUM_CHANNELS];

    Preferences _prefs;

    /// Build NVS key for a channel profile entry
    String profileKey(uint8_t ch, uint8_t idx, const char* field);

    /// Build NVS key for channel metadata
    String channelMetaKey(uint8_t ch, const char* field);

    /// Save a single profile to NVS
    void saveProfileToNVS(uint8_t ch, uint8_t idx);

    /// Load all profiles for a channel from NVS
    void loadChannelFromNVS(uint8_t ch);

    /// Populate a Profile with the default at given index (0..3)
    void getDefaultProfile(uint8_t idx, Profile& out);
};
