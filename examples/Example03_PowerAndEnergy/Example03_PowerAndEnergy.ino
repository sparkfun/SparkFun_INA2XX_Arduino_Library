/*
  Example 03 - Power and Energy (INA228 Only)

  This example demonstrates the INA228's built-in energy and charge
  accumulation registers. These are 40-bit hardware accumulators that
  automatically sum energy (Joules) and charge (Coulombs) over time.

  This feature is unique to the INA228 -- the INA237 does not have
  energy or charge accumulation, so this example supports the INA228 only.

  SparkFun Electronics
  Date: 2026
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

SfeINA228ArdI2C myINA228;
//INA237 is not supported for this example.

unsigned long lastResetTime = 0;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("INA2XX Example 03 - Power and Energy (INA228 Only)");

    Wire.begin();

    // begin() initializes the I2C bus and verifies the device by checking its Device ID.
    if (!myINA228.begin())
    {
        Serial.println("INA228 not found. Please check wiring. Freezing...");
        while (1)
            delay(1000);
    }
    Serial.println("INA228 detected! Energy and charge accumulation available.");

    // Calibrate for 15 mOhm shunt, 10A max.
    if (myINA228.calibrate(0.015, 10.0) != ksfTkErrOk)
    {
        Serial.println("Calibration failed. Freezing...");
        while (1)
            delay(1000);
    }

    // Clear the energy and charge accumulators.
    myINA228.resetAccumulators();
    Serial.println("Accumulators reset. Measuring energy and charge...");

    lastResetTime = millis();
    Serial.println();
}

void loop()
{
    float power = 0.0f;
    double energy = 0.0, charge = 0.0;
    float elapsedSec = (millis() - lastResetTime) / 1000.0f;

    myINA228.getPower_W(power);
    myINA228.getEnergy_J(energy);
    myINA228.getCharge_C(charge);

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
    bool energyOverflow = false, chargeOverflow = false;
    myINA228.isEnergyOverflow(energyOverflow);
    myINA228.isChargeOverflow(chargeOverflow);

    if (energyOverflow)
        Serial.println("  WARNING: Energy register overflow!");

    if (chargeOverflow)
        Serial.println("  WARNING: Charge register overflow!");

    // Optionally reset accumulators every 60 seconds to prevent overflow.
    if (elapsedSec > 60.0f)
    {
        Serial.println("--- Resetting accumulators ---");
        myINA228.resetAccumulators();
        lastResetTime = millis();
    }

    Serial.println();
    delay(1000);
}
