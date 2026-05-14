/**
 * @file sfDevINA2XX.cpp
 * @brief Implementation file for the SparkFun INA2XX family base driver.
 *
 * @details
 * This file implements the sfDevINA2XX base class methods for configuring and reading
 * shared registers on the INA228 and INA237 power monitor ICs. Device-specific measurement
 * methods are in sfDevINA228.cpp and sfDevINA237.cpp.
 *
 * @author SparkFun Electronics
 * @date 2025
 * @copyright Copyright (c) 2025, SparkFun Electronics Inc. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * @see https://github.com/sparkfun/SparkFun_INA2XX_Arduino_Library
 */

#include "sfDevINA2XX.h"

#define DEBUG_SERIAL_PRINTS (0)
#if DEBUG_SERIAL_PRINTS
#include "Arduino.h"
#endif

// ========================= Setup & Identity =================================

bool sfDevINA2XX::begin(sfTkIBus *theBus)
{
    if (!_theBus && !theBus)
        return false;

    if (theBus != nullptr)
        setCommunicationBus(theBus);

    // INA228/INA237 are big-endian for register data.
    _theBus->setByteOrder(sfTkByteOrder::BigEndian);

    return true;
}

void sfDevINA2XX::setCommunicationBus(sfTkIBus *theBus)
{
    _theBus = theBus;
}

uint16_t sfDevINA2XX::getManufacturerID(void)
{
    uint16_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfINA2XXRegManufacturerID, value) != ksfTkErrOk)
        return 0;

    return value;
}

uint16_t sfDevINA2XX::getDeviceID(void)
{
    uint16_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfINA2XXRegDeviceID, value) != ksfTkErrOk)
        return 0;

    return value;
}

bool sfDevINA2XX::reset(void)
{
    if (!_theBus)
        return false;

    // Read current CONFIG, set the RST bit, and write back.
    uint16_t config = 0;
    if (_theBus->readRegister(ksfINA2XXRegConfig, config) != ksfTkErrOk)
        return false;

    config |= ksfINA2XXConfigRst;

    return (_theBus->writeRegister(ksfINA2XXRegConfig, config) == ksfTkErrOk);
}

bool sfDevINA2XX::resetAccumulators(void)
{
    return _setRegisterBit(ksfINA2XXRegConfig, ksfINA2XXConfigRstAcc, true);
}

// ========================= CONFIG Register (0x00) ===========================

bool sfDevINA2XX::setADCRange(bool reducedRange)
{
    bool result = _setRegisterBit(ksfINA2XXRegConfig, ksfINA2XXConfigAdcRange, reducedRange);
    if (result)
        _adcRange = reducedRange;
    return result;
}

bool sfDevINA2XX::getADCRange(void)
{
    _adcRange = _getRegisterBit(ksfINA2XXRegConfig, ksfINA2XXConfigAdcRange);
    return _adcRange;
}

bool sfDevINA2XX::setConversionDelay(uint8_t delay2ms)
{
    return _setRegisterField(ksfINA2XXRegConfig, ksfINA2XXConfigConvDlyMask,
                             ksfINA2XXConfigConvDlyShift, delay2ms);
}

uint8_t sfDevINA2XX::getConversionDelay(void)
{
    return _getRegisterField(ksfINA2XXRegConfig, ksfINA2XXConfigConvDlyMask,
                             ksfINA2XXConfigConvDlyShift);
}

bool sfDevINA2XX::enableTempCompensation(bool enable)
{
    return _setRegisterBit(ksfINA2XXRegConfig, ksfINA2XXConfigTempComp, enable);
}

bool sfDevINA2XX::getTempCompensation(void)
{
    return _getRegisterBit(ksfINA2XXRegConfig, ksfINA2XXConfigTempComp);
}

// ====================== ADC_CONFIG Register (0x01) ==========================

bool sfDevINA2XX::setADCMode(sfe_ina2xx_mode_t mode)
{
    return _setRegisterField(ksfINA2XXRegAdcConfig, ksfINA2XXAdcConfigModeMask,
                             ksfINA2XXAdcConfigModeShift, (uint8_t)mode);
}

sfe_ina2xx_mode_t sfDevINA2XX::getADCMode(void)
{
    return (sfe_ina2xx_mode_t)_getRegisterField(ksfINA2XXRegAdcConfig,
                                                 ksfINA2XXAdcConfigModeMask,
                                                 ksfINA2XXAdcConfigModeShift);
}

