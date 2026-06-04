/**
 * @file sfDevINA2XX.cpp
 * @brief Implementation file for the SparkFun INA2XX family base driver.
 *
 * @details
 * This file implements the sfDevINA2XX base class methods for configuring and reading
 * shared registers on the INA228 and INA237 power monitor ICs. Device-specific measurement
 * methods are in sfDevINA228.cpp and sfDevINA237.cpp.
 *
 * Configuration registers (CONFIG, ADC_CONFIG, DIAG_ALRT) are accessed through bitfield unions:
 * the register word is read into the union, the relevant field is modified, and the word is
 * written back. Every method returns a SparkFun Toolkit error code so callers can detect and
 * propagate communication failures.
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

// ========================= Setup & Identity =================================

sfTkError_t sfDevINA2XX::begin(sfTkIBus *theBus)
{
    // Adopt the supplied bus if one was provided; otherwise keep any bus set by a prior begin().
    if (theBus != nullptr)
        _theBus = theBus;

    // We need a bus to talk to.
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    // INA228/INA237 are big-endian for register data.
    _theBus->setByteOrder(sfTkByteOrder::BigEndian);

    return ksfTkErrOk;
}

void sfDevINA2XX::setCommunicationBus(sfTkIBus *theBus)
{
    _theBus = theBus;
}

sfTkError_t sfDevINA2XX::getManufacturerID(uint16_t &id)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->readRegister(kRegManufacturerID, id);
}

sfTkError_t sfDevINA2XX::getDeviceID(uint16_t &id)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->readRegister(kRegDeviceID, id);
}

sfTkError_t sfDevINA2XX::reset(void)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_config_reg_t config = {};
    sfTkError_t rc = _theBus->readRegister(kRegConfig, config.word);
    if (rc != ksfTkErrOk)
        return rc;

    config.rst = 1;

    return _theBus->writeRegister(kRegConfig, config.word);
}

sfTkError_t sfDevINA2XX::resetAccumulators(void)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_config_reg_t config = {};
    sfTkError_t rc = _theBus->readRegister(kRegConfig, config.word);
    if (rc != ksfTkErrOk)
        return rc;

    config.rstAcc = 1;

    return _theBus->writeRegister(kRegConfig, config.word);
}

// ========================= CONFIG Register (0x00) ===========================

sfTkError_t sfDevINA2XX::setADCRange(bool reducedRange)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_config_reg_t config = {};
    sfTkError_t rc = _theBus->readRegister(kRegConfig, config.word);
    if (rc != ksfTkErrOk)
        return rc;

    config.adcRange = reducedRange ? 1 : 0;

    rc = _theBus->writeRegister(kRegConfig, config.word);
    if (rc == ksfTkErrOk)
        _adcRange = reducedRange;

    return rc;
}

sfTkError_t sfDevINA2XX::getADCRange(bool &reducedRange)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_config_reg_t config = {};
    sfTkError_t rc = _theBus->readRegister(kRegConfig, config.word);
    reducedRange = config.adcRange != 0;
    if (rc == ksfTkErrOk)
        _adcRange = reducedRange;

    return rc;
}

sfTkError_t sfDevINA2XX::setConversionDelay(uint8_t delay2ms)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_config_reg_t config = {};
    sfTkError_t rc = _theBus->readRegister(kRegConfig, config.word);
    if (rc != ksfTkErrOk)
        return rc;

    config.convDly = delay2ms;

    return _theBus->writeRegister(kRegConfig, config.word);
}

sfTkError_t sfDevINA2XX::getConversionDelay(uint8_t &delay2ms)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_config_reg_t config = {};
    sfTkError_t rc = _theBus->readRegister(kRegConfig, config.word);
    delay2ms = (uint8_t)config.convDly;
    return rc;
}

sfTkError_t sfDevINA2XX::enableTempCompensation(bool enable)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_config_reg_t config = {};
    sfTkError_t rc = _theBus->readRegister(kRegConfig, config.word);
    if (rc != ksfTkErrOk)
        return rc;

    config.tempComp = enable ? 1 : 0;

    return _theBus->writeRegister(kRegConfig, config.word);
}

sfTkError_t sfDevINA2XX::getTempCompensation(bool &enabled)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_config_reg_t config = {};
    sfTkError_t rc = _theBus->readRegister(kRegConfig, config.word);
    enabled = config.tempComp != 0;
    return rc;
}

// ====================== ADC_CONFIG Register (0x01) ==========================

sfTkError_t sfDevINA2XX::setADCMode(sfe_ina2xx_mode_t mode)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_adc_config_reg_t adc = {};
    sfTkError_t rc = _theBus->readRegister(kRegAdcConfig, adc.word);
    if (rc != ksfTkErrOk)
        return rc;

    adc.mode = (uint16_t)mode;

    return _theBus->writeRegister(kRegAdcConfig, adc.word);
}

sfTkError_t sfDevINA2XX::getADCMode(sfe_ina2xx_mode_t &mode)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_adc_config_reg_t adc = {};
    sfTkError_t rc = _theBus->readRegister(kRegAdcConfig, adc.word);
    mode = (sfe_ina2xx_mode_t)adc.mode;
    return rc;
}

sfTkError_t sfDevINA2XX::setBusVoltageConvTime(sfe_ina2xx_conv_time_t time)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_adc_config_reg_t adc = {};
    sfTkError_t rc = _theBus->readRegister(kRegAdcConfig, adc.word);
    if (rc != ksfTkErrOk)
        return rc;

    adc.vbusct = (uint16_t)time;

    return _theBus->writeRegister(kRegAdcConfig, adc.word);
}

sfTkError_t sfDevINA2XX::getBusVoltageConvTime(sfe_ina2xx_conv_time_t &time)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_adc_config_reg_t adc = {};
    sfTkError_t rc = _theBus->readRegister(kRegAdcConfig, adc.word);
    time = (sfe_ina2xx_conv_time_t)adc.vbusct;
    return rc;
}

sfTkError_t sfDevINA2XX::setShuntVoltageConvTime(sfe_ina2xx_conv_time_t time)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_adc_config_reg_t adc = {};
    sfTkError_t rc = _theBus->readRegister(kRegAdcConfig, adc.word);
    if (rc != ksfTkErrOk)
        return rc;

    adc.vshct = (uint16_t)time;

    return _theBus->writeRegister(kRegAdcConfig, adc.word);
}

sfTkError_t sfDevINA2XX::getShuntVoltageConvTime(sfe_ina2xx_conv_time_t &time)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_adc_config_reg_t adc = {};
    sfTkError_t rc = _theBus->readRegister(kRegAdcConfig, adc.word);
    time = (sfe_ina2xx_conv_time_t)adc.vshct;
    return rc;
}

sfTkError_t sfDevINA2XX::setTempConvTime(sfe_ina2xx_conv_time_t time)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_adc_config_reg_t adc = {};
    sfTkError_t rc = _theBus->readRegister(kRegAdcConfig, adc.word);
    if (rc != ksfTkErrOk)
        return rc;

    adc.vtct = (uint16_t)time;

    return _theBus->writeRegister(kRegAdcConfig, adc.word);
}

sfTkError_t sfDevINA2XX::getTempConvTime(sfe_ina2xx_conv_time_t &time)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_adc_config_reg_t adc = {};
    sfTkError_t rc = _theBus->readRegister(kRegAdcConfig, adc.word);
    time = (sfe_ina2xx_conv_time_t)adc.vtct;
    return rc;
}

sfTkError_t sfDevINA2XX::setAveragingCount(sfe_ina2xx_avg_count_t count)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_adc_config_reg_t adc = {};
    sfTkError_t rc = _theBus->readRegister(kRegAdcConfig, adc.word);
    if (rc != ksfTkErrOk)
        return rc;

    adc.avg = (uint16_t)count;

    return _theBus->writeRegister(kRegAdcConfig, adc.word);
}

sfTkError_t sfDevINA2XX::getAveragingCount(sfe_ina2xx_avg_count_t &count)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_adc_config_reg_t adc = {};
    sfTkError_t rc = _theBus->readRegister(kRegAdcConfig, adc.word);
    count = (sfe_ina2xx_avg_count_t)adc.avg;
    return rc;
}

// ================== Calibration Registers (0x02-0x03) =======================

sfTkError_t sfDevINA2XX::setShuntCal(uint16_t calValue)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    // Bit 15 is reserved; mask to 15 bits.
    return _theBus->writeRegister(kRegShuntCal, (uint16_t)(calValue & kShuntCalMask));
}

sfTkError_t sfDevINA2XX::getShuntCal(uint16_t &calValue)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfTkError_t rc = _theBus->readRegister(kRegShuntCal, calValue);
    calValue &= kShuntCalMask;
    return rc;
}

sfTkError_t sfDevINA2XX::setShuntTempCoefficient(uint16_t ppmPerDegC)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    // Mask to 14 bits (bits [13:0]).
    return _theBus->writeRegister(kRegShuntTempCo, (uint16_t)(ppmPerDegC & kShuntTempCoMask));
}

sfTkError_t sfDevINA2XX::getShuntTempCoefficient(uint16_t &ppmPerDegC)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfTkError_t rc = _theBus->readRegister(kRegShuntTempCo, ppmPerDegC);
    ppmPerDegC &= kShuntTempCoMask;
    return rc;
}

// ================== Diagnostics & Alert (0x0B) ==============================

sfTkError_t sfDevINA2XX::getDiagnosticFlags(sfe_ina2xx_diag_alrt_reg_t &flags)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->readRegister(kRegDiagAlrt, flags.word);
}

sfTkError_t sfDevINA2XX::setAlertLatch(bool latched)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    if (rc != ksfTkErrOk)
        return rc;

    diag.alatch = latched ? 1 : 0;

    return _theBus->writeRegister(kRegDiagAlrt, diag.word);
}

sfTkError_t sfDevINA2XX::getAlertLatch(bool &latched)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    latched = diag.alatch != 0;
    return rc;
}

sfTkError_t sfDevINA2XX::setConversionReadyAlert(bool enable)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    if (rc != ksfTkErrOk)
        return rc;

    diag.cnvr = enable ? 1 : 0;

    return _theBus->writeRegister(kRegDiagAlrt, diag.word);
}

sfTkError_t sfDevINA2XX::getConversionReadyAlert(bool &enabled)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    enabled = diag.cnvr != 0;
    return rc;
}

sfTkError_t sfDevINA2XX::setSlowAlert(bool enable)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    if (rc != ksfTkErrOk)
        return rc;

    diag.slowAlert = enable ? 1 : 0;

    return _theBus->writeRegister(kRegDiagAlrt, diag.word);
}

sfTkError_t sfDevINA2XX::getSlowAlert(bool &enabled)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    enabled = diag.slowAlert != 0;
    return rc;
}

sfTkError_t sfDevINA2XX::setAlertPolarity(bool activeHigh)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    if (rc != ksfTkErrOk)
        return rc;

    diag.apol = activeHigh ? 1 : 0;

    return _theBus->writeRegister(kRegDiagAlrt, diag.word);
}

sfTkError_t sfDevINA2XX::getAlertPolarity(bool &activeHigh)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    activeHigh = diag.apol != 0;
    return rc;
}

sfTkError_t sfDevINA2XX::isEnergyOverflow(bool &overflow)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    overflow = diag.energyOF != 0;
    return rc;
}

sfTkError_t sfDevINA2XX::isChargeOverflow(bool &overflow)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    overflow = diag.chargeOF != 0;
    return rc;
}

sfTkError_t sfDevINA2XX::isMathOverflow(bool &overflow)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    overflow = diag.mathOF != 0;
    return rc;
}

sfTkError_t sfDevINA2XX::isTempOverLimit(bool &overLimit)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    overLimit = diag.tmpOL != 0;
    return rc;
}

sfTkError_t sfDevINA2XX::isShuntOverVoltage(bool &overVoltage)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    overVoltage = diag.shntOL != 0;
    return rc;
}

sfTkError_t sfDevINA2XX::isShuntUnderVoltage(bool &underVoltage)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    underVoltage = diag.shntUL != 0;
    return rc;
}

sfTkError_t sfDevINA2XX::isBusOverVoltage(bool &overVoltage)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    overVoltage = diag.busOL != 0;
    return rc;
}

sfTkError_t sfDevINA2XX::isBusUnderVoltage(bool &underVoltage)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    underVoltage = diag.busUL != 0;
    return rc;
}

sfTkError_t sfDevINA2XX::isPowerOverLimit(bool &overLimit)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    overLimit = diag.pol != 0;
    return rc;
}

sfTkError_t sfDevINA2XX::isConversionReady(bool &ready)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    ready = diag.cnvrf != 0;
    return rc;
}

sfTkError_t sfDevINA2XX::isMemoryValid(bool &valid)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    valid = diag.memStat != 0;
    return rc;
}

// ==================== Threshold Registers (0x0C-0x11) =======================

sfTkError_t sfDevINA2XX::setShuntOverVoltageThreshold(int16_t threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->writeRegister(kRegSOVL, (uint16_t)threshold);
}

sfTkError_t sfDevINA2XX::getShuntOverVoltageThreshold(int16_t &threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    uint16_t value = 0;
    sfTkError_t rc = _theBus->readRegister(kRegSOVL, value);
    threshold = (int16_t)value;
    return rc;
}

sfTkError_t sfDevINA2XX::setShuntUnderVoltageThreshold(int16_t threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->writeRegister(kRegSUVL, (uint16_t)threshold);
}

sfTkError_t sfDevINA2XX::getShuntUnderVoltageThreshold(int16_t &threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    uint16_t value = 0;
    sfTkError_t rc = _theBus->readRegister(kRegSUVL, value);
    threshold = (int16_t)value;
    return rc;
}

sfTkError_t sfDevINA2XX::setBusOverVoltageThreshold(uint16_t threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    // Bit 15 is reserved; mask to 15 bits.
    return _theBus->writeRegister(kRegBOVL, (uint16_t)(threshold & kBusThresholdMask));
}

sfTkError_t sfDevINA2XX::getBusOverVoltageThreshold(uint16_t &threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfTkError_t rc = _theBus->readRegister(kRegBOVL, threshold);
    threshold &= kBusThresholdMask;
    return rc;
}

sfTkError_t sfDevINA2XX::setBusUnderVoltageThreshold(uint16_t threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->writeRegister(kRegBUVL, (uint16_t)(threshold & kBusThresholdMask));
}

sfTkError_t sfDevINA2XX::getBusUnderVoltageThreshold(uint16_t &threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfTkError_t rc = _theBus->readRegister(kRegBUVL, threshold);
    threshold &= kBusThresholdMask;
    return rc;
}

sfTkError_t sfDevINA2XX::setTempLimitThreshold(int16_t threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->writeRegister(kRegTempLimit, (uint16_t)threshold);
}

sfTkError_t sfDevINA2XX::getTempLimitThreshold(int16_t &threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    uint16_t value = 0;
    sfTkError_t rc = _theBus->readRegister(kRegTempLimit, value);
    threshold = (int16_t)value;
    return rc;
}

sfTkError_t sfDevINA2XX::setPowerLimitThreshold(uint16_t threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->writeRegister(kRegPowerLimit, threshold);
}

sfTkError_t sfDevINA2XX::getPowerLimitThreshold(uint16_t &threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->readRegister(kRegPowerLimit, threshold);
}

// ========================= Protected Helpers ================================

sfTkError_t sfDevINA2XX::readRegister24(uint8_t reg, uint32_t &value)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    uint8_t buf[3] = {0};
    size_t bytesRead = 0;

    sfTkError_t rc = _theBus->readRegister(reg, buf, 3, bytesRead);
    if (rc != ksfTkErrOk)
        return rc;

    // Assemble MSB-first (big-endian device).
    value = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | (uint32_t)buf[2];

    return ksfTkErrOk;
}

sfTkError_t sfDevINA2XX::readRegister40(uint8_t reg, uint64_t &value)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    uint8_t buf[5] = {0};
    size_t bytesRead = 0;

    sfTkError_t rc = _theBus->readRegister(reg, buf, 5, bytesRead);
    if (rc != ksfTkErrOk)
        return rc;

    // Assemble MSB-first (big-endian device).
    value = ((uint64_t)buf[0] << 32) | ((uint64_t)buf[1] << 24) | ((uint64_t)buf[2] << 16) |
            ((uint64_t)buf[3] << 8) | (uint64_t)buf[4];

    return ksfTkErrOk;
}
