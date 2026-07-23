/*
  Example 04 - Advanced Configuration

  This example demonstrates advanced ADC configuration options:
    - Changing the operating mode (continuous vs. triggered)
    - Setting conversion times for bus, shunt, and temperature channels
    - Configuring ADC averaging for noise reduction
    - Using shunt temperature compensation with a tempco value
    - Selecting the reduced ADC range for higher precision at low currents

  Select the device you're using by uncommenting the matching declaration below.
  All ADC configuration registers are shared between both devices.

  SparkFun Electronics
  Date: 2026
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

// Uncomment the sensor you're using
//SfeINA228ArdI2C myINA;
SfeINA237ArdI2C myINA;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("INA2XX Example 04 - Advanced Configuration");

    Wire.begin();

    // begin() initializes the I2C bus and verifies the device by checking its Device ID.
    if (!myINA.begin())
    {
        Serial.println("Power monitor not found. Please check wiring. Freezing...");
        while (1)
            delay(1000);
    }
    Serial.println("Power monitor detected!");

    // --- ADC Range ---
    // Use the reduced range (+/-40.96mV) for higher precision at low currents.
    // This gives 4x better resolution but limits the max measurable shunt voltage.
    // With a 15 mOhm shunt: max current = 40.96 mV / 15 mOhm = 2.73 A.
    // Use the default range (+/-163.84mV) for higher currents (up to ~10.9A).
    myINA.setADCRange(false);  // false = default +/-163.84mV range

    // Calibrate for 15 mOhm shunt, 10A max.
    // NOTE: calibrate() reads the current ADCRANGE setting internally,
    // so always call setADCRange() BEFORE calibrate().
    if (myINA.calibrate(0.015, 10.0) != ksfTkErrOk)
    {
        Serial.println("Calibration failed. Freezing...");
        while (1)
            delay(1000);
    }

    // --- Conversion Times ---
    // Longer conversion times improve measurement accuracy at the cost of speed.
    // Default is 1052us for all channels.
    myINA.setBusVoltageConvTime(INA2XX_CONV_1052US);
    myINA.setShuntVoltageConvTime(INA2XX_CONV_1052US);
    myINA.setTempConvTime(INA2XX_CONV_540US);  // Temp can be faster

    // --- Averaging ---
    // ADC averaging reduces noise by averaging multiple samples internally.
    // The output register updates only after all samples are collected.
    // 16 samples at 1052us each = ~17ms per update for bus+shunt.
    myINA.setAveragingCount(INA2XX_AVG_16);

    // --- Operating Mode ---
    // Continuous mode: the device continuously converts all enabled channels.
    // Triggered mode: a single conversion is performed, then the device goes idle.
    myINA.setADCMode(INA2XX_MODE_CONT_ALL);  // Continuous bus + shunt + temp

    // --- Temperature Compensation ---
    // If you know the temperature coefficient of your shunt resistor, you can
    // enable automatic compensation. The device adjusts the calibration based on
    // its die temperature measurement.
    // Typical copper-trace shunt tempco: ~3930 ppm/deg-C.
    // Typical precision resistor tempco: 10-50 ppm/deg-C.
    // Set to 0 to disable (or just don't enable it).
    // myINA.setShuntTempCoefficient(15);   // e.g., 15 ppm/deg-C
    // myINA.enableTempCompensation(true);

    // Print the configuration. Every getter returns its value through a
    // reference argument and reports a Toolkit error code.
    bool reducedRange;
    sfe_ina2xx_mode_t mode;
    sfe_ina2xx_conv_time_t busCT;
    sfe_ina2xx_conv_time_t shuntCT;
    sfe_ina2xx_conv_time_t tempCT;
    sfe_ina2xx_avg_count_t avg;
    bool tempComp;
    uint16_t shuntCal;

    myINA.getADCRange(reducedRange);
    myINA.getADCMode(mode);
    myINA.getBusVoltageConvTime(busCT);
    myINA.getShuntVoltageConvTime(shuntCT);
    myINA.getTempConvTime(tempCT);
    myINA.getAveragingCount(avg);
    myINA.getTempCompensation(tempComp);
    myINA.getShuntCal(shuntCal);

    Serial.println();
    Serial.println("Configuration:");
    Serial.print("  ADC Range:     ");
    Serial.println(reducedRange ? "+/-40.96 mV" : "+/-163.84 mV");
    Serial.print("  ADC Mode:      0x");
    Serial.println((uint8_t)mode, HEX);
    Serial.print("  Bus Conv Time: ");
    Serial.println((uint8_t)busCT);
    Serial.print("  Shunt Conv:    ");
    Serial.println((uint8_t)shuntCT);
    Serial.print("  Temp Conv:     ");
    Serial.println((uint8_t)tempCT);
    Serial.print("  Averaging:     ");
    Serial.println((uint8_t)avg);
    Serial.print("  Temp Comp:     ");
    Serial.println(tempComp ? "Enabled" : "Disabled");
    Serial.print("  SHUNT_CAL:     0x");
    Serial.println(shuntCal, HEX);

    Serial.println();
}

void loop()
{
    bool ready = false;
    myINA.isConversionReady(ready);

    // Wait for conversion to complete before reading.
    if (ready)
    {
        float busVoltage = 0.0f, shuntVoltage = 0.0f, current = 0.0f, power = 0.0f, temperature = 0.0f;

        myINA.getBusVoltage_V(busVoltage);
        myINA.getShuntVoltage_mV(shuntVoltage);
        myINA.getCurrent_A(current);
        myINA.getPower_W(power);
        myINA.getDieTemp_C(temperature);

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