bool sfDevINA2XX::setBusVoltageConvTime(sfe_ina2xx_conv_time_t time)
{
    return _setRegisterField(ksfINA2XXRegAdcConfig, ksfINA2XXAdcConfigVBusCTMask,
                             ksfINA2XXAdcConfigVBusCTShift, (uint8_t)time);
}

sfe_ina2xx_conv_time_t sfDevINA2XX::getBusVoltageConvTime(void)
{
    return (sfe_ina2xx_conv_time_t)_getRegisterField(ksfINA2XXRegAdcConfig,
                                                      ksfINA2XXAdcConfigVBusCTMask,
                                                      ksfINA2XXAdcConfigVBusCTShift);
}

bool sfDevINA2XX::setShuntVoltageConvTime(sfe_ina2xx_conv_time_t time)
{
    return _setRegisterField(ksfINA2XXRegAdcConfig, ksfINA2XXAdcConfigVShCTMask,
                             ksfINA2XXAdcConfigVShCTShift, (uint8_t)time);
}

sfe_ina2xx_conv_time_t sfDevINA2XX::getShuntVoltageConvTime(void)
{
    return (sfe_ina2xx_conv_time_t)_getRegisterField(ksfINA2XXRegAdcConfig,
                                                      ksfINA2XXAdcConfigVShCTMask,
                                                      ksfINA2XXAdcConfigVShCTShift);
}

bool sfDevINA2XX::setTempConvTime(sfe_ina2xx_conv_time_t time)
{
    return _setRegisterField(ksfINA2XXRegAdcConfig, ksfINA2XXAdcConfigVTCTMask,
                             ksfINA2XXAdcConfigVTCTShift, (uint8_t)time);
}

sfe_ina2xx_conv_time_t sfDevINA2XX::getTempConvTime(void)
{
    return (sfe_ina2xx_conv_time_t)_getRegisterField(ksfINA2XXRegAdcConfig,
                                                      ksfINA2XXAdcConfigVTCTMask,
                                                      ksfINA2XXAdcConfigVTCTShift);
}

bool sfDevINA2XX::setAveragingCount(sfe_ina2xx_avg_count_t count)
{
    return _setRegisterField(ksfINA2XXRegAdcConfig, ksfINA2XXAdcConfigAvgMask,
                             0, (uint8_t)count);
}

sfe_ina2xx_avg_count_t sfDevINA2XX::getAveragingCount(void)
{
    return (sfe_ina2xx_avg_count_t)_getRegisterField(ksfINA2XXRegAdcConfig,
                                                      ksfINA2XXAdcConfigAvgMask, 0);
}

// ================== Calibration Registers (0x02-0x03) =======================

bool sfDevINA2XX::setShuntCal(uint16_t calValue)
{
    if (!_theBus)
        return false;

    // Bit 15 is reserved; mask to 15 bits.
    calValue &= 0x7FFF;
    return (_theBus->writeRegister(ksfINA2XXRegShuntCal, calValue) == ksfTkErrOk);
}

uint16_t sfDevINA2XX::getShuntCal(void)
{
    uint16_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfINA2XXRegShuntCal, value) != ksfTkErrOk)
        return 0;

    return value & 0x7FFF;
}

bool sfDevINA2XX::setShuntTempCoefficient(uint16_t ppmPerDegC)
{
    if (!_theBus)
        return false;

    // Mask to 14 bits (bits [13:0]).
    ppmPerDegC &= ksfINA2XXShuntTempCoMask;
    return (_theBus->writeRegister(ksfINA2XXRegShuntTempCo, ppmPerDegC) == ksfTkErrOk);
}

uint16_t sfDevINA2XX::getShuntTempCoefficient(void)
{
    uint16_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfINA2XXRegShuntTempCo, value) != ksfTkErrOk)
        return 0;

    return value & ksfINA2XXShuntTempCoMask;
}

// ================== Diagnostics & Alert (0x0B) ==============================

uint16_t sfDevINA2XX::getDiagnosticFlags(void)
{
    uint16_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfINA2XXRegDiagAlrt, value) != ksfTkErrOk)
        return 0;

    return value;
}

bool sfDevINA2XX::setAlertLatch(bool latched)
{
    return _setRegisterBit(ksfINA2XXRegDiagAlrt, ksfINA2XXDiagAlatch, latched);
}

