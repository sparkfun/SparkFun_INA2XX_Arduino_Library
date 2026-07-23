/*
  Example 05 - Custom Shunt Resistor

  This example shows how to configure the INA228 or INA237 for a user-supplied
  shunt resistor instead of the default 15 mOhm resistor on the SparkFun board.

  Select the device you're using by uncommenting the matching declaration below.

  Use case: you have soldered a 100 mOhm shunt resistor (or any other value)
  to your circuit and need to tell the device so that current and power readings
  are computed correctly.

  How calibration works:
    Call calibrate(shuntResistanceOhms, maxCurrentAmps). The library computes:
      CURRENT_LSB = maxCurrentAmps / 2^ADCBits
      SHUNT_CAL   = CalibrationScale * CURRENT_LSB * shuntResistanceOhms
    This value is written to the hardware SHUNT_CAL register. All subsequent
    current and power reads use these values automatically.

  ADC range trade-off with a 100 mOhm shunt:
    The on-chip ADC measures shunt voltage and converts it to current. The full-
    scale shunt voltage determines how much current you can measure:

      Default range (+/-163.84 mV):  max I = 163.84 mV / 100 mOhm = 1.64 A
      Reduced range (+/-40.96  mV):  max I = 40.96  mV / 100 mOhm = 0.41 A

    The reduced range offers 4x finer resolution, which is usually the reason
    you chose a higher-resistance shunt. This example uses the reduced range.
    If you need to measure up to ~1.6 A, change REDUCED_ADC_RANGE to false
    and update MAX_CURRENT_A accordingly.

  IMPORTANT: always call setADCRange() BEFORE calibrate(). The calibration
  formula depends on the range setting — calling them in the wrong order will
  produce incorrect current readings.

  SparkFun Electronics
  Date: 2026
  SparkFun code, firmware, and software is released under the MIT License.
    Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> INA228 or INA237
  QWIIC --> QWIIC

  Connect your custom shunt resistor in series with the load across the
  VIN+/VIN- terminals. Make sure VBUS is connected to the supply side.

  Serial.print it out at 115200 baud to serial monitor.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/XXXXX
*/

#include <SparkFun_INA2XX.h>

// --- User configuration ---------------------------------------------------
// Set these to match your shunt resistor and expected current range.

const float SHUNT_RESISTANCE_OHMS = 0.100f;  // 100 mOhm custom shunt

// Reduced range: max ~0.41 A, 4x finer resolution  (true)
// Default range: max ~1.64 A, wider measurement range (false)
const bool REDUCED_ADC_RANGE = true;

// Set to the maximum current your circuit will draw.
// Must not exceed the ADC range limit above or readings will clip.
const float MAX_CURRENT_A = 0.4f;
// --------------------------------------------------------------------------

// Uncomment the sensor you're using
//SfeINA228ArdI2C myINA;
SfeINA237ArdI2C myINA;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("INA2XX Example 05 - Custom Shunt Resistor");

    Wire.begin();

    // begin() initializes the I2C bus and verifies the device by checking its Device ID.
    if (!myINA.begin())
    {
        Serial.println("Power monitor not found. Did you forget to uncomment the correct IC? Please check wiring. Freezing...");
        while (1)
            delay(1000);
    }
    Serial.println("Power monitor detected!");

    // Step 1: Set the ADC range BEFORE calibrating.
    // true  = reduced range (+/-40.96 mV)  -- higher precision, lower max current
    // false = default range (+/-163.84 mV) -- lower precision, higher max current
    myINA.setADCRange(REDUCED_ADC_RANGE);

    // Step 2: Calibrate with your custom shunt resistance and expected max current.
    // The library writes the correct SHUNT_CAL register value automatically.
    if (myINA.calibrate(SHUNT_RESISTANCE_OHMS, MAX_CURRENT_A) != ksfTkErrOk)
    {
        Serial.println("Calibration failed. Freezing...");
        while (1)
            delay(1000);
    }

    // Print the configuration so you can verify it in the serial monitor.
    uint16_t shuntCal = 0;
    myINA.getShuntCal(shuntCal);

    Serial.println();
    Serial.println("Configuration:");
    Serial.print("  Shunt resistance: ");
    Serial.print(SHUNT_RESISTANCE_OHMS * 1000.0f, 1);
    Serial.println(" mOhm");
    Serial.print("  ADC range:        ");
    Serial.println(REDUCED_ADC_RANGE ? "+/-40.96 mV (reduced)" : "+/-163.84 mV (default)");
    Serial.print("  Max current:      ");
    Serial.print(MAX_CURRENT_A, 3);
    Serial.println(" A");
    Serial.print("  SHUNT_CAL reg:    0x");
    Serial.println(shuntCal, HEX);
    Serial.println();
    Serial.println("Reading measurements...");
    Serial.println();
}

void loop()
{
    float busVoltage = 0.0f, shuntVoltage_mV = 0.0f, current = 0.0f, power = 0.0f;

    sfTkError_t rc = myINA.getBusVoltage_V(busVoltage);
    if (rc == ksfTkErrOk)
        rc = myINA.getShuntVoltage_mV(shuntVoltage_mV);
    if (rc == ksfTkErrOk)
        rc = myINA.getCurrent_A(current);
    if (rc == ksfTkErrOk)
        rc = myINA.getPower_W(power);

    if (rc != ksfTkErrOk)
    {
        Serial.println("Read error. Retrying...");
        delay(500);
        return;
    }

    // Shunt voltage is printed alongside current as a sanity check:
    // current (A) should equal shuntVoltage_mV / (SHUNT_RESISTANCE_OHMS * 1000).
    Serial.print("Bus Voltage:   ");
    Serial.print(busVoltage, 4);
    Serial.println(" V");

    Serial.print("Shunt Voltage: ");
    Serial.print(shuntVoltage_mV, 4);
    Serial.println(" mV");

    Serial.print("Current:       ");
    Serial.print(current, 4);
    Serial.println(" A");

    Serial.print("Power:         ");
    Serial.print(power, 4);
    Serial.println(" W");

    Serial.println();
    delay(500);
}
