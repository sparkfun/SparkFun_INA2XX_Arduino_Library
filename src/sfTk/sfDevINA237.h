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
 * The measurement bodies themselves live once in sfDevINA2XX as individually templated/parameterized
 * helpers; the methods below are thin forwarders that pass the INA237 types and constants. Like the
 * base class, every method returns a SparkFun Toolkit error code; measured values are returned
 * through reference (output) parameters.
 *
 * @author SparkFun Electronics
 * @date June 2026
 * @copyright Copyright (c) 2026, SparkFun Electronics Inc. This project is released under the MIT License.
 *
 * SPDX-License-Identifier: MIT
 *
 * @see https://github.com/sparkfun/SparkFun_INA2XX_Arduino_Library
 */

#pragma once

#include "sfDevINA2XX.h"

/// @brief INA237 power monitor driver (also covers the register-compatible INA238).
///
/// @details Inherits shared configuration from sfDevINA2XX and forwards each measurement to the
/// matching shared helper (templated on the INA237's 16-bit register width). Unlike the INA228,
/// the INA237 has no energy or charge accumulation registers.
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
    /// @param shuntResOhms Shunt resistance in Ohms (e.g., 0.015 for 15 mOhm).
    /// @param maxCurrentA Maximum expected current in Amps.
    /// @return ::ksfTkErrOk on success, ::ksfTkErrFail for invalid arguments, or an error code on
    /// communication failure.
    sfTkError_t calibrate(float shuntResOhms, float maxCurrentA)
    {
        return calibrateImpl(shuntResOhms, maxCurrentA, kCalScale, kCurrentFullScale);
    }

    // ========================= Measurements (Engineering Units) ==============

    /// @brief Read the shunt voltage in millivolts (16-bit VSHUNT).
    /// @details Scales by 5 uV/LSB (ADCRANGE=0) or 1.25 uV/LSB (ADCRANGE=1).
    /// @param milliVolts Output reference that receives the shunt voltage in mV.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getShuntVoltage_mV(float &milliVolts)
    {
        return shuntVoltageToMilliVolts<int16_t, false>(milliVolts, kShuntLSBDefault, kShuntLSBReduced);
    }

    /// @brief Read the bus voltage in Volts (16-bit VBUS, 3.125 mV/LSB).
    /// @param volts Output reference that receives the bus voltage in V.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getBusVoltage_V(float &volts)
    {
        return busVoltageToVolts<uint16_t, false>(volts, kBusLSB);
    }

    /// @brief Read the calculated current in Amps (16-bit CURRENT, scaled by CURRENT_LSB).
    /// @note calibrate() must be called before this method returns meaningful values.
    /// @param amps Output reference that receives the current in A.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getCurrent_A(float &amps)
    {
        return currentToAmps<int16_t, false>(amps);
    }

    /// @brief Read the calculated power in Watts (24-bit POWER, scaled by 0.2 * CURRENT_LSB).
    /// @note calibrate() must be called before this method returns meaningful values.
    /// @param watts Output reference that receives the power in W.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getPower_W(float &watts)
    {
        return powerToWatts(watts, kPowerLSBScale);
    }

    /// @brief Read the die temperature in degrees Celsius.
    /// @details DIETEMP data is a 12-bit two's complement value in bits [15:4]; scales by
    /// 125 m-deg-C/LSB.
    /// @param celsius Output reference that receives the temperature in deg-C.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getDieTemp_C(float &celsius)
    {
        return dieTemperatureToCelsius(celsius, kDieTempShift, kTempLSB);
    }

    // ========================= Raw Register Access ===========================

    /// @brief Read the raw 16-bit shunt voltage value.
    /// @param value Output reference that receives the signed 16-bit value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getShuntVoltageRaw(int16_t &value)
    {
        return readSignedMeasurementRaw<int16_t, false>(kRegVShunt, value);
    }

    /// @brief Read the raw 16-bit bus voltage value.
    /// @param value Output reference that receives the unsigned 16-bit value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getBusVoltageRaw(uint16_t &value)
    {
        return readUnsignedMeasurementRaw<uint16_t, false>(kRegVBus, value);
    }

    /// @brief Read the raw 16-bit current value.
    /// @param value Output reference that receives the signed 16-bit value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getCurrentRaw(int16_t &value)
    {
        return readSignedMeasurementRaw<int16_t, false>(kRegCurrent, value);
    }

    /// @brief Read the raw 24-bit power value.
    /// @param value Output reference that receives the unsigned 24-bit value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getPowerRaw(uint32_t &value)
    {
        return readPowerRaw(value);
    }

  protected:
    // --- INA237 scaling and calibration constants ---
    static constexpr float kCalScale = 819.2e6f;         ///< Calibration scale constant.
    static constexpr float kShuntLSBDefault = 5.0e-6f;   ///< Shunt LSB in volts (ADCRANGE = 0).
    static constexpr float kShuntLSBReduced = 1.25e-6f;  ///< Shunt LSB in volts (ADCRANGE = 1).
    static constexpr float kBusLSB = 3.125e-3f;          ///< Bus voltage LSB in volts.
    static constexpr float kTempLSB = 125.0e-3f;         ///< Die temperature LSB in deg-C.
    static constexpr float kCurrentFullScale = 32768.0f; ///< 2^15, positive half of the 16-bit ADC.
    static constexpr float kPowerLSBScale = 0.2f;        ///< Power LSB = 0.2 x CURRENT_LSB.
    static constexpr uint8_t kDieTempShift = 4;          ///< DIETEMP data is a 12-bit value in [15:4].
};
