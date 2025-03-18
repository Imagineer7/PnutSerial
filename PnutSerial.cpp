#include "PnutSerial.h"

// Constructor
PnutSerial::PnutSerial(Stream &serial)
    : _serial(&serial),
      _mode(ON_LAUNCH),
      _firstReading(true),
      _groundElevation(0),
      _readTimeoutMs(1000),
      _debug(NULL),
      _bufferHead(0),
      _bufferTail(0),
      _queueHead(0),
      _queueTail(0),
      _rawDataCallback(NULL)
{
}

// begin(): Attempt to initialize the serial interface if possible.
void PnutSerial::begin(uint32_t baudRate, uint16_t config) {
    // For HardwareSerial objects, we can call begin().
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM)
    HardwareSerial* hs = dynamic_cast<HardwareSerial*>(_serial);
    if (hs) {
        hs->begin(baudRate, config);
    }
#endif
    // For other Stream objects (like many SoftwareSerial implementations), the user should
    // initialize them externally.
}

// Set the telemetry mode and reset the state.
void PnutSerial::setMode(TelemetryMode mode) {
    _mode = mode;
    reset();
}

// Set the read timeout in milliseconds.
void PnutSerial::setReadTimeout(unsigned long timeoutMs) {
    _readTimeoutMs = timeoutMs;
}

// Set an optional debug output (for example, Serial).
void PnutSerial::setDebugOutput(Print *debugPort) {
    _debug = debugPort;
}

// Process incoming serial data by reading available characters into the ring buffer,
// then extract complete lines and parse them.
void PnutSerial::processSerial() {
    while (_serial->available()) {
        char c = _serial->read();
        if (!bufferIsFull()) {
            pushToBuffer(c);
        } else {
            if (_debug) {
                _debug->println("Ring buffer full, dropping data.");
            }
        }
    }

    // Process complete lines from the ring buffer.
    String line;
    while (popLine(line)) {
        //If a raw data callback is set, call it with the unmodified line.
        if (_rawDataCallback) {
            _rawDataCallback(line);
        }
        if (_debug) {
            _debug->print("Received line: ");
            _debug->println(line);
        }
        int value;
        PnutSerialError err = parseLine(line, value);
        if (err == PNUT_OK) {
            // In ON_PAD mode, the first reading is the ground elevation.
            if (_mode == ON_PAD && _firstReading) {
                _groundElevation = value;
                _firstReading = false;
                if (_debug) {
                    _debug->print("Ground elevation set to: ");
                    _debug->println(_groundElevation);
                }
            }
            enqueueValue(value);
        } else {
            if (_debug) {
                _debug->print("Error parsing line: ");
                _debug->println(err);
            }
        }
    }
}

//Setter for the raw data callback
void PnutSerial::setRawDataCallback(RawDataCallback callback) {
    _rawDataCallback = callback;
}

// Retrieve the next parsed altitude from the internal queue.
PnutSerialError PnutSerial::getNextReading(int &altitude) {
    if (dequeueValue(altitude)) {
        return PNUT_OK;
    }
    return PNUT_ERR_NO_DATA;
}

// Convenience function: wait up to the timeout period while processing serial input,
// then return the next available altitude reading.
PnutSerialError PnutSerial::readAltitude(int &altitude) {
    unsigned long startTime = millis();
    while (millis() - startTime < _readTimeoutMs) {
        processSerial();
        if (dequeueValue(altitude)) {
            return PNUT_OK;
        }
    }
    return PNUT_ERR_TIMEOUT;
}

// Reset internal buffers, queues, and telemetry state.
void PnutSerial::reset() {
    _bufferHead = 0;
    _bufferTail = 0;
    _queueHead = 0;
    _queueTail = 0;
    _firstReading = true;
    _groundElevation = 0;
}

// Return the ground elevation (valid only in ON_PAD mode after the first reading).
int PnutSerial::getGroundElevation() const {
    return _groundElevation;
}

// ----------------------
// Ring Buffer Management

bool PnutSerial::bufferIsEmpty() const {
    return (_bufferHead == _bufferTail);
}

bool PnutSerial::bufferIsFull() const {
    return ((_bufferHead + 1) % BUFFER_SIZE) == _bufferTail;
}

void PnutSerial::pushToBuffer(char c) {
    _ringBuffer[_bufferHead] = c;
    _bufferHead = (_bufferHead + 1) % BUFFER_SIZE;
}

bool PnutSerial::popFromBuffer(char &c) {
    if (bufferIsEmpty()) {
        return false;
    }
    c = _ringBuffer[_bufferTail];
    _bufferTail = (_bufferTail + 1) % BUFFER_SIZE;
    return true;
}

// Extract a complete line (terminated by '\n') from the ring buffer.
bool PnutSerial::popLine(String &line) {
    line = "";
    size_t tempTail = _bufferTail;
    bool foundNewline = false;
    while (tempTail != _bufferHead) {
        char c = _ringBuffer[tempTail];
        if (c == '\n') {
            foundNewline = true;
            break;
        }
        tempTail = (tempTail + 1) % BUFFER_SIZE;
    }
    if (!foundNewline) {
        return false; // No complete line available.
    }
    // Extract characters up to (but not including) the newline.
    while (_bufferTail != _bufferHead) {
        char c;
        popFromBuffer(c);
        if (c == '\n') {
            break;
        }
        line += c;
    }
    line.trim();
    return true;
}

// ----------------------
// Queue Management for Parsed Altitudes

void PnutSerial::enqueueValue(int value) {
    size_t nextPos = (_queueHead + 1) % QUEUE_SIZE;
    if (nextPos != _queueTail) { // Queue is not full.
        _parsedQueue[_queueHead] = value;
        _queueHead = nextPos;
    } else {
        if (_debug) {
            _debug->println("Parsed queue full, dropping value.");
        }
    }
}

bool PnutSerial::dequeueValue(int &value) {
    if (_queueTail == _queueHead) {
        return false; // Queue is empty.
    }
    value = _parsedQueue[_queueTail];
    _queueTail = (_queueTail + 1) % QUEUE_SIZE;
    return true;
}

// ----------------------
// Line Parsing and Checksum Verification

PnutSerialError PnutSerial::parseLine(const String &line, int &value) {
    // If a '*' is present, assume the format "data*checksum".
    String dataPart = line;
    String checksumPart = "";
    int starIndex = line.indexOf('*');
    if (starIndex != -1) {
        dataPart = line.substring(0, starIndex);
        checksumPart = line.substring(starIndex + 1);
        // Validate checksum.
        uint8_t computed = computeChecksum(dataPart);
        uint8_t provided = (uint8_t)strtoul(checksumPart.c_str(), NULL, 16);
        if (computed != provided) {
            return PNUT_ERR_CHECKSUM_MISMATCH;
        }
    }
    dataPart.trim();
    if (dataPart.length() == 0) {
        return PNUT_ERR_INCOMPLETE;
    }
    // Verify that dataPart is entirely numeric.
    for (size_t i = 0; i < dataPart.length(); i++) {
        if (!isDigit(dataPart.charAt(i))) {
            return PNUT_ERR_NON_NUMERIC;
        }
    }
    value = dataPart.toInt();
    return PNUT_OK;
}

// Compute a simple checksum: sum of ASCII values modulo 256.
uint8_t PnutSerial::computeChecksum(const String &data) {
    uint16_t sum = 0;
    for (size_t i = 0; i < data.length(); i++) {
        sum += data.charAt(i);
    }
    return (uint8_t)(sum & 0xFF);
}
