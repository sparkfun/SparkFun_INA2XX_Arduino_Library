/*
  Example 02 - Alert Configuration

  This example demonstrates how to configure the ALERT pin to signal
  when voltage or current thresholds are exceeded. The SparkFun Qwiic Power
  Monitor board breaks out the ALERT pin for use with an external interrupt.

  Select the device you're using by uncommenting the matching declaration below.
  Alert threshold registers and diagnostic flags are shared between both devices.

  The ALERT pin is active-low open-drain by default. This example sets up:
    - Bus overvoltage threshold at 14V
    - Shunt overvoltage (overcurrent) threshold at ~5A
    - Latched alert mode (flags stay set until the DIAG_ALRT register is read)

  SparkFun Electronics
  Date: 2026
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> INA228 or INA237
  QWIIC --> QWIIC
  ALERT pin --> GPIO pin 2 (with external pull-up to 3.3V)

  Serial.print it out at 115200 baud to serial monitor.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/XXXXX
*/

#include <SparkFun_INA2XX.h>

// Uncomment the sensor you're using
// SfeINA228ArdI2C myINA;
SfeINA237ArdI2C myINA;

const int alertPin = 2;  // Connect the ALERT pin to this GPIO

// Alert thresholds in millivolts. The library converts these to register
// values internally — no LSB math required.
const float busOverVoltage_mV = 14000.0f;  // 14 V bus overvoltage
const float shuntOverVoltage_mV = 75.0f;   // 75 mV across the 15 mOhm shunt = ~5 A

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("INA2XX Example 02 - Alert Configuration");

    Wire.begin();
    pinMode(alertPin, INPUT_PULLUP);

    // begin() initializes the I2C bus and verifies the device by checking its Device ID.
    if (!myINA.begin())
    {
        Serial.println("Power monitor not found. Please check wiring. Freezing...");
        while (1)
            delay(1000);
    }
    Serial.println("Power monitor detected!");

    // Calibrate for 15 mOhm shunt, 10A max.
    if (myINA.calibrate(0.015, 10.0) != ksfTkErrOk)
    {
        Serial.println("Calibration failed. Freezing...");
        while (1)
            delay(1000);
    }

    // Set up alert thresholds, in millivolts.
    // All alert/diagnostic registers are in the base class, so both devices
    // support the same interface.
    myINA.setBusOverVoltageThreshold_mV(busOverVoltage_mV);
    myINA.setShuntOverVoltageThreshold_mV(shuntOverVoltage_mV);
    myINA.setAlertLatch(true);
    myINA.setAlertPolarity(false);

    Serial.println("Alert thresholds configured:");
    Serial.print("  Bus OV:   ");
    Serial.print(busOverVoltage_mV / 1000.0f, 1);
    Serial.println(" V");
    Serial.print("  Shunt OV: ");
    Serial.print(shuntOverVoltage_mV, 1);
    Serial.println(" mV");
    Serial.println("  Mode:     Latched, active-low");
    Serial.println();
}

void loop()
{
    // Check the ALERT pin.
    if (digitalRead(alertPin) == LOW)
    {
        Serial.println("*** ALERT TRIGGERED ***");

        // Read diagnostic flags to determine what caused the alert.
        // This also clears the latched flags. The flags come back as a bitfield struct.
        sfe_ina2xx_diag_alrt_reg_t diagFlags = {};
        if (myINA.getDiagnosticFlags(diagFlags) == ksfTkErrOk)
        {
            if (diagFlags.busOL)
                Serial.println("  -> Bus overvoltage detected!");

            if (diagFlags.shntOL)
                Serial.println("  -> Shunt overvoltage (overcurrent) detected!");

            if (diagFlags.tmpOL)
                Serial.println("  -> Temperature over-limit detected!");

            if (diagFlags.mathOF)
                Serial.println("  -> Math overflow in current/power calculation!");
        }

        Serial.println();
    }

    // Print current readings.
    float busV = 0.0f, current = 0.0f;
    myINA.getBusVoltage_V(busV);
    myINA.getCurrent_A(current);

    Serial.print("Bus: ");
    Serial.print(busV, 3);
    Serial.print(" V  |  Current: ");
    Serial.print(current, 4);
    Serial.print(" A  |  Alert pin: ");
    Serial.println(digitalRead(alertPin) ? "HIGH (OK)" : "LOW (ALERT)");

    delay(500);
}
