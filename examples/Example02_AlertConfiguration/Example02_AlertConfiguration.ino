/*
  Example 02 - Alert Configuration

  This example demonstrates how to configure the INA228's ALERT pin to signal
  when voltage or current thresholds are exceeded. The SparkFun Qwiic Power
  Monitor board breaks out the ALERT pin for use with an external interrupt.

  The ALERT pin is active-low open-drain by default. This example sets up:
    - Bus overvoltage threshold at 14V
    - Shunt overvoltage (overcurrent) threshold at ~5A
    - Latched alert mode (flags stay set until the DIAG_ALRT register is read)

  SparkFun Electronics
  Date: 2025
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> INA228
  QWIIC --> QWIIC
  ALERT pin --> GPIO pin 2 (with external pull-up to 3.3V)

  Serial.print it out at 115200 baud to serial monitor.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/XXXXX
*/

#include <SparkFun_INA2XX.h>

SfeINA228ArdI2C myPowerMonitor;

const int alertPin = 2;  // Connect the ALERT pin to this GPIO

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("INA228 Example 02 - Alert Configuration");

    Wire.begin();
    pinMode(alertPin, INPUT_PULLUP);

    if (!myPowerMonitor.begin())
    {
        Serial.println("INA228 not found. Please check wiring. Freezing...");
        while (1)
            delay(1000);
    }

    Serial.println("INA228 connected!");

    // Calibrate for 15 mOhm shunt, 10A max.
    myPowerMonitor.calibrate(0.015, 10.0);

    // Set up alert thresholds.

    // Bus overvoltage threshold: 14V.
    // BOVL register LSB = 3.125 mV, so 14V / 3.125 mV = 4480.
    myPowerMonitor.setBusOverVoltageThreshold(4480);

    // Shunt overvoltage threshold for ~5A overcurrent protection.
    // At 5A through a 15 mOhm shunt: V_shunt = 5 * 0.015 = 75 mV.
    // SOVL register LSB = 5 uV (ADCRANGE=0), so 75 mV / 5 uV = 15000.
    myPowerMonitor.setShuntOverVoltageThreshold(15000);

    // Enable latched alert mode (alert stays active until register is read).
    myPowerMonitor.setAlertLatch(true);

    // Set alert polarity to active-low (default, matches open-drain output).
    myPowerMonitor.setAlertPolarity(false);

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
        // This also clears the latched flags.
        uint16_t diagFlags = myPowerMonitor.getDiagnosticFlags();

        if (diagFlags & ksfINA2XXDiagBusOL)
            Serial.println("  -> Bus overvoltage detected!");

        if (diagFlags & ksfINA2XXDiagShntOL)
            Serial.println("  -> Shunt overvoltage (overcurrent) detected!");

        if (diagFlags & ksfINA2XXDiagTmpOL)
            Serial.println("  -> Temperature over-limit detected!");

        if (diagFlags & ksfINA2XXDiagMathOF)
            Serial.println("  -> Math overflow in current/power calculation!");

        Serial.println();
    }

    // Print current readings.
    Serial.print("Bus: ");
    Serial.print(myPowerMonitor.getBusVoltage_V(), 3);
    Serial.print(" V  |  Current: ");
    Serial.print(myPowerMonitor.getCurrent_A(), 4);
    Serial.print(" A  |  Alert pin: ");
    Serial.println(digitalRead(alertPin) ? "HIGH (OK)" : "LOW (ALERT)");

    delay(500);
}
