#include "PnutSerial.h"

PnutAltimeter::PnutAltimeter(uint8_t rxPin, uint8_t txPin)
    : altSerial(rxPin, txPin), mode(ON_LAUNCH), firstReading(true),
      groundElevation(0), readTimeoutMs(1000) { // default timeout 1000ms
}

void PnutAltimeter::begin() {
    // Set the serial port to 9600 bps.
    altSerial.begin(9600);
}

void PnutAltimeter::setMode(TelemetryMode newMode) {
    mode = newMode;
    reset(); // Reset state when the mode changes.
}

void PnutAltimeter::setReadTimeout(unsigned long timeoutMs) {
    readTimeoutMs = timeoutMs;
}

bool PnutAltimeter::available() {
    return altSerial.available() > 0;
}

void PnutAltimeter::reset() {
    firstReading = true;
    groundElevation = 0;
    buffer = "";
}

int PnutAltimeter::getGroundElevation() const {
    return groundElevation;
}

bool PnutAltimeter::isNumeric(const String &str) {
    if (str.length() == 0) return false;
    for (size_t i = 0; i < str.length(); i++) {
        // Allow only digits. (If negative numbers are possible, you might want to allow a '-' at index 0.)
        if (!isDigit(str.charAt(i))) {
            return false;
        }
    }
    return true;
}

bool PnutAltimeter::readAltitude(int &altitude) {
    unsigned long startTime = millis();
    buffer = "";

    // Buffer characters until a newline is received or until the timeout is reached.
    while (millis() - startTime < readTimeoutMs) {
        while (altSerial.available()) {
            char c = altSerial.read();
            // If newline is reached, exit the inner loop.
            if (c == '\n') {
                break;
            }
            buffer += c;
        }
        // If we have received a carriage return or newline, break out.
        if (buffer.indexOf('\r') != -1 || buffer.indexOf('\n') != -1) {
            break;
        }
    }

    // Remove any leading/trailing whitespace including carriage return.
    buffer.trim();

    // If the buffer is empty, we treat this as a timeout or bad data.
    if (buffer.length() == 0) {
        return false;
    }

    // Check if the buffered data is numeric.
    if (!isNumeric(buffer)) {
        return false;
    }

    // Convert the ASCII string to an integer.
    int value = buffer.toInt();

    // Process the value based on the telemetry mode.
    if (mode == ON_PAD) {
        if (firstReading) {
            // The first reading in ON_PAD mode is the ground elevation (MSL).
            groundElevation = value;
            firstReading = false;
            altitude = value;
            return true;
        } else {
            // Subsequent readings represent AGL altitude.
            altitude = value;
            return true;
        }
    } else if (mode == ON_LAUNCH) {
        // In ON_LAUNCH mode, all readings are AGL altitudes.
        altitude = value;
        return true;
    }

    return false;
}
