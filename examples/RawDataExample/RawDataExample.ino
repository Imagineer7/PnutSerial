#include <SoftwareSerial.h>
#include "PnutSerial.h"

SoftwareSerial mySerial(10, 11);
PnutSerial altimeter(mySerial);

void logRawData(const String &rawData) {
    // For example, print the raw data to the Serial monitor.
    Serial.print("Raw data: ");
    Serial.println(rawData);
}

void setup() {
    Serial.begin(9600);
    mySerial.begin(9600);
    
    // Register the callback function
    altimeter.setRawDataCallback(logRawData);
    
    // Set mode if needed (ON_PAD or ON_LAUNCH)
    altimeter.setMode(ON_PAD);
}

void loop() {
    // Process incoming data and retrieve altitude reading if available.
    int altitude;
    if (altimeter.readAltitude(altitude) == PNUT_OK) {
        Serial.print("Altitude: ");
        Serial.println(altitude);
    }
}
