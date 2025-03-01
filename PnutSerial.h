#ifndef PNUTSERIAL_H
#define PNUTSERIAL_H

#include <Arduino.h>

// Telemetry mode: In ON_PAD mode the first reading is the ground elevation (MSL),
// and subsequent readings are AGL altitudes. In ON_LAUNCH mode, all readings are AGL.
enum TelemetryMode {
    ON_PAD,
    ON_LAUNCH
};

// Error codes returned by the library.
enum PnutAltimeterError {
    PNUT_OK = 0,
    PNUT_ERR_TIMEOUT,
    PNUT_ERR_NON_NUMERIC,
    PNUT_ERR_INCOMPLETE,
    PNUT_ERR_CHECKSUM_MISMATCH,
    PNUT_ERR_NO_DATA,
};

class PnutAltimeter {
public:
    // Constructor accepts any Stream-derived object.
    PnutAltimeter(Stream &serial);

    // Initialize the serial interface.
    // For streams that support begin() (e.g., HardwareSerial), the library will attempt
    // to initialize them with the given baud rate and configuration.
    // For others (like many SoftwareSerial implementations), the user may need to begin() externally.
    void begin(uint32_t baudRate = 9600, uint16_t config = SERIAL_8N1);

    // Set telemetry mode: ON_PAD or ON_LAUNCH.
    void setMode(TelemetryMode mode);

    // Set the read timeout (in milliseconds) for receiving a complete data line.
    void setReadTimeout(unsigned long timeoutMs);

    // Enable debug logging by providing a Print-derived object (e.g., Serial).
    void setDebugOutput(Print *debugPort);

    // Process incoming serial data: fill the ring buffer and parse complete lines.
    // This function should be called frequently (for example, inside loop()).
    void processSerial();

    // Retrieve the next parsed altitude reading from the internal queue.
    // Returns PNUT_OK if a reading was available, or an error code if not.
    PnutAltimeterError getNextReading(int &altitude);

    // A convenience function that calls processSerial() and then attempts to get a reading.
    // It waits up to the configured timeout for a valid reading.
    PnutAltimeterError readAltitude(int &altitude);

    // Reset the internal state (buffers, queues, and mode state).
    void reset();

    // Retrieve the ground elevation (only valid in ON_PAD mode after the first reading).
    int getGroundElevation() const;

private:
    Stream *_serial;             // Pointer to the serial interface.
    TelemetryMode _mode;         // Telemetry mode.
    bool _firstReading;          // Indicates if the ground elevation has not been read yet.
    int _groundElevation;        // Stored ground elevation in ON_PAD mode.
    unsigned long _readTimeoutMs; // Read timeout in milliseconds.
    Print *_debug;               // Optional debug output.

    // Ring buffer for incoming characters.
    static const size_t BUFFER_SIZE = 256;
    char _ringBuffer[BUFFER_SIZE];
    size_t _bufferHead;
    size_t _bufferTail;

    // Queue for parsed altitude values.
    static const size_t QUEUE_SIZE = 16;
    int _parsedQueue[QUEUE_SIZE];
    size_t _queueHead;
    size_t _queueTail;

    // Helper functions for ring buffer management.
    bool bufferIsEmpty() const;
    bool bufferIsFull() const;
    void pushToBuffer(char c);
    bool popFromBuffer(char &c);
    // Attempt to extract a complete line (terminated by '\n') from the ring buffer.
    bool popLine(String &line);

    // Parse a line and output a numeric altitude value.
    // This function also checks for an optional checksum if the line contains '*'.
    PnutAltimeterError parseLine(const String &line, int &value);

    // Compute a simple checksum: the sum of ASCII values modulo 256.
    // Used when the line format is "value*checksum" (checksum in two hex digits).
    uint8_t computeChecksum(const String &data);

    // Queue management for parsed altitude values.
    void enqueueValue(int value);
    bool dequeueValue(int &value);
};

#endif
