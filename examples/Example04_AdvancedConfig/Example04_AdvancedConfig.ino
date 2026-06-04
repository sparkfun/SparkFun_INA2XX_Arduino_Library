/*
  Example 04 - Advanced Configuration

  This example demonstrates advanced ADC configuration options:
    - Changing the operating mode (continuous vs. triggered)
    - Setting conversion times for bus, shunt, and temperature channels
    - Configuring ADC averaging for noise reduction
    - Using shunt temperature compensation with a tempco value
    - Selecting the reduced ADC range for higher precision at low currents

  The sketch auto-detects whether an INA228 or INA237 is connected.
  All ADC configuration registers are shared between both devices.

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

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("INA2XX Example 04 - Advanced Configuration (Auto-Detect)");

    Wire.begin();

    // Auto-detect which device is connected.
    if (myINA228.begin())
    {
        Serial.println("INA228 detected!");
        isINA228 = true;
    }
    else if (myINA237.begin())
    {
        Serial.println("INA237 detected!");
        isINA228 = false;
    }
    else
    {
        Serial.println("No INA228 or INA237 found. Please check wiring. Freezing...");
        while (1)
            delay(1000);
    }

    // --- ADC Range ---
    // Use the reduced range (+/-40.96mV) for higher precision at low currents.
    // This gives 4x better resolution but limits the max measurable shunt voltage.
    // With a 15 mOhm shunt: max current = 40.96 mV / 15 mOhm = 2.73 A.
    // Use the default range (+/-163.84mV) for higher currents (up to ~10.9A).
    if (isINA228)
        myINA228.setADCRange(false);  // false = default +/-163.84mV range
    else
        myINA237.setADCRange(false);

    // Calibrate for 15 mOhm shunt, 10A max.
    // NOTE: calibrate() reads the current ADCRANGE setting internally,
    // so always call setADCRange() BEFORE calibrate().
    if ((isINA228 ? myINA228.calibrate(0.015, 10.0) : myINA237.calibrate(0.015, 10.0)) != ksfTkErrOk)
    {
        Serial.println("Calibration failed. Freezing...");
        while (1)
            delay(1000);
    }

    // --- Conversion Times ---
    // Longer conversion times improve measurement accuracy at the cost of speed.
    // Default is 1052us for all channels.
    if (isINA228)
    {
        myINA228.setBusVoltageConvTime(INA2XX_CONV_1052US);
        myINA228.setShuntVoltageConvTime(INA2XX_CONV_1052US);
        myINA228.setTempConvTime(INA2XX_CONV_540US);  // Temp can be faster
    }
    else
    {
        myINA237.setBusVoltageConvTime(INA2XX_CONV_1052US);
        myINA237.setShuntVoltageConvTime(INA2XX_CONV_1052US);
        myINA237.setTempConvTime(INA2XX_CONV_540US);
    }

    // --- Averaging ---
    // ADC averaging reduces noise by averaging multiple samples internally.
    // The output register updates only after all samples are collected.
    // 16 samples at 1052us each = ~17ms per update for bus+shunt.
    if (isINA228)
        myINA228.setAveragingCount(INA2XX_AVG_16);
    else
        myINA237.setAveragingCount(INA2XX_AVG_16);

    // --- Operating Mode ---
    // Continuous mode: the device continuously converts all enabled channels.
    // Triggered mode: a single conversion is performed, then the device goes idle.
    if (isINA228)
        myINA228.setADCMode(INA2XX_MODE_CONT_ALL);  // Continuous bus + shunt + temp
    else
        myINA237.setADCMode(INA2XX_MODE_CONT_ALL);

    // --- Temperature Compensation ---
    // If you know the temperature coefficient of your shunt resistor, you can
    // enable automatic compensation. The device adjusts the calibration based on
    // its die temperature measurement.
    // Typical copper-trace shunt tempco: ~3930 ppm/deg-C.
    // Typical precision resistor tempco: 10-50 ppm/deg-C.
    // Set to 0 to disable (or just don't enable it).
    // if (isINA228)
    // {
    //     myINA228.setShuntTempCoefficient(15);   // e.g., 15 ppm/deg-C
    //     myINA228.enableTempCompensation(true);
    // }
    // else
    // {
    //     myINA237.setShuntTempCoefficient(15);
    //     myINA237.enableTempCompensation(true);
    // }

    // Print the configuration. Pointer to the base class so we read each setting once,
    // regardless of which device was detected. Every getter returns its value through a
    // reference argument and reports a Toolkit error code.
    sfDevINA2XX *dev = isINA228 ? (sfDevINA2XX *)&myINA228 : (sfDevINA2XX *)&myINA237;

    bool reducedRange = false;
    sfe_ina2xx_mode_t mode = INA2XX_MODE_CONT_ALL;
    sfe_ina2xx_conv_time_t busCT = INA2XX_CONV_1052US, shuntCT = INA2XX_CONV_1052US, tempCT = INA2XX_CONV_1052US;
    sfe_ina2xx_avg_count_t avg = INA2XX_AVG_1;
    bool tempComp = false;
    uint16_t shuntCal = 0;

    dev->getADCRange(reducedRange);
    dev->getADCMode(mode);
    dev->getBusVoltageConvTime(busCT);
    dev->getShuntVoltageConvTime(shuntCT);
    dev->getTempConvTime(tempCT);
    dev->getAveragingCount(avg);
    dev->getTempCompensation(tempComp);
    dev->getShuntCal(shuntCal);

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
    sfDevINA2XX *dev = isINA228 ? (sfDevINA2XX *)&myINA228 : (sfDevINA2XX *)&myINA237;

    bool ready = false;
    dev->isConversionReady(ready);

    // Wait for conversion to complete before reading.
    if (ready)
    {
        float busVoltage = 0.0f, shuntVoltage = 0.0f, current = 0.0f, power = 0.0f, temperature = 0.0f;

        if (isINA228)
        {
            myINA228.getBusVoltage_V(busVoltage);
            myINA228.getShuntVoltage_mV(shuntVoltage);
            myINA228.getCurrent_A(current);
            myINA228.getPower_W(power);
            myINA228.getDieTemp_C(temperature);
        }
        else
        {
            myINA237.getBusVoltage_V(busVoltage);
            myINA237.getShuntVoltage_mV(shuntVoltage);
            myINA237.getCurrent_A(current);
            myINA237.getPower_W(power);
            myINA237.getDieTemp_C(temperature);
        }

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
