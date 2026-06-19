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
 * The measurement bodies themselves live once in sfDevINA2XX as individually templated/parameterized
 * helpers; the methods below are thin forwarders that pass the INA228 types and constants. Like the
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

/// @brief INA228 high-precision power/energy/charge monitor driver.
///
/// @details Inherits shared configuration from sfDevINA2XX and forwards each measurement to the
/// matching shared helper (templated on the INA228's 24-bit register width), adding the
/// INA228-only 40-bit energy and charge accumulation.
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
    /// @param shuntResOhms Shunt resistance in Ohms (e.g., 0.015 for 15 mOhm).
    /// @param maxCurrentA Maximum expected current in Amps.
    /// @return ::ksfTkErrOk on success, ::ksfTkErrFail for invalid arguments, or an error code on
    /// communication failure.
    sfTkError_t calibrate(float shuntResOhms, float maxCurrentA)
    {
        return calibrateImpl(shuntResOhms, maxCurrentA, kCalScale, kCurrentFullScale);
    }

    // ========================= Measurements (Engineering Units) ==============

    /// @brief Read the shunt voltage in millivolts (24-bit VSHUNT, bits [23:4]).
    /// @details Scales by 312.5 nV/LSB (ADCRANGE=0) or 78.125 nV/LSB (ADCRANGE=1).
    /// @param milliVolts Output reference that receives the shunt voltage in mV.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getShuntVoltage_mV(float &milliVolts)
    {
        return shuntVoltageToMilliVolts<int32_t, true>(milliVolts, kShuntLSBDefault, kShuntLSBReduced);
    }

    /// @brief Read the bus voltage in Volts (24-bit VBUS, bits [23:4], 195.3125 uV/LSB).
    /// @param volts Output reference that receives the bus voltage in V.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getBusVoltage_V(float &volts)
    {
        return busVoltageToVolts<uint32_t, true>(volts, kBusLSB);
    }

    /// @brief Read the calculated current in Amps (24-bit CURRENT, scaled by CURRENT_LSB).
    /// @note calibrate() must be called before this method returns meaningful values.
    /// @param amps Output reference that receives the current in A.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getCurrent_A(float &amps)
    {
        return currentToAmps<int32_t, true>(amps);
    }

    /// @brief Read the calculated power in Watts (24-bit POWER, scaled by 3.2 * CURRENT_LSB).
    /// @note calibrate() must be called before this method returns meaningful values.
    /// @param watts Output reference that receives the power in W.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getPower_W(float &watts)
    {
        return powerToWatts(watts, kPowerLSBScale);
    }

    /// @brief Read the die temperature in degrees Celsius (16-bit DIETEMP, 7.8125 m-deg-C/LSB).
    /// @param celsius Output reference that receives the temperature in deg-C.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getDieTemp_C(float &celsius)
    {
        return dieTemperatureToCelsius(celsius, kDieTempShift, kTempLSB);
    }

    // ========================= Raw Register Access ===========================

    /// @brief Read the raw 20-bit shunt voltage value (sign-extended).
    /// @param value Output reference that receives the signed 20-bit value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getShuntVoltageRaw(int32_t &value)
    {
        return readSignedMeasurementRaw<int32_t, true>(kRegVShunt, value);
    }

    /// @brief Read the raw 20-bit bus voltage value.
    /// @param value Output reference that receives the unsigned 20-bit value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getBusVoltageRaw(uint32_t &value)
    {
        return readUnsignedMeasurementRaw<uint32_t, true>(kRegVBus, value);
    }

    /// @brief Read the raw 20-bit current value (sign-extended).
    /// @param value Output reference that receives the signed 20-bit value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getCurrentRaw(int32_t &value)
    {
        return readSignedMeasurementRaw<int32_t, true>(kRegCurrent, value);
    }

    /// @brief Read the raw 24-bit power value.
    /// @param value Output reference that receives the unsigned 24-bit value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getPowerRaw(uint32_t &value)
    {
        return readPowerRaw(value);
    }

    // ========================= Energy & Charge (INA228 Only) ================

    /// @brief Read the accumulated energy in Joules.
    /// @details Reads the 40-bit ENERGY register (unsigned) and scales by
    /// 16 * 3.2 * CURRENT_LSB.
    /// @note calibrate() must be called before this method returns meaningful values.
    /// @param joules Output reference that receives the energy in Joules.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getEnergy_J(double &joules);

    /// @brief Read the accumulated charge in Coulombs.
    /// @details Reads the 40-bit CHARGE register (two's complement) and scales by
    /// CURRENT_LSB.
    /// @note calibrate() must be called before this method returns meaningful values.
    /// @param coulombs Output reference that receives the charge in Coulombs.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getCharge_C(double &coulombs);

  protected:
    // --- INA228 scaling and calibration constants ---
    static constexpr float kCalScale = 13107.2e6f;        ///< Calibration scale constant.
    static constexpr float kShuntLSBDefault = 312.5e-9f;  ///< Shunt LSB in volts (ADCRANGE = 0).
    static constexpr float kShuntLSBReduced = 78.125e-9f; ///< Shunt LSB in volts (ADCRANGE = 1).
    static constexpr float kBusLSB = 195.3125e-6f;        ///< Bus voltage LSB in volts.
    static constexpr float kTempLSB = 7.8125e-3f;         ///< Die temperature LSB in deg-C.
    static constexpr float kCurrentFullScale = 524288.0f; ///< 2^19, positive half of the 20-bit ADC.
    static constexpr float kPowerLSBScale = 3.2f;         ///< Power LSB = 3.2 x CURRENT_LSB.
    static constexpr uint8_t kDieTempShift = 0;           ///< DIETEMP is a full 16-bit value.
    static constexpr double kEnergyLSBScale = 16.0 * 3.2; ///< Energy LSB = 16 x 3.2 x CURRENT_LSB.
};