bool sfDevINA2XX::getAlertLatch(void)
{
    return _getRegisterBit(ksfINA2XXRegDiagAlrt, ksfINA2XXDiagAlatch);
}

bool sfDevINA2XX::setConversionReadyAlert(bool enable)
{
    return _setRegisterBit(ksfINA2XXRegDiagAlrt, ksfINA2XXDiagCnvr, enable);
}

bool sfDevINA2XX::getConversionReadyAlert(void)
{
    return _getRegisterBit(ksfINA2XXRegDiagAlrt, ksfINA2XXDiagCnvr);
}

bool sfDevINA2XX::setSlowAlert(bool enable)
{
    return _setRegisterBit(ksfINA2XXRegDiagAlrt, ksfINA2XXDiagSlowAlert, enable);
}

bool sfDevINA2XX::getSlowAlert(void)
{
    return _getRegisterBit(ksfINA2XXRegDiagAlrt, ksfINA2XXDiagSlowAlert);
}

bool sfDevINA2XX::setAlertPolarity(bool activeHigh)
{
    return _setRegisterBit(ksfINA2XXRegDiagAlrt, ksfINA2XXDiagApol, activeHigh);
}

bool sfDevINA2XX::getAlertPolarity(void)
{
    return _getRegisterBit(ksfINA2XXRegDiagAlrt, ksfINA2XXDiagApol);
}

bool sfDevINA2XX::isEnergyOverflow(void)
{
    return _getDiagBit(ksfINA2XXDiagEnergyOF);
}

bool sfDevINA2XX::isChargeOverflow(void)
{
    return _getDiagBit(ksfINA2XXDiagChargeOF);
}

bool sfDevINA2XX::isMathOverflow(void)
{
    return _getDiagBit(ksfINA2XXDiagMathOF);
}

bool sfDevINA2XX::isTempOverLimit(void)
{
    return _getDiagBit(ksfINA2XXDiagTmpOL);
}

bool sfDevINA2XX::isShuntOverVoltage(void)
{
    return _getDiagBit(ksfINA2XXDiagShntOL);
}

bool sfDevINA2XX::isShuntUnderVoltage(void)
{
    return _getDiagBit(ksfINA2XXDiagShntUL);
}

bool sfDevINA2XX::isBusOverVoltage(void)
{
    return _getDiagBit(ksfINA2XXDiagBusOL);
}

bool sfDevINA2XX::isBusUnderVoltage(void)
{
    return _getDiagBit(ksfINA2XXDiagBusUL);
}

bool sfDevINA2XX::isPowerOverLimit(void)
{
    return _getDiagBit(ksfINA2XXDiagPOL);
}

bool sfDevINA2XX::isConversionReady(void)
{
    return _getDiagBit(ksfINA2XXDiagCnvrf);
}

bool sfDevINA2XX::isMemoryValid(void)
{
    return _getDiagBit(ksfINA2XXDiagMemStat);
}

// ==================== Threshold Registers (0x0C-0x11) =======================

bool sfDevINA2XX::setShuntOverVoltageThreshold(int16_t threshold)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfINA2XXRegSOVL, (uint16_t)threshold) == ksfTkErrOk);
}

int16_t sfDevINA2XX::getShuntOverVoltageThreshold(void)
{
    uint16_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfINA2XXRegSOVL, value) != ksfTkErrOk)
        return 0;

    return (int16_t)value;
}

bool sfDevINA2XX::setShuntUnderVoltageThreshold(int16_t threshold)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfINA2XXRegSUVL, (uint16_t)threshold) == ksfTkErrOk);
}

int16_t sfDevINA2XX::getShuntUnderVoltageThreshold(void)
{
    uint16_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfINA2XXRegSUVL, value) != ksfTkErrOk)
        return 0;

    return (int16_t)value;
}

bool sfDevINA2XX::setBusOverVoltageThreshold(uint16_t threshold)
{
    if (!_theBus)
        return false;

    // Bit 15 is reserved; mask to 15 bits.
    threshold &= 0x7FFF;
    return (_theBus->writeRegister(ksfINA2XXRegBOVL, threshold) == ksfTkErrOk);
}

uint16_t sfDevINA2XX::getBusOverVoltageThreshold(void)
{
    uint16_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfINA2XXRegBOVL, value) != ksfTkErrOk)
        return 0;

    return value & 0x7FFF;
}

