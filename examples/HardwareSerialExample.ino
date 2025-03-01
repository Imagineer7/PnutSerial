#include <PnutSerial.h>

// Use a hardware serial port for the altimeter.
// For boards with multiple hardware serial ports (e.g., Arduino Mega),
// Serial1 is used for the altimeter, while Serial remains for debugging.
PnutSerial altimeter(Serial1);

void setup() {
    // Initialize the debugging Serial port.
    Serial.begin(9600);
    while (!Serial) {
        ; // Wait for Serial port connection (needed for some boards).
    }
    Serial.println("Starting PnutSerial hardware serial example...");

    // Initialize the hardware serial port for the altimeter.
    // For HardwareSerial objects, begin() will configure the port.
    altimeter.begin(9600, SERIAL_8N1);

    // Set telemetry mode to ON_PAD (first reading is ground elevation).
    altimeter.setMode(ON_PAD);

    // Set a read timeout of 1000 milliseconds.
    altimeter.setReadTimeout(1000);

    // Optionally, enable debug output from the library (e.g., to Serial).
    // altimeter.setDebugOutput(Serial);
}

void loop() {
    int altitude;
    // Attempt to read a telemetry data point.
    if (altimeter.readAltitude(altitude) == PNUT_OK) {
        // In ON_PAD mode, the first reading is the ground elevation (MSL).
        if (altimeter.getGroundElevation() == altitude && altitude != 0) {
            Serial.print("Ground Elevation (MSL): ");
            Serial.println(altimeter.getGroundElevation());
        } else {
            Serial.print("AGL Altitude: ");
            Serial.println(altitude);
        }
    } else {
        // Handle read errors or timeouts.
        Serial.println("Error: No valid data received or parsing error.");
    }
}
