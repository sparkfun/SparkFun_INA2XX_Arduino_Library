/*
  Example 03 - Power and Energy (INA228 Only)

  This example demonstrates the INA228's built-in energy and charge
  accumulation registers. These are 40-bit hardware accumulators that
  automatically sum energy (Joules) and charge (Coulombs) over time.

  This feature is unique to the INA228 -- the INA237 does not have
  energy or charge accumulation. If an INA237 is detected, this example
  will fall back to reading power only (no energy/charge).

  SparkFun Electronics
  Date: 2025
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> INA228 or INA237
  QWIIC --> QWIIC

  Connect your load across the screw terminals.

  Serial.print it out at 115200 baud to serial monitor.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/XXXXX
*/

#include <SparkFun_INA2XX.h>

SfeINA228ArdI2C myINA228;
SfeINA237ArdI2C myINA237;

bool isINA228 = false;

unsigned long lastResetTime = 0;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("INA2XX Example 03 - Power and Energy (Auto-Detect)");

    Wire.begin();

    // Auto-detect which device is connected.
    if (myINA228.begin())
    {
        Serial.println("INA228 detected! Energy and charge accumulation available.");
        isINA228 = true;
    }
    else if (myINA237.begin())
    {
        Serial.println("INA237 detected. Energy/charge not available — power readings only.");
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

    // Clear the energy and charge accumulators (INA228 only).
    if (isINA228)
    {
        myINA228.resetAccumulators();
        Serial.println("Accumulators reset. Measuring energy and charge...");
    }
    else
    {
        Serial.println("Measuring power (no accumulation on INA237)...");
    }

    lastResetTime = millis();
    Serial.println();
}

void loop()
{
    float power = 0.0f;
    float elapsedSec = (millis() - lastResetTime) / 1000.0f;

    if (isINA228)
    {
        double energy = 0.0, charge = 0.0;
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
    }
    else
    {
        // INA237: power readings only, no energy/charge accumulation.
        float current = 0.0f, busV = 0.0f;
        myINA237.getPower_W(power);
        myINA237.getCurrent_A(current);
        myINA237.getBusVoltage_V(busV);

        Serial.print("Bus: ");
        Serial.print(busV, 4);
        Serial.print(" V  |  Current: ");
        Serial.print(current, 4);
        Serial.print(" A  |  Power: ");
        Serial.print(power, 4);
        Serial.println(" W");
    }

    Serial.println();
    delay(1000);
}
