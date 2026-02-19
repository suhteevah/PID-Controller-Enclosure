#pragma once

#include <Arduino.h>
#include <Adafruit_MAX31855.h>
#include "config.h"

enum class TCStatus : uint8_t {
    OK,
    OPEN_CIRCUIT,
    SHORT_GND,
    SHORT_VCC,
    READ_ERROR,
    NOT_READY
};

class Thermocouple {
public:
    Thermocouple();
    ~Thermocouple();

    void begin(uint8_t csPin);
    void update();

    float getTemperatureF() const   { return _tempF; }
    float getTemperatureC() const   { return _tempC; }
    float getColdJunctionC() const  { return _coldJunctionC; }
    TCStatus getStatus() const      { return _status; }
    bool isOk() const               { return _status == TCStatus::OK; }
    uint8_t getErrorCount() const   { return _consecutiveErrors; }
    const char* getStatusString() const;

private:
    Adafruit_MAX31855* _sensor;
    float _tempF;
    float _tempC;
    float _coldJunctionC;
    TCStatus _status;
    uint8_t _consecutiveErrors;
    uint32_t _lastReadTime;
    bool _initialized;
};
