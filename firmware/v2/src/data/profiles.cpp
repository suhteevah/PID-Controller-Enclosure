#include "profiles.h"

// ============================================================
// ESP-Nail v2 - Temperature Profile Manager
// ============================================================

// NVS namespace for profiles (separate from main settings to avoid key collisions)
static const char* PROFILE_NS = "enail2_prof";

ProfileManager::ProfileManager() {
    memset(_profiles, 0, sizeof(_profiles));
    memset(_activeIndex, 0, sizeof(_activeIndex));
    memset(_profileCount, 0, sizeof(_profileCount));
}

bool ProfileManager::begin() {
    // Try to read profiles from NVS. If nothing stored, load defaults.
    _prefs.begin(PROFILE_NS, true);
    bool hasData = _prefs.getBool("init", false);
    _prefs.end();

    if (!hasData) {
        Serial.println("[Profiles] No stored profiles found, loading defaults");
        loadDefaults();
    } else {
        for (uint8_t ch = 0; ch < NUM_CHANNELS; ch++) {
            loadChannelFromNVS(ch);
        }
        Serial.println("[Profiles] Loaded profiles from NVS");
    }

    return true;
}

// --- Key builders ---

String ProfileManager::profileKey(uint8_t ch, uint8_t idx, const char* field) {
    char buf[24];
    snprintf(buf, sizeof(buf), "c%up%u_%s", ch, idx, field);
    return String(buf);
}

String ProfileManager::channelMetaKey(uint8_t ch, const char* field) {
    char buf[16];
    snprintf(buf, sizeof(buf), "c%u_%s", ch, field);
    return String(buf);
}

// --- Default profiles ---

void ProfileManager::getDefaultProfile(uint8_t idx, Profile& out) {
    memset(&out, 0, sizeof(Profile));
    out.hasCustomPID = false;
    out.kp = PID_KP_DEFAULT;
    out.ki = PID_KI_DEFAULT;
    out.kd = PID_KD_DEFAULT;

    switch (idx) {
        case 0:
            strncpy(out.name, "Low Temp", PROFILE_NAME_MAX_LEN - 1);
            out.tempF = 500.0f;
            break;
        case 1:
            strncpy(out.name, "Medium", PROFILE_NAME_MAX_LEN - 1);
            out.tempF = 620.0f;
            break;
        case 2:
            strncpy(out.name, "Standard", PROFILE_NAME_MAX_LEN - 1);
            out.tempF = 710.0f;
            break;
        case 3:
            strncpy(out.name, "Hot", PROFILE_NAME_MAX_LEN - 1);
            out.tempF = 800.0f;
            break;
        default:
            strncpy(out.name, "Custom", PROFILE_NAME_MAX_LEN - 1);
            out.tempF = TEMP_DEFAULT_F;
            break;
    }
}

void ProfileManager::loadDefaults() {
    for (uint8_t ch = 0; ch < NUM_CHANNELS; ch++) {
        _profileCount[ch] = NUM_DEFAULT_PROFILES;
        _activeIndex[ch] = 2;  // "Standard" by default

        for (uint8_t i = 0; i < MAX_PROFILES_PER_CH; i++) {
            if (i < NUM_DEFAULT_PROFILES) {
                getDefaultProfile(i, _profiles[ch][i]);
            } else {
                memset(&_profiles[ch][i], 0, sizeof(Profile));
            }
        }
    }

    // Persist to NVS
    _prefs.begin(PROFILE_NS, false);
    _prefs.clear();
    _prefs.putBool("init", true);
    _prefs.end();

    for (uint8_t ch = 0; ch < NUM_CHANNELS; ch++) {
        _prefs.begin(PROFILE_NS, false);
        _prefs.putUChar(channelMetaKey(ch, "cnt").c_str(), _profileCount[ch]);
        _prefs.putUChar(channelMetaKey(ch, "act").c_str(), _activeIndex[ch]);
        _prefs.end();

        for (uint8_t i = 0; i < _profileCount[ch]; i++) {
            saveProfileToNVS(ch, i);
        }
    }

    Serial.println("[Profiles] Default profiles loaded and saved");
}

// --- NVS persistence ---

void ProfileManager::saveProfileToNVS(uint8_t ch, uint8_t idx) {
    if (ch >= NUM_CHANNELS || idx >= MAX_PROFILES_PER_CH) return;

    const Profile& p = _profiles[ch][idx];

    _prefs.begin(PROFILE_NS, false);
    _prefs.putString(profileKey(ch, idx, "name").c_str(), p.name);
    _prefs.putFloat(profileKey(ch, idx, "temp").c_str(),  p.tempF);
    _prefs.putFloat(profileKey(ch, idx, "kp").c_str(),    p.kp);
    _prefs.putFloat(profileKey(ch, idx, "ki").c_str(),    p.ki);
    _prefs.putFloat(profileKey(ch, idx, "kd").c_str(),    p.kd);
    _prefs.putBool(profileKey(ch, idx, "cpid").c_str(),   p.hasCustomPID);
    _prefs.end();
}

