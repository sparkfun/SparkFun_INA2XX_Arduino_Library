/*
  Example 01 - Basic Readings

  This example shows how to set up the INA228 power monitor with the SparkFun
  Qwiic Power Monitor board and read current, bus voltage, power, and die
  temperature.

  The board features a 15 mOhm shunt resistor and a three-pin screw terminal
  for +/- Vin and VBUS, allowing both current and power measurements.

  This example calibrates for a maximum current of 10A, which is suitable for
  most use cases with the 15 mOhm shunt resistor.

  SparkFun Electronics
  Date: 2025
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> INA228
  QWIIC --> QWIIC

  Connect your load across the screw terminals:
    VIN+ (high side) and VIN- (low side) with VBUS to your supply.

  Serial.print it out at 115200 baud to serial monitor.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/XXXXX
*/

#include <SparkFun_INA2XX.h>

SfeINA228ArdI2C myPowerMonitor;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("INA228 Example 01 - Basic Readings");

    Wire.begin();

    // Initialize the INA228 at default I2C address (0x45).
    if (!myPowerMonitor.begin())
    {
        Serial.println("INA228 not found. Please check wiring. Freezing...");
        while (1)
            delay(1000);
    }

    Serial.println("INA228 connected!");

    // Calibrate for the 15 mOhm shunt resistor on the board, 10A max current.
    if (!myPowerMonitor.calibrate(0.015, 10.0))
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
    float busVoltage = myPowerMonitor.getBusVoltage_V();
    float current = myPowerMonitor.getCurrent_A();
    float power = myPowerMonitor.getPower_W();
    float temperature = myPowerMonitor.getDieTemp_C();

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
