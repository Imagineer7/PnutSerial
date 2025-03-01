# PnutSerial Arduino Library

The **PnutAltimeter** library is an Arduino library designed to interface with the PerfectFlight Pnut altimeter. It simplifies reading telemetry data by handling serial communication, parsing ASCII data, managing telemetry modes, and providing robust error handling with timeouts and multi-line buffering.

## Features

- **Flexible Serial Communication:**  
  Easily integrates with HardwareSerial or SoftwareSerial interfaces.

- **Telemetry Modes:**  
  - **ON_PAD:** The first data point is the ground elevation (MSL), while subsequent readings are AGL altitudes.
  - **ON_LAUNCH:** All readings are AGL altitudes.

- **Data Parsing Enhancements:**  
  Buffers complete lines of data, trims extraneous characters, and validates numeric input.

- **Robust Error Handling:**  
  Detects empty or non-numeric data and includes a timeout mechanism to prevent the system from hanging during transmission pauses.

- **Easy Integration:**  
  A straightforward API designed for quick setup and use in your projects.

## Requirements

- Arduino IDE 1.8.x or newer
- Arduino board (e.g., Uno, Mega, etc.)
- [SoftwareSerial library](https://www.arduino.cc/en/Reference/SoftwareSerial) (if using SoftwareSerial)

## Installation

1. **Clone or Download:**  
   Clone this repository or download the ZIP file.

2. **Copy to Libraries Folder:**  
   Place the `PnutAltimeter` folder into your Arduino libraries directory, typically found at:  
   `~/Documents/Arduino/libraries/`

3. **Restart the Arduino IDE:**  
   If the IDE is open, close and reopen it to recognize the new library.

## Usage

### Example Sketch

Below is an example sketch demonstrating how to use the library with a SoftwareSerial interface. Adjust the pins if you're using a different configuration or a hardware serial port.

```cpp
#include <PnutAltimeter.h>

// Initialize PnutAltimeter on SoftwareSerial pins 10 (RX) and 11 (TX)
// Note: Only the RX pin is used for receiving data from the altimeter.
PnutAltimeter altimeter(10, 11);

void setup() {
    Serial.begin(9600);
    altimeter.begin();
    // Set telemetry mode to ON_PAD (first reading is ground elevation)
    altimeter.setMode(ON_PAD);
    // Set a read timeout of 1000 milliseconds
    altimeter.setReadTimeout(1000);
}

void loop() {
    int altitude;
    // Attempt to read a telemetry data point
    if (altimeter.readAltitude(altitude)) {
        // In ON_PAD mode, the first reading is the ground elevation (MSL)
        if (altimeter.getGroundElevation() == altitude && altitude != 0) {
            Serial.print("Ground Elevation (MSL): ");
            Serial.println(altimeter.getGroundElevation());
        } else {
            Serial.print("AGL Altitude: ");
            Serial.println(altitude);
        }
    } else {
        // Handle read errors or timeouts
        Serial.println("Error: No valid data received or parsing error.");
    }
}
