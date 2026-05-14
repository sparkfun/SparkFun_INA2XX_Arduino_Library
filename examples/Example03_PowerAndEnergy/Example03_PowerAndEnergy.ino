/*
  Example 03 - Power and Energy (INA228 Only)

  This example demonstrates the INA228's built-in energy and charge
  accumulation registers. These are 40-bit hardware accumulators that
  automatically sum energy (Joules) and charge (Coulombs) over time.

  This feature is unique to the INA228 -- the INA237 does not have
  energy or charge accumulation.

  The example reads energy and charge continuously, and demonstrates
  how to reset the accumulators when desired.

  SparkFun Electronics
  Date: 2025
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> INA228
  QWIIC --> QWIIC

  Connect your load across the screw terminals.

  Serial.print it out at 115200 baud to serial monitor.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/XXXXX
*/

#include <SparkFun_INA2XX.h>

SfeINA228ArdI2C myPowerMonitor;

unsigned long lastResetTime = 0;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("INA228 Example 03 - Power and Energy");

    Wire.begin();

    if (!myPowerMonitor.begin())
    {
        Serial.println("INA228 not found. Please check wiring. Freezing...");
        while (1)
            delay(1000);
    }

    Serial.println("INA228 connected!");

    // Calibrate for 15 mOhm shunt, 10A max.
    myPowerMonitor.calibrate(0.015, 10.0);

    // Clear the energy and charge accumulators to start fresh.
    myPowerMonitor.resetAccumulators();
    lastResetTime = millis();

    Serial.println("Accumulators reset. Measuring energy and charge...");
    Serial.println();
}

void loop()
{
    float power = myPowerMonitor.getPower_W();
    double energy = myPowerMonitor.getEnergy_J();
    double charge = myPowerMonitor.getCharge_C();
    float elapsedSec = (millis() - lastResetTime) / 1000.0f;

    Serial.print("Power:   ");
    Serial.print(power, 4);
    Serial.println(" W");

    Serial.print("Energy:  ");
    Serial.print(energy, 6);
    Serial.print(" J  (");
    Serial.print(energy / 3600.0, 6);
    Serial.println(" Wh)");

    Serial.print("Charge:  ");
    Serial.print(charge, 6);
    Serial.print(" C  (");
    Serial.print(charge / 3.6, 6);
    Serial.println(" mAh)");

    Serial.print("Elapsed: ");
    Serial.print(elapsedSec, 1);
    Serial.println(" s");

    // Check for accumulator overflow.
    if (myPowerMonitor.isEnergyOverflow())
        Serial.println("  WARNING: Energy register overflow!");

    if (myPowerMonitor.isChargeOverflow())
        Serial.println("  WARNING: Charge register overflow!");

    Serial.println();

    // Optionally reset accumulators every 60 seconds to prevent overflow.
    if (elapsedSec > 60.0f)
    {
        Serial.println("--- Resetting accumulators ---");
        myPowerMonitor.resetAccumulators();
        lastResetTime = millis();
        Serial.println();
    }

    delay(1000);
}
