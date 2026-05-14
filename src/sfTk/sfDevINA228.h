/**
 * @file sfDevINA228.h
 * @brief Header file for the SparkFun INA228 high-precision power monitor driver.
 *
 * @details
 * sfDevINA228 extends sfDevINA2XX with INA228-specific measurement capabilities:
 *   - 20-bit ADC with 24-bit measurement registers (VSHUNT, VBUS, CURRENT)
 *   - 24-bit POWER register
 *   - 40-bit ENERGY and CHARGE accumulation registers
 *   - High-precision LSB scaling (312.5 nV shunt, 195.3125 uV bus, 7.8125 m-deg-C temp)
 *   - Calibration using scale constant 13107.2 x 10^6
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

/// @brief INA228 high-precision power/energy/charge monitor driver.
///
/// @details Inherits shared configuration from sfDevINA2XX and adds 20-bit ADC measurement
/// reads with engineering-unit conversion, 40-bit energy/charge accumulation, and the
/// INA228-specific calibration formula.
class sfDevINA228 : public sfDevINA2XX
{
  public:
    sfDevINA228() : sfDevINA2XX()
    {
    }

    // ========================= Calibration ==================================

    /// @brief Calibrate the device for current and power measurements.
    /// @details Computes CURRENT_LSB from maxCurrent and the 20-bit ADC range (2^19),
    /// then calculates SHUNT_CAL = 13107.2e6 * CURRENT_LSB * Rshunt (x4 if ADCRANGE=1).
    /// Stores _currentLSB and _shuntRes internally for engineering-unit conversions.
    /// @param shuntResOhms Shunt resistance in Ohms (e.g., 0.015 for 15 mOhm).
    /// @param maxCurrent_A Maximum expected current in Amps.
    /// @return True on success, false on error.
    bool calibrate(float shuntResOhms, float maxCurrent_A);

    // ========================= Measurements (Engineering Units) ==============

    /// @brief Read the shunt voltage in millivolts.
    /// @details Reads the 24-bit VSHUNT register, extracts the 20-bit value from bits [23:4],
    /// and scales by 312.5 nV/LSB (ADCRANGE=0) or 78.125 nV/LSB (ADCRANGE=1).
    /// @return Shunt voltage in mV, or 0.0 on error.
    float getShuntVoltage_mV(void);

    /// @brief Read the bus voltage in Volts.
    /// @details Reads the 24-bit VBUS register, extracts the 20-bit value from bits [23:4],
    /// and scales by 195.3125 uV/LSB.
    /// @return Bus voltage in V, or 0.0 on error.
    float getBusVoltage_V(void);

    /// @brief Read the calculated current in Amps.
    /// @details Reads the 24-bit CURRENT register, extracts the 20-bit two's complement
    /// value from bits [23:4], and scales by CURRENT_LSB (set during calibrate()).
    /// @note calibrate() must be called before this method returns meaningful values.
    /// @return Current in A, or 0.0 on error.
    float getCurrent_A(void);

    /// @brief Read the calculated power in Watts.
    /// @details Reads the 24-bit POWER register (unsigned) and scales by
    /// 3.2 * CURRENT_LSB (set during calibrate()).
    /// @note calibrate() must be called before this method returns meaningful values.
    /// @return Power in W, or 0.0 on error.
    float getPower_W(void);

    /// @brief Read the die temperature in degrees Celsius.
    /// @details Reads the 16-bit DIETEMP register (two's complement) and scales by
    /// 7.8125 m-deg-C/LSB.
    /// @return Temperature in deg-C, or 0.0 on error.
    float getDieTemp_C(void);

    // ========================= Energy & Charge (INA228 Only) ================

    /// @brief Read the accumulated energy in Joules.
    /// @details Reads the 40-bit ENERGY register (unsigned) and scales by
    /// 16 * 3.2 * CURRENT_LSB.
    /// @note calibrate() must be called before this method returns meaningful values.
    /// @return Energy in Joules, or 0.0 on error.
    double getEnergy_J(void);

    /// @brief Read the accumulated charge in Coulombs.
    /// @details Reads the 40-bit CHARGE register (two's complement) and scales by
    /// CURRENT_LSB.
    /// @note calibrate() must be called before this method returns meaningful values.
    /// @return Charge in Coulombs, or 0.0 on error.
    double getCharge_C(void);

    // ========================= Raw Register Access ===========================

    /// @brief Read the raw 20-bit shunt voltage value.
    /// @return Signed 20-bit value (sign-extended to int32_t), or 0 on error.
    int32_t getShuntVoltageRaw(void);

    /// @brief Read the raw 20-bit bus voltage value.
    /// @return Unsigned 20-bit value, or 0 on error.
    uint32_t getBusVoltageRaw(void);

    /// @brief Read the raw 20-bit current value.
    /// @return Signed 20-bit value (sign-extended to int32_t), or 0 on error.
    int32_t getCurrentRaw(void);

    /// @brief Read the raw 24-bit power value.
    /// @return Unsigned 24-bit value, or 0 on error.
    uint32_t getPowerRaw(void);
};
