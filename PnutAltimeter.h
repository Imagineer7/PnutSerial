#ifndef PNUTALTIMETER_H
#define PNUTALTIMETER_H

#include <Arduino.h>
#include <SoftwareSerial.h>

// Define telemetry modes
enum TelemetryMode {
    ON_PAD,    // First reading is ground elevation (MSL), subsequent values are AGL altitudes.
    ON_LAUNCH  // All readings are AGL altitudes.
};

class PnutAltimeter {
public:
    // Constructor: rxPin is where the Arduino reads the altimeter's TX.
    // txPin is provided for compatibility, though it’s unused in our setup.
    PnutAltimeter(uint8_t rxPin, uint8_t txPin);

    // Initialize serial communication with the altimeter.
    void begin();

    // Set telemetry mode: ON_PAD or ON_LAUNCH.
    void setMode(TelemetryMode newMode);

    // Set the read timeout (in milliseconds) for a complete line.
    void setReadTimeout(unsigned long timeoutMs);

    // Read a single telemetry data point.
    // Returns true if a valid reading is obtained and assigns the value to 'altitude'.
    // Returns false if no valid data was received (e.g., due to a timeout or parse error).
    bool readAltitude(int &altitude);

    // Check if serial data is available.
    bool available();

    // Reset the state machine (e.g., when starting a new telemetry session).
    void reset();

    // Get the ground elevation (only valid in ON_PAD mode after the first reading).
    int getGroundElevation() const;

private:
    SoftwareSerial altSerial;
    String buffer;             // Temporary buffer for incoming serial data.
    TelemetryMode mode;        // Telemetry mode: ON_PAD or ON_LAUNCH.
    bool firstReading;         // True until the ground elevation (first reading) is processed.
    int groundElevation;       // Stores the ground elevation when using ON_PAD mode.
    unsigned long readTimeoutMs; // Maximum time to wait for a complete line (in milliseconds).

    // Helper function to check if a string contains only numeric characters.
    bool isNumeric(const String &str);
};

#endif