void ProfileManager::loadChannelFromNVS(uint8_t ch) {
    if (ch >= NUM_CHANNELS) return;

    _prefs.begin(PROFILE_NS, true);
    _profileCount[ch] = _prefs.getUChar(channelMetaKey(ch, "cnt").c_str(), NUM_DEFAULT_PROFILES);
    _activeIndex[ch]  = _prefs.getUChar(channelMetaKey(ch, "act").c_str(), 2);
    _prefs.end();

    // Clamp values
    if (_profileCount[ch] > MAX_PROFILES_PER_CH) _profileCount[ch] = MAX_PROFILES_PER_CH;
    if (_activeIndex[ch] >= _profileCount[ch]) _activeIndex[ch] = 0;

    for (uint8_t i = 0; i < _profileCount[ch]; i++) {
        Profile& p = _profiles[ch][i];
        memset(&p, 0, sizeof(Profile));

        _prefs.begin(PROFILE_NS, true);
        String name = _prefs.getString(profileKey(ch, i, "name").c_str(), "");
        strncpy(p.name, name.c_str(), PROFILE_NAME_MAX_LEN - 1);
        p.name[PROFILE_NAME_MAX_LEN - 1] = '\0';

        p.tempF       = _prefs.getFloat(profileKey(ch, i, "temp").c_str(), TEMP_DEFAULT_F);
        p.kp          = _prefs.getFloat(profileKey(ch, i, "kp").c_str(),   PID_KP_DEFAULT);
        p.ki          = _prefs.getFloat(profileKey(ch, i, "ki").c_str(),   PID_KI_DEFAULT);
        p.kd          = _prefs.getFloat(profileKey(ch, i, "kd").c_str(),   PID_KD_DEFAULT);
        p.hasCustomPID = _prefs.getBool(profileKey(ch, i, "cpid").c_str(), false);
        _prefs.end();

        // Validate temperature
        if (p.tempF < TEMP_MIN_F || p.tempF > TEMP_MAX_F || isnan(p.tempF)) {
            p.tempF = TEMP_DEFAULT_F;
        }
        // Validate PID values
        if (p.kp < 0.0f || p.kp > 100.0f || isnan(p.kp)) p.kp = PID_KP_DEFAULT;
        if (p.ki < 0.0f || p.ki > 50.0f  || isnan(p.ki)) p.ki = PID_KI_DEFAULT;
        if (p.kd < 0.0f || p.kd > 100.0f || isnan(p.kd)) p.kd = PID_KD_DEFAULT;
    }

    // Clear unused profile slots
    for (uint8_t i = _profileCount[ch]; i < MAX_PROFILES_PER_CH; i++) {
        memset(&_profiles[ch][i], 0, sizeof(Profile));
    }
}

// --- Public API ---

bool ProfileManager::getProfile(uint8_t ch, uint8_t idx, Profile& outProfile) {
    if (ch >= NUM_CHANNELS || idx >= _profileCount[ch]) return false;
    outProfile = _profiles[ch][idx];
    return true;
}

bool ProfileManager::setProfile(uint8_t ch, uint8_t idx, const Profile& profile) {
    if (ch >= NUM_CHANNELS || idx >= MAX_PROFILES_PER_CH) return false;

    _profiles[ch][idx] = profile;
    // Ensure null termination
    _profiles[ch][idx].name[PROFILE_NAME_MAX_LEN - 1] = '\0';

    // Validate
    Profile& p = _profiles[ch][idx];
    if (p.tempF < TEMP_MIN_F || p.tempF > TEMP_MAX_F || isnan(p.tempF)) {
        p.tempF = TEMP_DEFAULT_F;
    }
    if (p.kp < 0.0f || p.kp > 100.0f || isnan(p.kp)) p.kp = PID_KP_DEFAULT;
    if (p.ki < 0.0f || p.ki > 50.0f  || isnan(p.ki)) p.ki = PID_KI_DEFAULT;
    if (p.kd < 0.0f || p.kd > 100.0f || isnan(p.kd)) p.kd = PID_KD_DEFAULT;

    // Update count if expanding
    if (idx >= _profileCount[ch]) {
        _profileCount[ch] = idx + 1;
        _prefs.begin(PROFILE_NS, false);
        _prefs.putUChar(channelMetaKey(ch, "cnt").c_str(), _profileCount[ch]);
        _prefs.end();
    }

    saveProfileToNVS(ch, idx);

    Serial.printf("[Profiles] Set ch%u profile %u: \"%s\" @ %.0fF\n", ch, idx, p.name, p.tempF);
    return true;
}

bool ProfileManager::getActiveProfile(uint8_t ch, Profile& outProfile) {
    if (ch >= NUM_CHANNELS) return false;
    return getProfile(ch, _activeIndex[ch], outProfile);
}

bool ProfileManager::setActiveProfile(uint8_t ch, uint8_t idx) {
    if (ch >= NUM_CHANNELS || idx >= _profileCount[ch]) return false;

    _activeIndex[ch] = idx;

    _prefs.begin(PROFILE_NS, false);
    _prefs.putUChar(channelMetaKey(ch, "act").c_str(), idx);
    _prefs.end();

    Serial.printf("[Profiles] Ch%u active profile set to %u\n", ch, idx);
    return true;
}

uint8_t ProfileManager::getActiveProfileIndex(uint8_t ch) {
    if (ch >= NUM_CHANNELS) return 0;
    return _activeIndex[ch];
}

uint8_t ProfileManager::getProfileCount(uint8_t ch) {
    if (ch >= NUM_CHANNELS) return 0;
    return _profileCount[ch];
}
