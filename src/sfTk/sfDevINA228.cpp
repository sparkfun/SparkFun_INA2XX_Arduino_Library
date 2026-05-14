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

// INA228 calibration scale constant.
static const float kINA228CalScale = 13107.2e6f;

// INA228 shunt voltage LSB in volts.
static const float kINA228ShuntLSBDefault = 312.5e-9f;  // ADCRANGE = 0
static const float kINA228ShuntLSBReduced = 78.125e-9f;  // ADCRANGE = 1

// INA228 bus voltage LSB in volts.
static const float kINA228BusLSB = 195.3125e-6f;

// INA228 die temperature LSB in degrees C.
static const float kINA228TempLSB = 7.8125e-3f;

// ========================= Calibration ======================================

bool sfDevINA228::calibrate(float shuntResOhms, float maxCurrent_A)
{
    if (!_theBus || shuntResOhms <= 0.0f || maxCurrent_A <= 0.0f)
        return false;

    _shuntRes = shuntResOhms;

    // CURRENT_LSB = MaxCurrent / 2^19  (20-bit ADC, positive half of range)
    _currentLSB = maxCurrent_A / 524288.0f;

    // SHUNT_CAL = 13107.2 x 10^6 x CURRENT_LSB x Rshunt
    float calValue = kINA228CalScale * _currentLSB * _shuntRes;

    // If ADCRANGE = 1, multiply by 4.
    _adcRange = getADCRange();
    if (_adcRange)
        calValue *= 4.0f;

    // Clamp to 15-bit range.
    uint16_t calReg = (uint16_t)calValue;
    if (calReg > 0x7FFF)
        calReg = 0x7FFF;

    return setShuntCal(calReg);
}

// ========================= Measurements (Engineering Units) =================

float sfDevINA228::getShuntVoltage_mV(void)
{
    int32_t raw = getShuntVoltageRaw();

    // Select LSB based on ADC range.
    float lsb = _adcRange ? kINA228ShuntLSBReduced : kINA228ShuntLSBDefault;

    // Convert to millivolts (LSB is in volts).
    return (float)raw * lsb * 1000.0f;
}

float sfDevINA228::getBusVoltage_V(void)
{
    uint32_t raw = getBusVoltageRaw();

    return (float)raw * kINA228BusLSB;
}

float sfDevINA228::getCurrent_A(void)
{
    int32_t raw = getCurrentRaw();

    return (float)raw * _currentLSB;
}

float sfDevINA228::getPower_W(void)
{
    uint32_t raw = getPowerRaw();

    // Power LSB = 3.2 x CURRENT_LSB.
    return (float)raw * 3.2f * _currentLSB;
}

float sfDevINA228::getDieTemp_C(void)
{
    if (!_theBus)
        return 0.0f;

    uint16_t raw = 0;
    if (_theBus->readRegister(ksfINA2XXRegDieTemp, raw) != ksfTkErrOk)
        return 0.0f;

    // DIETEMP is a 16-bit two's complement value.
    int16_t signedRaw = (int16_t)raw;

    return (float)signedRaw * kINA228TempLSB;
}

// ========================= Energy & Charge ==================================

double sfDevINA228::getEnergy_J(void)
{
    if (!_theBus)
        return 0.0;

    uint64_t raw = 0;
    if (_readRegister40(ksfINA2XXRegEnergy, raw) != ksfTkErrOk)
        return 0.0;

    // Energy LSB = 16 x 3.2 x CURRENT_LSB.
    return (double)raw * 16.0 * 3.2 * (double)_currentLSB;
}

double sfDevINA228::getCharge_C(void)
{
    if (!_theBus)
        return 0.0;

    uint64_t raw = 0;
    if (_readRegister40(ksfINA2XXRegCharge, raw) != ksfTkErrOk)
        return 0.0;

    // CHARGE is a 40-bit two's complement value.
    int64_t signedRaw = (int64_t)raw;
    // Sign-extend from 40 bits.
    if (signedRaw & ((int64_t)1 << 39))
        signedRaw |= ~(((int64_t)1 << 40) - 1);

    // Charge LSB = CURRENT_LSB.
    return (double)signedRaw * (double)_currentLSB;
}

// ========================= Raw Register Access ==============================

int32_t sfDevINA228::getShuntVoltageRaw(void)
{
    if (!_theBus)
        return 0;

    uint32_t raw = 0;
    if (_readRegister24(ksfINA2XXRegVShunt, raw) != ksfTkErrOk)
        return 0;

    // Data is in bits [23:4]; shift right by 4 to get the 20-bit value.
    int32_t value = (int32_t)(raw >> 4);

    // Sign-extend from 20 bits.
    if (value & (1L << 19))
        value |= ~((1L << 20) - 1);

    return value;
}

uint32_t sfDevINA228::getBusVoltageRaw(void)
{
    if (!_theBus)
        return 0;

    uint32_t raw = 0;
    if (_readRegister24(ksfINA2XXRegVBus, raw) != ksfTkErrOk)
        return 0;

    // Data is in bits [23:4]; shift right by 4. Always positive.
    return raw >> 4;
}

int32_t sfDevINA228::getCurrentRaw(void)
{
    if (!_theBus)
        return 0;

    uint32_t raw = 0;
    if (_readRegister24(ksfINA2XXRegCurrent, raw) != ksfTkErrOk)
        return 0;

    // Data is in bits [23:4]; shift right by 4 to get the 20-bit value.
    int32_t value = (int32_t)(raw >> 4);

    // Sign-extend from 20 bits.
    if (value & (1L << 19))
        value |= ~((1L << 20) - 1);

    return value;
}

uint32_t sfDevINA228::getPowerRaw(void)
{
    if (!_theBus)
        return 0;

    uint32_t raw = 0;
    if (_readRegister24(ksfINA2XXRegPower, raw) != ksfTkErrOk)
        return 0;

    // All 24 bits are data (unsigned).
    return raw;
}
