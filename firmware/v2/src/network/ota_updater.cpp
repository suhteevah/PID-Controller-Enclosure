#if ENABLE_OTA
#include "ota_updater.h"

OTAUpdater::OTAUpdater() : _progress(0), _updating(false), _error(false) {}

void OTAUpdater::begin() {
    _progress = 0; _updating = false; _error = false; _errorMsg = "";
}

bool OTAUpdater::handleChunk(uint8_t* data, size_t len, size_t index, size_t total, bool final) {
    if (index == 0) {
        _updating = true; _error = false; _progress = 0;
        if (!Update.begin(total)) {
            _error = true; _errorMsg = "Not enough space";
            _updating = false; return false;
        }
    }
    if (Update.write(data, len) != len) {
        _error = true; _errorMsg = "Write failed";
        _updating = false; return false;
    }
    _progress = (float)(index + len) / (float)total * 100.0f;
    if (final) {
        if (!Update.end(true)) {
            _error = true; _errorMsg = "Update finalization failed";
            _updating = false; return false;
        }
        _updating = false; _progress = 100;
    }
    return true;
}
#endif
