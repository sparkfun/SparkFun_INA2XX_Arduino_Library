/**
 * @file sfDevINA237.cpp
 * @brief Implementation file for the SparkFun INA237 power monitor driver.
 *
 * @details
 * This file implements INA237-specific measurement methods including 16-bit register reads
 * and the INA237 calibration formula.
 *
 * INA237 ADC Details:
 *   - 16-bit ADC
 *   - VSHUNT/VBUS/CURRENT registers are 16 bits wide
 *   - POWER register is 24 bits wide (unsigned)
 *   - No ENERGY or CHARGE registers
 *   - Shunt LSB: 5 uV (ADCRANGE=0) or 1.25 uV (ADCRANGE=1)
 *   - Bus LSB: 3.125 mV
 *   - Temp LSB: 125 m-deg-C (data in bits [15:4], 12-bit)
 *   - Calibration scale: 819.2 x 10^6 (x4 for ADCRANGE=1)
 *
 * @author SparkFun Electronics
 * @date 2025
 * @copyright Copyright (c) 2025, SparkFun Electronics Inc. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * @see https://github.com/sparkfun/SparkFun_INA2XX_Arduino_Library
 */

#include "sfDevINA237.h"

// ========================= Calibration ======================================

sfTkError_t sfDevINA237::calibrate(float shuntResOhms, float maxCurrentA)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    if (shuntResOhms <= 0.0f || maxCurrentA <= 0.0f)
        return ksfTkErrFail;

    _shuntRes = shuntResOhms;

    // CURRENT_LSB = MaxCurrent / 2^15  (16-bit ADC, positive half of range)
    _currentLSB = maxCurrentA / kCurrentFullScale;

    // SHUNT_CAL = 819.2 x 10^6 x CURRENT_LSB x Rshunt
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

sfTkError_t sfDevINA237::getShuntVoltage_mV(float &milliVolts)
{
    int16_t raw = 0;
    sfTkError_t rc = getShuntVoltageRaw(raw);
    if (rc != ksfTkErrOk)
        return rc;

    // Select LSB based on ADC range, then convert to millivolts (LSB is in volts).
    float lsb = _adcRange ? kShuntLSBReduced : kShuntLSBDefault;
    milliVolts = (float)raw * lsb * 1000.0f;
    return ksfTkErrOk;
}

sfTkError_t sfDevINA237::getBusVoltage_V(float &volts)
{
    uint16_t raw = 0;
    sfTkError_t rc = getBusVoltageRaw(raw);
    if (rc != ksfTkErrOk)
        return rc;

    volts = (float)raw * kBusLSB;
    return ksfTkErrOk;
}

sfTkError_t sfDevINA237::getCurrent_A(float &amps)
{
    int16_t raw = 0;
    sfTkError_t rc = getCurrentRaw(raw);
    if (rc != ksfTkErrOk)
        return rc;

    amps = (float)raw * _currentLSB;
    return ksfTkErrOk;
}

sfTkError_t sfDevINA237::getPower_W(float &watts)
{
    uint32_t raw = 0;
    sfTkError_t rc = getPowerRaw(raw);
    if (rc != ksfTkErrOk)
        return rc;

    // Power LSB = 0.2 x CURRENT_LSB.
    watts = (float)raw * kPowerLSBScale * _currentLSB;
    return ksfTkErrOk;
}

sfTkError_t sfDevINA237::getDieTemp_C(float &celsius)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    uint16_t raw = 0;
    sfTkError_t rc = _theBus->readRegister(kRegDieTemp, raw);
    if (rc != ksfTkErrOk)
        return rc;

    // Data is in bits [15:4]; shift right by 4 to get a 12-bit two's complement value.
    int16_t signedRaw = ((int16_t)raw) >> 4;

    celsius = (float)signedRaw * kTempLSB;
    return ksfTkErrOk;
}

// ========================= Raw Register Access ==============================

sfTkError_t sfDevINA237::getShuntVoltageRaw(int16_t &value)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    uint16_t raw = 0;
    sfTkError_t rc = _theBus->readRegister(kRegVShunt, raw);
    // VSHUNT is a 16-bit two's complement value.
    value = (int16_t)raw;
    return rc;
}

sfTkError_t sfDevINA237::getBusVoltageRaw(uint16_t &value)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->readRegister(kRegVBus, value);
}

sfTkError_t sfDevINA237::getCurrentRaw(int16_t &value)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    uint16_t raw = 0;
    sfTkError_t rc = _theBus->readRegister(kRegCurrent, raw);
    // CURRENT is a 16-bit two's complement value.
    value = (int16_t)raw;
    return rc;
}

sfTkError_t sfDevINA237::getPowerRaw(uint32_t &value)
{
    // All 24 bits are data (unsigned).
    return readRegister24(kRegPower, value);
}
