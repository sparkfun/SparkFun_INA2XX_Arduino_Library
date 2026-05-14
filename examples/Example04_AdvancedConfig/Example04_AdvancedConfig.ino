/*
  Example 04 - Advanced Configuration

  This example demonstrates advanced ADC configuration options for the INA228:
    - Changing the operating mode (continuous vs. triggered)
    - Setting conversion times for bus, shunt, and temperature channels
    - Configuring ADC averaging for noise reduction
    - Using shunt temperature compensation with a tempco value
    - Selecting the reduced ADC range for higher precision at low currents

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

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("INA228 Example 04 - Advanced Configuration");

    Wire.begin();

    if (!myPowerMonitor.begin())
    {
        Serial.println("INA228 not found. Please check wiring. Freezing...");
        while (1)
            delay(1000);
    }

    Serial.println("INA228 connected!");

    // --- ADC Range ---
    // Use the reduced range (+/-40.96mV) for higher precision at low currents.
    // This gives 4x better resolution but limits the max measurable shunt voltage.
    // With a 15 mOhm shunt: max current = 40.96 mV / 15 mOhm = 2.73 A.
    // Use the default range (+/-163.84mV) for higher currents (up to ~10.9A).
    myPowerMonitor.setADCRange(false);  // false = default +/-163.84mV range

    // Calibrate for 15 mOhm shunt, 10A max.
    // NOTE: calibrate() reads the current ADCRANGE setting internally,
    // so always call setADCRange() BEFORE calibrate().
    myPowerMonitor.calibrate(0.015, 10.0);

    // --- Conversion Times ---
    // Longer conversion times improve measurement accuracy at the cost of speed.
    // Default is 1052us for all channels.
    myPowerMonitor.setBusVoltageConvTime(INA2XX_CONV_1052US);
    myPowerMonitor.setShuntVoltageConvTime(INA2XX_CONV_1052US);
    myPowerMonitor.setTempConvTime(INA2XX_CONV_540US);  // Temp can be faster

    // --- Averaging ---
    // ADC averaging reduces noise by averaging multiple samples internally.
    // The output register updates only after all samples are collected.
    // 16 samples at 1052us each = ~17ms per update for bus+shunt.
    myPowerMonitor.setAveragingCount(INA2XX_AVG_16);

    // --- Operating Mode ---
    // Continuous mode: the INA228 continuously converts all enabled channels.
    // Triggered mode: a single conversion is performed, then the device goes idle.
    myPowerMonitor.setADCMode(INA2XX_MODE_CONT_ALL);  // Continuous bus + shunt + temp

    // --- Temperature Compensation ---
    // If you know the temperature coefficient of your shunt resistor, you can
    // enable automatic compensation. The INA228 adjusts the calibration based on
    // its die temperature measurement.
    // Typical copper-trace shunt tempco: ~3930 ppm/deg-C.
    // Typical precision resistor tempco: 10-50 ppm/deg-C.
    // Set to 0 to disable (or just don't enable it).
    // myPowerMonitor.setShuntTempCoefficient(15);   // e.g., 15 ppm/deg-C
    // myPowerMonitor.enableTempCompensation(true);

    // Print the configuration.
    Serial.println();
    Serial.println("Configuration:");
    Serial.print("  ADC Range:     ");
    Serial.println(myPowerMonitor.getADCRange() ? "+/-40.96 mV" : "+/-163.84 mV");
    Serial.print("  ADC Mode:      0x");
    Serial.println(myPowerMonitor.getADCMode(), HEX);
    Serial.print("  Bus Conv Time: ");
    Serial.println(myPowerMonitor.getBusVoltageConvTime());
    Serial.print("  Shunt Conv:    ");
    Serial.println(myPowerMonitor.getShuntVoltageConvTime());
    Serial.print("  Temp Conv:     ");
    Serial.println(myPowerMonitor.getTempConvTime());
    Serial.print("  Averaging:     ");
    Serial.println(myPowerMonitor.getAveragingCount());
    Serial.print("  Temp Comp:     ");
    Serial.println(myPowerMonitor.getTempCompensation() ? "Enabled" : "Disabled");
    Serial.print("  SHUNT_CAL:     0x");
    Serial.println(myPowerMonitor.getShuntCal(), HEX);
    Serial.println();
}

void loop()
{
    // Wait for conversion to complete before reading.
    if (myPowerMonitor.isConversionReady())
    {
        float busVoltage = myPowerMonitor.getBusVoltage_V();
        float shuntVoltage = myPowerMonitor.getShuntVoltage_mV();
        float current = myPowerMonitor.getCurrent_A();
        float power = myPowerMonitor.getPower_W();
        float temperature = myPowerMonitor.getDieTemp_C();

        Serial.print("Bus: ");
        Serial.print(busVoltage, 4);
        Serial.print(" V | Shunt: ");
        Serial.print(shuntVoltage, 4);
        Serial.print(" mV | I: ");
        Serial.print(current, 4);
        Serial.print(" A | P: ");
        Serial.print(power, 4);
        Serial.print(" W | T: ");
        Serial.print(temperature, 1);
        Serial.println(" C");
    }

    delay(100);
}
