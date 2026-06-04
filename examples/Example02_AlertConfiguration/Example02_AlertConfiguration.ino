/*
  Example 02 - Alert Configuration

  This example demonstrates how to configure the ALERT pin to signal
  when voltage or current thresholds are exceeded. The SparkFun Qwiic Power
  Monitor board breaks out the ALERT pin for use with an external interrupt.

  The sketch auto-detects whether an INA228 or INA237 is connected.
  Alert threshold registers and diagnostic flags are shared between both devices.

  The ALERT pin is active-low open-drain by default. This example sets up:
    - Bus overvoltage threshold at 14V
    - Shunt overvoltage (overcurrent) threshold at ~5A
    - Latched alert mode (flags stay set until the DIAG_ALRT register is read)

  SparkFun Electronics
  Date: 2025
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

SfeINA228ArdI2C myINA228;
SfeINA237ArdI2C myINA237;

bool isINA228 = false;

const int alertPin = 2;  // Connect the ALERT pin to this GPIO

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("INA2XX Example 02 - Alert Configuration (Auto-Detect)");

    Wire.begin();
    pinMode(alertPin, INPUT_PULLUP);

    // Auto-detect which device is connected.
    if (myINA228.begin())
    {
        Serial.println("INA228 detected!");
        isINA228 = true;
    }
    else if (myINA237.begin())
    {
        Serial.println("INA237 detected!");
        isINA228 = false;
    }
    else
    {
        Serial.println("No INA228 or INA237 found. Please check wiring. Freezing...");
        while (1)
            delay(1000);
    }

    // Calibrate for 15 mOhm shunt, 10A max.
    if ((isINA228 ? myINA228.calibrate(0.015, 10.0) : myINA237.calibrate(0.015, 10.0)) != ksfTkErrOk)
    {
        Serial.println("Calibration failed. Freezing...");
        while (1)
            delay(1000);
    }

    // Set up alert thresholds.
    // All alert/diagnostic registers are in the base class, so both devices
    // support the same interface. Each setter returns a Toolkit error code, which
    // we ignore here for brevity (see Example 01 for full error handling).

    // Bus overvoltage threshold: 14V.
    // BOVL register LSB = 3.125 mV, so 14V / 3.125 mV = 4480.
    if (isINA228)
    {
        myINA228.setBusOverVoltageThreshold(4480);
        myINA228.setShuntOverVoltageThreshold(15000);
        myINA228.setAlertLatch(true);
        myINA228.setAlertPolarity(false);
    }
    else
    {
        myINA237.setBusOverVoltageThreshold(4480);
        myINA237.setShuntOverVoltageThreshold(15000);
        myINA237.setAlertLatch(true);
        myINA237.setAlertPolarity(false);
    }

    Serial.println("Alert thresholds configured:");
    Serial.println("  Bus OV:   14.0 V");
    Serial.println("  Shunt OV: 75 mV (~5A)");
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
        sfTkError_t rc = isINA228 ? myINA228.getDiagnosticFlags(diagFlags)
                                  : myINA237.getDiagnosticFlags(diagFlags);

        if (rc == ksfTkErrOk)
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
    if (isINA228)
    {
        myINA228.getBusVoltage_V(busV);
        myINA228.getCurrent_A(current);
    }
    else
    {
        myINA237.getBusVoltage_V(busV);
        myINA237.getCurrent_A(current);
    }

    Serial.print("Bus: ");
    Serial.print(busV, 3);
    Serial.print(" V  |  Current: ");
    Serial.print(current, 4);
    Serial.print(" A  |  Alert pin: ");
    Serial.println(digitalRead(alertPin) ? "HIGH (OK)" : "LOW (ALERT)");

    delay(500);
}
