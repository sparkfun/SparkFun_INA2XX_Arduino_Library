/*
  Example 01 - Basic Readings

  This example shows how to set up the INA228 or INA237 power monitor with the
  SparkFun Qwiic Power Monitor board and read current, bus voltage, power, and
  die temperature.

  Select the device you're using by uncommenting the matching declaration below.
  Both the INA228 and INA237 share the same default I2C address (0x40) and the
  same read interface, so no other code changes are needed.

  The board features a 15 mOhm shunt resistor and a three-pin screw terminal
  for +/- Vin and VBUS, allowing both current and power measurements.

  This example calibrates for a maximum current of 10A, which is suitable for
  most use cases with the 15 mOhm shunt resistor.

  SparkFun Electronics
  Date: 2026
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

// Uncomment the sensor you're using
SfeINA228ArdI2C myINA;
// SfeINA237ArdI2C myINA;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("INA2XX Example 01 - Basic Readings");

    Wire.begin();

    // begin() initializes the I2C bus and verifies the device by checking its Device ID.
    if (!myINA.begin())
    {
        Serial.println("Power monitor not found. Please check wiring. Freezing...");
        while (1)
            delay(1000);
    }
    Serial.println("Power monitor detected!");

    // Calibrate for the 15 mOhm shunt resistor on the board, 10A max current.
    myINA.calibrate(0.015, 10.0);

    Serial.println("Calibrated. Reading measurements...");
    Serial.println();
}

void loop()
{
    float busVoltage = 0.0f, current = 0.0f, power = 0.0f, temperature = 0.0f;

    // Each getter returns its reading through the reference argument.
    myINA.getBusVoltage_V(busVoltage);
    myINA.getCurrent_A(current);
    myINA.getPower_W(power);
    myINA.getDieTemp_C(temperature);

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
