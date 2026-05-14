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

// INA237 calibration scale constant.
static const float kINA237CalScale = 819.2e6f;

// INA237 shunt voltage LSB in volts.
static const float kINA237ShuntLSBDefault = 5.0e-6f;    // ADCRANGE = 0
static const float kINA237ShuntLSBReduced = 1.25e-6f;   // ADCRANGE = 1

// INA237 bus voltage LSB in volts.
static const float kINA237BusLSB = 3.125e-3f;

// INA237 die temperature LSB in degrees C.
static const float kINA237TempLSB = 125.0e-3f;

// ========================= Calibration ======================================

bool sfDevINA237::calibrate(float shuntResOhms, float maxCurrent_A)
{
    if (!_theBus || shuntResOhms <= 0.0f || maxCurrent_A <= 0.0f)
        return false;

    _shuntRes = shuntResOhms;

    // CURRENT_LSB = MaxCurrent / 2^15  (16-bit ADC, positive half of range)
    _currentLSB = maxCurrent_A / 32768.0f;

    // SHUNT_CAL = 819.2 x 10^6 x CURRENT_LSB x Rshunt
    float calValue = kINA237CalScale * _currentLSB * _shuntRes;

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

float sfDevINA237::getShuntVoltage_mV(void)
{
    int16_t raw = getShuntVoltageRaw();

    // Select LSB based on ADC range.
    float lsb = _adcRange ? kINA237ShuntLSBReduced : kINA237ShuntLSBDefault;

    // Convert to millivolts (LSB is in volts).
    return (float)raw * lsb * 1000.0f;
}

float sfDevINA237::getBusVoltage_V(void)
{
    uint16_t raw = getBusVoltageRaw();

    return (float)raw * kINA237BusLSB;
}

float sfDevINA237::getCurrent_A(void)
{
    int16_t raw = getCurrentRaw();

    return (float)raw * _currentLSB;
}

float sfDevINA237::getPower_W(void)
{
    uint32_t raw = getPowerRaw();

    // Power LSB = 0.2 x CURRENT_LSB.
    return (float)raw * 0.2f * _currentLSB;
}

float sfDevINA237::getDieTemp_C(void)
{
    if (!_theBus)
        return 0.0f;

    uint16_t raw = 0;
    if (_theBus->readRegister(ksfINA2XXRegDieTemp, raw) != ksfTkErrOk)
        return 0.0f;

    // Data is in bits [15:4]; shift right by 4 to get a 12-bit two's complement value.
    int16_t signedRaw = ((int16_t)raw) >> 4;

    return (float)signedRaw * kINA237TempLSB;
}

// ========================= Raw Register Access ==============================

int16_t sfDevINA237::getShuntVoltageRaw(void)
{
    if (!_theBus)
        return 0;

    uint16_t raw = 0;
    if (_theBus->readRegister(ksfINA2XXRegVShunt, raw) != ksfTkErrOk)
        return 0;

    // VSHUNT is a 16-bit two's complement value.
    return (int16_t)raw;
}

uint16_t sfDevINA237::getBusVoltageRaw(void)
{
    if (!_theBus)
        return 0;

    uint16_t raw = 0;
    if (_theBus->readRegister(ksfINA2XXRegVBus, raw) != ksfTkErrOk)
        return 0;

    return raw;
}

int16_t sfDevINA237::getCurrentRaw(void)
{
    if (!_theBus)
        return 0;

    uint16_t raw = 0;
    if (_theBus->readRegister(ksfINA2XXRegCurrent, raw) != ksfTkErrOk)
        return 0;

    // CURRENT is a 16-bit two's complement value.
    return (int16_t)raw;
}

uint32_t sfDevINA237::getPowerRaw(void)
{
    if (!_theBus)
        return 0;

    uint32_t raw = 0;
    if (_readRegister24(ksfINA2XXRegPower, raw) != ksfTkErrOk)
        return 0;

    // All 24 bits are data (unsigned).
    return raw;
}
