#include <SoftwareSerial.h>
#include <PnutSerial.h>

// Create a SoftwareSerial instance (only RX is used for receiving data).
SoftwareSerial mySerial(10, 11);

// Initialize PnutSerial with the SoftwareSerial instance.
PnutSerial altimeter(mySerial);

void setup() {
    Serial.begin(9600);
    // Start the SoftwareSerial at 9600 bps.
    mySerial.begin(9600);
    // Initialize the PnutSerial library.
    altimeter.begin(9600, SERIAL_8N1);
    // Set telemetry mode to ON_PAD (first reading is ground elevation).
    altimeter.setMode(ON_PAD);
    // Set a read timeout of 1000 milliseconds.
    altimeter.setReadTimeout(1000);
    // Optionally enable debug output (for example, send debug messages to Serial).
    // altimeter.setDebugOutput(Serial);
}

void loop() {
    int altitude;
    // Attempt to read a telemetry data point.
    // The function returns PNUT_OK if a valid reading is available.
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
