/**
 * @file sfDevINA237.h
 * @brief Header file for the SparkFun INA237 power monitor driver.
 *
 * @details
 * sfDevINA237 extends sfDevINA2XX with INA237-specific measurement capabilities:
 *   - 16-bit ADC with 16-bit measurement registers (VSHUNT, VBUS, CURRENT)
 *   - 24-bit POWER register
 *   - No energy or charge accumulation (registers 0x09/0x0A do not exist)
 *   - Lower-precision LSB scaling (5 uV shunt, 3.125 mV bus, 125 m-deg-C temp)
 *   - Calibration using scale constant 819.2 x 10^6
 *
 * @author SparkFun Electronics
 * @date 2025
 * @copyright Copyright (c) 2025, SparkFun Electronics Inc. This project is released under the MIT License.
 *
 * SPDX-License-Identifier: MIT
 *
 * @see https://github.com/sparkfun/SparkFun_INA2XX_Arduino_Library
 */

#pragma once

#include "sfDevINA2XX.h"

/// @brief INA237 power monitor driver.
///
/// @details Inherits shared configuration from sfDevINA2XX and adds 16-bit ADC measurement
/// reads with engineering-unit conversion and the INA237-specific calibration formula.
/// Unlike the INA228, the INA237 has no energy or charge accumulation registers.
class sfDevINA237 : public sfDevINA2XX
{
  public:
    sfDevINA237() : sfDevINA2XX()
    {
    }

    // ========================= Calibration ==================================

    /// @brief Calibrate the device for current and power measurements.
    /// @details Computes CURRENT_LSB from maxCurrent and the 16-bit ADC range (2^15),
    /// then calculates SHUNT_CAL = 819.2e6 * CURRENT_LSB * Rshunt (x4 if ADCRANGE=1).
    /// Stores _currentLSB and _shuntRes internally for engineering-unit conversions.
    /// @param shuntResOhms Shunt resistance in Ohms (e.g., 0.015 for 15 mOhm).
    /// @param maxCurrent_A Maximum expected current in Amps.
    /// @return True on success, false on error.
    bool calibrate(float shuntResOhms, float maxCurrent_A);

    // ========================= Measurements (Engineering Units) ==============

    /// @brief Read the shunt voltage in millivolts.
    /// @details Reads the 16-bit VSHUNT register (two's complement) and scales by
    /// 5 uV/LSB (ADCRANGE=0) or 1.25 uV/LSB (ADCRANGE=1).
    /// @return Shunt voltage in mV, or 0.0 on error.
    float getShuntVoltage_mV(void);

    /// @brief Read the bus voltage in Volts.
    /// @details Reads the 16-bit VBUS register (unsigned) and scales by 3.125 mV/LSB.
    /// @return Bus voltage in V, or 0.0 on error.
    float getBusVoltage_V(void);

    /// @brief Read the calculated current in Amps.
    /// @details Reads the 16-bit CURRENT register (two's complement) and scales by
    /// CURRENT_LSB (set during calibrate()).
    /// @note calibrate() must be called before this method returns meaningful values.
    /// @return Current in A, or 0.0 on error.
    float getCurrent_A(void);

    /// @brief Read the calculated power in Watts.
    /// @details Reads the 24-bit POWER register (unsigned) and scales by
    /// 0.2 * CURRENT_LSB (set during calibrate()).
    /// @note calibrate() must be called before this method returns meaningful values.
    /// @return Power in W, or 0.0 on error.
    float getPower_W(void);

    /// @brief Read the die temperature in degrees Celsius.
    /// @details Reads the 16-bit DIETEMP register; data is in bits [15:4] as a 12-bit
    /// two's complement value. Scales by 125 m-deg-C/LSB.
    /// @return Temperature in deg-C, or 0.0 on error.
    float getDieTemp_C(void);

    // ========================= Raw Register Access ===========================

    /// @brief Read the raw 16-bit shunt voltage value.
    /// @return Signed 16-bit value, or 0 on error.
    int16_t getShuntVoltageRaw(void);

    /// @brief Read the raw 16-bit bus voltage value.
    /// @return Unsigned 16-bit value, or 0 on error.
    uint16_t getBusVoltageRaw(void);

    /// @brief Read the raw 16-bit current value.
    /// @return Signed 16-bit value, or 0 on error.
    int16_t getCurrentRaw(void);

    /// @brief Read the raw 24-bit power value.
    /// @return Unsigned 24-bit value, or 0 on error.
    uint32_t getPowerRaw(void);
};
