/**
 * @file sfDevINA228.cpp
 * @brief Implementation file for the SparkFun INA228 high-precision power monitor driver.
 *
 * @details
 * This file implements INA228-specific measurement methods including 24-bit register reads,
 * 40-bit energy/charge reads, and the INA228 calibration formula.
 *
 * INA228 ADC Details:
 *   - 20-bit delta-sigma ADC
 *   - VSHUNT/VBUS/CURRENT registers are 24 bits wide; data is in bits [23:4], lower 4 reserved
 *   - POWER register is 24 bits wide, all bits used (unsigned)
 *   - ENERGY register is 40 bits wide (unsigned)
 *   - CHARGE register is 40 bits wide (two's complement)
 *   - Shunt LSB: 312.5 nV (ADCRANGE=0) or 78.125 nV (ADCRANGE=1)
 *   - Bus LSB: 195.3125 uV
 *   - Temp LSB: 7.8125 m-deg-C
 *   - Calibration scale: 13107.2 x 10^6 (x4 for ADCRANGE=1)
 *
 * @author SparkFun Electronics
 * @date 2025
 * @copyright Copyright (c) 2025, SparkFun Electronics Inc. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * @see https://github.com/sparkfun/SparkFun_INA2XX_Arduino_Library
 */

#include "sfDevINA228.h"

// ========================= Calibration ======================================

sfTkError_t sfDevINA228::calibrate(float shuntResOhms, float maxCurrentA)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    if (shuntResOhms <= 0.0f || maxCurrentA <= 0.0f)
        return ksfTkErrFail;

    _shuntRes = shuntResOhms;

    // CURRENT_LSB = MaxCurrent / 2^19  (20-bit ADC, positive half of range)
    _currentLSB = maxCurrentA / kCurrentFullScale;

    // SHUNT_CAL = 13107.2 x 10^6 x CURRENT_LSB x Rshunt
    float calValue = kCalScale * _currentLSB * _shuntRes;

    // If ADCRANGE = 1, multiply by 4. Refresh the cached range from the device first.
    sfTkError_t rc = getADCRange(_adcRange);
    if (rc != ksfTkErrOk)
        return rc;

    if (_adcRange)
        calValue *= 4.0f;

    // Clamp to 15-bit range.
    uint16_t calReg = (uint16_t)calValue;
    if (calReg > 0x7FFF)
        calReg = 0x7FFF;

    return setShuntCal(calReg);
}

// ========================= Measurements (Engineering Units) =================

sfTkError_t sfDevINA228::getShuntVoltage_mV(float &milliVolts)
{
    int32_t raw = 0;
    sfTkError_t rc = getShuntVoltageRaw(raw);
    if (rc != ksfTkErrOk)
        return rc;

    // Select LSB based on ADC range, then convert to millivolts (LSB is in volts).
    float lsb = _adcRange ? kShuntLSBReduced : kShuntLSBDefault;
    milliVolts = (float)raw * lsb * 1000.0f;
    return ksfTkErrOk;
}

sfTkError_t sfDevINA228::getBusVoltage_V(float &volts)
{
    uint32_t raw = 0;
    sfTkError_t rc = getBusVoltageRaw(raw);
    if (rc != ksfTkErrOk)
        return rc;

    volts = (float)raw * kBusLSB;
    return ksfTkErrOk;
}

sfTkError_t sfDevINA228::getCurrent_A(float &amps)
{
    int32_t raw = 0;
    sfTkError_t rc = getCurrentRaw(raw);
    if (rc != ksfTkErrOk)
        return rc;

    amps = (float)raw * _currentLSB;
    return ksfTkErrOk;
}

sfTkError_t sfDevINA228::getPower_W(float &watts)
{
    uint32_t raw = 0;
    sfTkError_t rc = getPowerRaw(raw);
    if (rc != ksfTkErrOk)
        return rc;

    // Power LSB = 3.2 x CURRENT_LSB.
    watts = (float)raw * kPowerLSBScale * _currentLSB;
    return ksfTkErrOk;
}

sfTkError_t sfDevINA228::getDieTemp_C(float &celsius)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    uint16_t raw = 0;
    sfTkError_t rc = _theBus->readRegister(kRegDieTemp, raw);
    if (rc != ksfTkErrOk)
        return rc;

    // DIETEMP is a 16-bit two's complement value.
    celsius = (float)((int16_t)raw) * kTempLSB;
    return ksfTkErrOk;
}

// ========================= Energy & Charge ==================================

sfTkError_t sfDevINA228::getEnergy_J(double &joules)
{
    uint64_t raw = 0;
    sfTkError_t rc = readRegister40(kRegEnergy, raw);
    if (rc != ksfTkErrOk)
        return rc;

    // Energy LSB = 16 x 3.2 x CURRENT_LSB.
    joules = (double)raw * kEnergyLSBScale * (double)_currentLSB;
    return ksfTkErrOk;
}

sfTkError_t sfDevINA228::getCharge_C(double &coulombs)
{
    uint64_t raw = 0;
    sfTkError_t rc = readRegister40(kRegCharge, raw);
    if (rc != ksfTkErrOk)
        return rc;

    // CHARGE is a 40-bit two's complement value.
    int64_t signedRaw = (int64_t)raw;
    // Sign-extend from 40 bits.
    if (signedRaw & ((int64_t)1 << 39))
        signedRaw |= ~(((int64_t)1 << 40) - 1);

    // Charge LSB = CURRENT_LSB.
    coulombs = (double)signedRaw * (double)_currentLSB;
    return ksfTkErrOk;
}

// ========================= Raw Register Access ==============================

sfTkError_t sfDevINA228::getShuntVoltageRaw(int32_t &value)
{
    uint32_t raw = 0;
    sfTkError_t rc = readRegister24(kRegVShunt, raw);
    if (rc != ksfTkErrOk)
        return rc;

    // Data is in bits [23:4]; shift right by 4 to get the 20-bit value.
    int32_t result = (int32_t)(raw >> 4);

    // Sign-extend from 20 bits.
    if (result & (1L << 19))
        result |= ~((1L << 20) - 1);

    value = result;
    return ksfTkErrOk;
}

sfTkError_t sfDevINA228::getBusVoltageRaw(uint32_t &value)
{
    uint32_t raw = 0;
    sfTkError_t rc = readRegister24(kRegVBus, raw);
    if (rc != ksfTkErrOk)
        return rc;

    // Data is in bits [23:4]; shift right by 4. Always positive.
    value = raw >> 4;
    return ksfTkErrOk;
}

sfTkError_t sfDevINA228::getCurrentRaw(int32_t &value)
{
    uint32_t raw = 0;
    sfTkError_t rc = readRegister24(kRegCurrent, raw);
    if (rc != ksfTkErrOk)
        return rc;

    // Data is in bits [23:4]; shift right by 4 to get the 20-bit value.
    int32_t result = (int32_t)(raw >> 4);

    // Sign-extend from 20 bits.
    if (result & (1L << 19))
        result |= ~((1L << 20) - 1);

    value = result;
    return ksfTkErrOk;
}

sfTkError_t sfDevINA228::getPowerRaw(uint32_t &value)
{
    // All 24 bits are data (unsigned).
    return readRegister24(kRegPower, value);
}
