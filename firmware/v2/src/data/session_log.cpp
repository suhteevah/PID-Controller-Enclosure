#include "session_log.h"

const char* SessionLogger::LOG_PATH = "/sessions.dat";

SessionLogger::SessionLogger() {
    for (int i = 0; i < NUM_CHANNELS; i++) _active[i] = {false, 0, 0, 0, 0, 0, 0};
}

void SessionLogger::begin() {
    if (!LittleFS.begin(true)) {
        Serial.println(F("[SESSION] LittleFS mount failed"));
    }
}

void SessionLogger::update() { /* periodic maintenance if needed */ }

void SessionLogger::startSession(uint8_t ch, float targetTemp) {
    if (ch >= NUM_CHANNELS) return;
    _active[ch] = {true, millis(), 0, 0, 0, 0, targetTemp};
}

void SessionLogger::endSession(uint8_t ch) {
    if (ch >= NUM_CHANNELS || !_active[ch].active) return;
    ActiveSession& s = _active[ch];
    SessionRecord rec;
    rec.startTime = s.startMs / 1000;
    rec.durationSec = (millis() - s.startMs) / 1000;
    rec.peakTempF = s.peakTemp;
    rec.avgTempF = s.sampleCount > 0 ? s.tempSum / s.sampleCount : 0;
    rec.targetTempF = s.targetTemp;
    rec.channel = ch;
    rec.energyEstWh = s.energySum / 3600.0f;
    appendRecord(rec);
    s.active = false;
}

void SessionLogger::addDataPoint(uint8_t ch, float temp, float pidOutput) {
    if (ch >= NUM_CHANNELS || !_active[ch].active) return;
    ActiveSession& s = _active[ch];
    s.sampleCount++;
    s.tempSum += temp;
    if (temp > s.peakTemp) s.peakTemp = temp;
    // Estimate energy: assume 100W max per channel, scale by PID output
    float watts = (pidOutput / 100.0f) * 100.0f;
    float intervalSec = SESSION_SAMPLE_INTERVAL_S;
    s.energySum += watts * intervalSec;  // Watt-seconds
}

void SessionLogger::appendRecord(const SessionRecord& rec) {
    File f = LittleFS.open(LOG_PATH, "a");
    if (!f) return;
    f.write((const uint8_t*)&rec, sizeof(SessionRecord));
    f.close();
    // Trim to MAX_SESSION_RECORDS
    if (getSessionCount() > MAX_SESSION_RECORDS) {
        // Read all, keep last N, rewrite
        File rf = LittleFS.open(LOG_PATH, "r");
        if (!rf) return;
        size_t total = rf.size() / sizeof(SessionRecord);
        size_t skip = total - MAX_SESSION_RECORDS;
        rf.seek(skip * sizeof(SessionRecord));
        uint8_t* buf = new uint8_t[MAX_SESSION_RECORDS * sizeof(SessionRecord)];
        rf.read(buf, MAX_SESSION_RECORDS * sizeof(SessionRecord));
        rf.close();
        File wf = LittleFS.open(LOG_PATH, "w");
        wf.write(buf, MAX_SESSION_RECORDS * sizeof(SessionRecord));
        wf.close();
        delete[] buf;
    }
}

uint16_t SessionLogger::getSessionCount() {
    File f = LittleFS.open(LOG_PATH, "r");
    if (!f) return 0;
    uint16_t count = f.size() / sizeof(SessionRecord);
    f.close();
    return count;
}

SessionRecord SessionLogger::getSession(uint16_t idx) {
    SessionRecord rec = {};
    File f = LittleFS.open(LOG_PATH, "r");
    if (!f) return rec;
    f.seek(idx * sizeof(SessionRecord));
    f.read((uint8_t*)&rec, sizeof(SessionRecord));
    f.close();
    return rec;
}

String SessionLogger::exportJSON() {
    JsonDocument doc;
    JsonArray arr = doc["sessions"].to<JsonArray>();
    uint16_t count = getSessionCount();
    for (uint16_t i = 0; i < count; i++) {
        SessionRecord r = getSession(i);
        JsonObject o = arr.add<JsonObject>();
        o["start"] = r.startTime;
        o["duration"] = r.durationSec;
        o["channel"] = r.channel;
        o["peakTemp"] = r.peakTempF;
        o["avgTemp"] = r.avgTempF;
        o["targetTemp"] = r.targetTempF;
        o["energyWh"] = r.energyEstWh;
    }
    String out;
    serializeJson(doc, out);
    return out;
}

void SessionLogger::clearAll() {
    LittleFS.remove(LOG_PATH);
}