bool sfDevINA2XX::setBusUnderVoltageThreshold(uint16_t threshold)
{
    if (!_theBus)
        return false;

    threshold &= 0x7FFF;
    return (_theBus->writeRegister(ksfINA2XXRegBUVL, threshold) == ksfTkErrOk);
}

uint16_t sfDevINA2XX::getBusUnderVoltageThreshold(void)
{
    uint16_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfINA2XXRegBUVL, value) != ksfTkErrOk)
        return 0;

    return value & 0x7FFF;
}

bool sfDevINA2XX::setTempLimitThreshold(int16_t threshold)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfINA2XXRegTempLimit, (uint16_t)threshold) == ksfTkErrOk);
}

int16_t sfDevINA2XX::getTempLimitThreshold(void)
{
    uint16_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfINA2XXRegTempLimit, value) != ksfTkErrOk)
        return 0;

    return (int16_t)value;
}

bool sfDevINA2XX::setPowerLimitThreshold(uint16_t threshold)
{
    if (!_theBus)
        return false;

    return (_theBus->writeRegister(ksfINA2XXRegPowerLimit, threshold) == ksfTkErrOk);
}

uint16_t sfDevINA2XX::getPowerLimitThreshold(void)
{
    uint16_t value = 0;

    if (!_theBus)
        return 0;

    if (_theBus->readRegister(ksfINA2XXRegPowerLimit, value) != ksfTkErrOk)
        return 0;

    return value;
}

// ========================= Protected Helpers ================================

bool sfDevINA2XX::_getDiagBit(uint16_t bitMask)
{
    uint16_t diag = getDiagnosticFlags();
    return (diag & bitMask) != 0;
}

bool sfDevINA2XX::_setRegisterBit(uint8_t reg, uint16_t bitMask, bool set)
{
    if (!_theBus)
        return false;

    uint16_t value = 0;
    if (_theBus->readRegister(reg, value) != ksfTkErrOk)
        return false;

    if (set)
        value |= bitMask;
    else
        value &= ~bitMask;

    return (_theBus->writeRegister(reg, value) == ksfTkErrOk);
}

bool sfDevINA2XX::_getRegisterBit(uint8_t reg, uint16_t bitMask)
{
    if (!_theBus)
        return false;

    uint16_t value = 0;
    if (_theBus->readRegister(reg, value) != ksfTkErrOk)
        return false;

    return (value & bitMask) != 0;
}

bool sfDevINA2XX::_setRegisterField(uint8_t reg, uint16_t mask, uint8_t shift, uint8_t value)
{
    if (!_theBus)
        return false;

    uint16_t regVal = 0;
    if (_theBus->readRegister(reg, regVal) != ksfTkErrOk)
        return false;

    regVal &= ~mask;
    regVal |= ((uint16_t)value << shift) & mask;

    return (_theBus->writeRegister(reg, regVal) == ksfTkErrOk);
}

uint8_t sfDevINA2XX::_getRegisterField(uint8_t reg, uint16_t mask, uint8_t shift)
{
    if (!_theBus)
        return 0;

    uint16_t value = 0;
    if (_theBus->readRegister(reg, value) != ksfTkErrOk)
        return 0;

    return (uint8_t)((value & mask) >> shift);
}

sfTkError_t sfDevINA2XX::_readRegister24(uint8_t reg, uint32_t &value)
{
    if (!_theBus)
        return ksfTkErrFail;

    uint8_t buf[3] = {0};
    size_t bytesRead = 0;

    sfTkError_t result = _theBus->readRegister(reg, buf, 3, bytesRead);
    if (result != ksfTkErrOk)
        return result;

    // Assemble MSB-first (big-endian device).
    value = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | (uint32_t)buf[2];

    return ksfTkErrOk;
}

sfTkError_t sfDevINA2XX::_readRegister40(uint8_t reg, uint64_t &value)
{
    if (!_theBus)
        return ksfTkErrFail;

    uint8_t buf[5] = {0};
    size_t bytesRead = 0;

    sfTkError_t result = _theBus->readRegister(reg, buf, 5, bytesRead);
    if (result != ksfTkErrOk)
        return result;

    // Assemble MSB-first (big-endian device).
    value = ((uint64_t)buf[0] << 32) | ((uint64_t)buf[1] << 24) | ((uint64_t)buf[2] << 16) |
            ((uint64_t)buf[3] << 8) | (uint64_t)buf[4];

    return ksfTkErrOk;
}