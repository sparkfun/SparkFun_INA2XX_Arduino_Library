/*
  Example 01 - Basic Readings

  This example shows how to set up the INA228 or INA237 power monitor with the
  SparkFun Qwiic Power Monitor board and read current, bus voltage, power, and
  die temperature.

  The sketch auto-detects which device is connected by checking the Device ID
  register. Both the INA228 and INA237 share the same default I2C address (0x40),
  so you can use either board without changing code.

  The board features a 15 mOhm shunt resistor and a three-pin screw terminal
  for +/- Vin and VBUS, allowing both current and power measurements.

  This example calibrates for a maximum current of 10A, which is suitable for
  most use cases with the 15 mOhm shunt resistor.

  SparkFun Electronics
  Date: 2025
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> INA228 or INA237
  QWIIC --> QWIIC

  Connect your load across the screw terminals:
    VIN+ (high side) and VIN- (low side) with VBUS to your supply.

  Serial.print it out at 115200 baud to serial monitor.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/XXXXX
*/

#include <SparkFun_INA2XX.h>

// Create both device objects — only one will be used.
SfeINA228ArdI2C myINA228;
SfeINA237ArdI2C myINA237;

// Pointer to the base class so we can call shared methods on whichever is found.
sfDevINA2XX *myPowerMonitor = nullptr;

// Track which device was detected.
bool isINA228 = false;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("INA2XX Example 01 - Basic Readings (Auto-Detect)");

    Wire.begin();

    // Try INA228 first (checks Device ID for 0x228x).
    if (myINA228.begin())
    {
        Serial.println("INA228 detected!");
        myPowerMonitor = &myINA228;
        isINA228 = true;
    }
    // If that fails, try INA237 (checks Device ID for 0x237x).
    else if (myINA237.begin())
    {
        Serial.println("INA237 detected!");
        myPowerMonitor = &myINA237;
        isINA228 = false;
    }
    else
    {
        Serial.println("No INA228 or INA237 found. Please check wiring. Freezing...");
        while (1)
            delay(1000);
    }

    // Calibrate for the 15 mOhm shunt resistor on the board, 10A max current.
    // Both devices use the same calibrate() interface, which returns a Toolkit error code.
    sfTkError_t rc = isINA228 ? myINA228.calibrate(0.015, 10.0) : myINA237.calibrate(0.015, 10.0);
    if (rc != ksfTkErrOk)
    {
        Serial.println("Calibration failed. Freezing...");
        while (1)
            delay(1000);
    }

    Serial.println("Calibrated. Reading measurements...");
    Serial.println();
}

void loop()
{
    float busVoltage = 0.0f, current = 0.0f, power = 0.0f, temperature = 0.0f;
    sfTkError_t rc;

    // Every getter returns a Toolkit error code; the reading comes back through the reference
    // argument. If any read fails, skip this cycle rather than print stale data.
    if (isINA228)
    {
        rc = myINA228.getBusVoltage_V(busVoltage);
        if (rc == ksfTkErrOk)
            rc = myINA228.getCurrent_A(current);
        if (rc == ksfTkErrOk)
            rc = myINA228.getPower_W(power);
        if (rc == ksfTkErrOk)
            rc = myINA228.getDieTemp_C(temperature);
    }
    else
    {
        rc = myINA237.getBusVoltage_V(busVoltage);
        if (rc == ksfTkErrOk)
            rc = myINA237.getCurrent_A(current);
        if (rc == ksfTkErrOk)
            rc = myINA237.getPower_W(power);
        if (rc == ksfTkErrOk)
            rc = myINA237.getDieTemp_C(temperature);
    }

    if (rc != ksfTkErrOk)
    {
        Serial.println("Read error. Retrying...");
        delay(500);
        return;
    }

    Serial.print("Bus Voltage: ");
    Serial.print(busVoltage, 4);
    Serial.println(" V");

    Serial.print("Current:     ");
    Serial.print(current, 4);
    Serial.println(" A");

    Serial.print("Power:       ");
    Serial.print(power, 4);
    Serial.println(" W");

    Serial.print("Die Temp:    ");
    Serial.print(temperature, 2);
    Serial.println(" C");

    Serial.println();
    delay(500);
}
