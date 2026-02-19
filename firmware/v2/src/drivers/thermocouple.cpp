#include "thermocouple.h"

Thermocouple::Thermocouple()
    : _sensor(nullptr), _tempF(0), _tempC(0), _coldJunctionC(0),
      _status(TCStatus::NOT_READY), _consecutiveErrors(0),
      _lastReadTime(0), _initialized(false) {}

Thermocouple::~Thermocouple() {
    if (_sensor) {
        delete _sensor;
        _sensor = nullptr;
    }
}

void Thermocouple::begin(uint8_t csPin) {
    if (_sensor) delete _sensor;

    _sensor = new Adafruit_MAX31855(PIN_SPI_SCK, csPin, PIN_SPI_MISO);
    if (!_sensor->begin()) {
        _status = TCStatus::READ_ERROR;
        return;
    }

    _status = TCStatus::NOT_READY;
    _initialized = true;
    _lastReadTime = millis();

    // Initial settling read
    delay(100);
    _sensor->readCelsius();
}

void Thermocouple::update() {
    if (!_initialized || !_sensor) return;

    uint32_t now = millis();
    if ((now - _lastReadTime) < TC_READ_INTERVAL_MS) return;
    _lastReadTime = now;

    double tempC = _sensor->readCelsius();
    uint8_t fault = _sensor->readError();

    if (fault) {
        _consecutiveErrors++;
        if (fault & MAX31855_FAULT_OPEN)        _status = TCStatus::OPEN_CIRCUIT;
        else if (fault & MAX31855_FAULT_SHORT_GND) _status = TCStatus::SHORT_GND;
        else if (fault & MAX31855_FAULT_SHORT_VCC) _status = TCStatus::SHORT_VCC;
        else                                    _status = TCStatus::READ_ERROR;
        return;
    }

    if (isnan(tempC)) {
        _consecutiveErrors++;
        _status = TCStatus::READ_ERROR;
        return;
    }

    _consecutiveErrors = 0;
    _tempC = (float)tempC;
    _tempF = _tempC * 9.0f / 5.0f + 32.0f;
    _coldJunctionC = (float)_sensor->readInternal();
    _status = TCStatus::OK;
}

const char* Thermocouple::getStatusString() const {
    switch (_status) {
        case TCStatus::OK:           return "OK";
        case TCStatus::OPEN_CIRCUIT: return "OPEN";
        case TCStatus::SHORT_GND:    return "SHORT-GND";
        case TCStatus::SHORT_VCC:    return "SHORT-VCC";
        case TCStatus::READ_ERROR:   return "ERROR";
        case TCStatus::NOT_READY:    return "INIT";
        default:                     return "UNKNOWN";
    }
}
