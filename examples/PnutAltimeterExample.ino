#include <PnutAltimeter.h>

// Example: using pins 10 (RX) and 11 (TX) for SoftwareSerial (only RX is used).
PnutAltimeter altimeter(10, 11);

void setup() {
    Serial.begin(9600);
    altimeter.begin();
    altimeter.setMode(ON_PAD);           // Set to ON_PAD mode if needed.
    altimeter.setReadTimeout(1000);       // Set a 1-second timeout for reading.
}

void loop() {
    int altitude;
    // Attempt to read a telemetry data point.
    if (altimeter.readAltitude(altitude)) {
        // In ON_PAD mode, the first reading is ground elevation.
        if (altimeter.getGroundElevation() == altitude && altitude != 0) {
            Serial.print("Ground Elevation (MSL): ");
            Serial.println(altimeter.getGroundElevation());
        } else {
            Serial.print("AGL Altitude: ");
            Serial.println(altitude);
        }
    } else {
        // Optional: Handle a timeout or error in data parsing.
        Serial.println("Error: No valid data received or parsing error.");
    }
}
