/**
 * @file sfDevINA2XX.cpp
 * @brief Implementation file for the SparkFun INA2XX family base driver.
 *
 * @details
 * This file defines the sfDevINA2XX class-template member functions and explicitly instantiates
 * the template for the two type combinations the library uses: @c <int32_t, uint32_t> (INA228)
 * and @c <int16_t, uint16_t> (INA237). Because the template is explicitly instantiated here, the
 * definitions do not need to live in the header — each instantiation is compiled exactly once in
 * this translation unit, and the @c extern @c template declarations in sfDevINA2XX.h point every
 * other translation unit at these copies.
 *
 * Configuration registers (CONFIG, ADC_CONFIG, DIAG_ALRT) are accessed through bitfield unions:
 * the register word is read into the union, the relevant field is modified, and the word is
 * written back. Every method returns a SparkFun Toolkit error code so callers can detect and
 * propagate communication failures.
 *
 * Device-specific features that are not shared (the INA228 energy/charge accumulators) live in
 * sfDevINA228.cpp.
 *
 * @author SparkFun Electronics
 * @date June 2026
 * @copyright Copyright (c) 2026, SparkFun Electronics Inc. This project is released under the MIT License.
 *
 * SPDX-License-Identifier: MIT
 *
 * @see https://github.com/sparkfun/SparkFun_INA2XX_Arduino_Library
 */

#include "sfDevINA2XX.h"

// ========================= Setup & Identity =================================

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::begin(sfTkIBus *theBus)
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

template <typename signed_raw_t, typename unsigned_raw_t>
void sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setCommunicationBus(sfTkIBus *theBus)
{
    _theBus = theBus;
}

template <typename signed_raw_t, typename unsigned_raw_t>
bool sfDevINA2XX<signed_raw_t, unsigned_raw_t>::isConnected(void)
{
    uint16_t devID = 0;
    if (getDeviceID(devID) != ksfTkErrOk)
        return false;

    // Match the expected device ID, or the alternate ID of a register-compatible part.
    uint16_t maskedID = devID & kDeviceIDMask;
    return (maskedID == _deviceID || maskedID == _deviceIDAlt);
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getManufacturerID(uint16_t &id)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->readRegister(kRegManufacturerID, id);
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getDeviceID(uint16_t &id)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->readRegister(kRegDeviceID, id);
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::reset(void)
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

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::resetAccumulators(void)
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

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setADCRange(bool reducedRange)
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

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getADCRange(bool &reducedRange)
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

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setConversionDelay(uint8_t delay2ms)
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

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getConversionDelay(uint8_t &delay2ms)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_config_reg_t config = {};
    sfTkError_t rc = _theBus->readRegister(kRegConfig, config.word);
    delay2ms = (uint8_t)config.convDly;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::enableTempCompensation(bool enable)
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

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getTempCompensation(bool &enabled)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_config_reg_t config = {};
    sfTkError_t rc = _theBus->readRegister(kRegConfig, config.word);
    enabled = config.tempComp != 0;
    return rc;
}

// ====================== ADC_CONFIG Register (0x01) ==========================

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setADCMode(sfe_ina2xx_mode_t mode)
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

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getADCMode(sfe_ina2xx_mode_t &mode)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_adc_config_reg_t adc = {};
    sfTkError_t rc = _theBus->readRegister(kRegAdcConfig, adc.word);
    mode = (sfe_ina2xx_mode_t)adc.mode;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setBusVoltageConvTime(sfe_ina2xx_conv_time_t time)
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

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getBusVoltageConvTime(sfe_ina2xx_conv_time_t &time)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_adc_config_reg_t adc = {};
    sfTkError_t rc = _theBus->readRegister(kRegAdcConfig, adc.word);
    time = (sfe_ina2xx_conv_time_t)adc.vbusct;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setShuntVoltageConvTime(sfe_ina2xx_conv_time_t time)
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

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getShuntVoltageConvTime(sfe_ina2xx_conv_time_t &time)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_adc_config_reg_t adc = {};
    sfTkError_t rc = _theBus->readRegister(kRegAdcConfig, adc.word);
    time = (sfe_ina2xx_conv_time_t)adc.vshct;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setTempConvTime(sfe_ina2xx_conv_time_t time)
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

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getTempConvTime(sfe_ina2xx_conv_time_t &time)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_adc_config_reg_t adc = {};
    sfTkError_t rc = _theBus->readRegister(kRegAdcConfig, adc.word);
    time = (sfe_ina2xx_conv_time_t)adc.vtct;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setAveragingCount(sfe_ina2xx_avg_count_t count)
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

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getAveragingCount(sfe_ina2xx_avg_count_t &count)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_adc_config_reg_t adc = {};
    sfTkError_t rc = _theBus->readRegister(kRegAdcConfig, adc.word);
    count = (sfe_ina2xx_avg_count_t)adc.avg;
    return rc;
}

// ================== Calibration Registers (0x02-0x03) =======================

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::calibrate(float shuntResOhms, float maxCurrentA)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    if (shuntResOhms <= 0.0f || maxCurrentA <= 0.0f)
        return ksfTkErrFail;

    _shuntRes = shuntResOhms;

    // CURRENT_LSB = MaxCurrent / 2^(ADC bits - 1)  (positive half of the ADC range)
    _currentLSB = maxCurrentA / _currentFullScale;

    // SHUNT_CAL = CalScale x CURRENT_LSB x Rshunt
    float calValue = _calScale * _currentLSB * _shuntRes;

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

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setShuntCal(uint16_t calValue)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    // Bit 15 is reserved; mask to 15 bits.
    return _theBus->writeRegister(kRegShuntCal, (uint16_t)(calValue & kShuntCalMask));
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getShuntCal(uint16_t &calValue)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfTkError_t rc = _theBus->readRegister(kRegShuntCal, calValue);
    calValue &= kShuntCalMask;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setShuntTempCoefficient(uint16_t ppmPerDegC)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    // Mask to 14 bits (bits [13:0]).
    return _theBus->writeRegister(kRegShuntTempCo, (uint16_t)(ppmPerDegC & kShuntTempCoMask));
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getShuntTempCoefficient(uint16_t &ppmPerDegC)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfTkError_t rc = _theBus->readRegister(kRegShuntTempCo, ppmPerDegC);
    ppmPerDegC &= kShuntTempCoMask;
    return rc;
}

// ================== Measurements (Engineering Units) ========================

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getShuntVoltage_mV(float &milliVolts)
{
    signed_raw_t raw = 0;
    sfTkError_t rc = getShuntVoltageRaw(raw);
    if (rc != ksfTkErrOk)
        return rc;

    // Select LSB based on ADC range, then convert to millivolts (LSB is in volts).
    float lsb = _adcRange ? _shuntLSBReduced : _shuntLSBDefault;
    milliVolts = (float)raw * lsb * 1000.0f;
    return ksfTkErrOk;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getBusVoltage_V(float &volts)
{
    unsigned_raw_t raw = 0;
    sfTkError_t rc = getBusVoltageRaw(raw);
    if (rc != ksfTkErrOk)
        return rc;

    volts = (float)raw * _busLSB;
    return ksfTkErrOk;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getCurrent_A(float &amps)
{
    signed_raw_t raw = 0;
    sfTkError_t rc = getCurrentRaw(raw);
    if (rc != ksfTkErrOk)
        return rc;

    amps = (float)raw * _currentLSB;
    return ksfTkErrOk;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getPower_W(float &watts)
{
    uint32_t raw = 0;
    sfTkError_t rc = getPowerRaw(raw);
    if (rc != ksfTkErrOk)
        return rc;

    watts = (float)raw * _powerLSBScale * _currentLSB;
    return ksfTkErrOk;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getDieTemp_C(float &celsius)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    uint16_t raw = 0;
    sfTkError_t rc = _theBus->readRegister(kRegDieTemp, raw);
    if (rc != ksfTkErrOk)
        return rc;

    // Two's complement; the arithmetic shift discards reserved low bits (if any).
    int16_t signedRaw = (int16_t)raw >> _tempShift;
    celsius = (float)signedRaw * _tempLSB;
    return ksfTkErrOk;
}

// ========================= Raw Register Access ==============================

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getShuntVoltageRaw(signed_raw_t &value)
{
    return readMeasurementSigned(kRegVShunt, value);
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getBusVoltageRaw(unsigned_raw_t &value)
{
    return readMeasurementUnsigned(kRegVBus, value);
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getCurrentRaw(signed_raw_t &value)
{
    return readMeasurementSigned(kRegCurrent, value);
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getPowerRaw(uint32_t &value)
{
    // All 24 bits are data (unsigned) on both devices.
    return readRegister24(kRegPower, value);
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::readMeasurementSigned(uint8_t reg, signed_raw_t &value)
{
    if (_is24BitMeasurements)
    {
        uint32_t raw = 0;
        sfTkError_t rc = readRegister24(reg, raw);
        if (rc != ksfTkErrOk)
            return rc;

        // Data is in bits [23:4]; shift right by 4 to get the 20-bit value.
        int32_t result = (int32_t)(raw >> 4);

        // Sign-extend from 20 bits.
        if (result & (1L << 19))
            result |= ~((1L << 20) - 1);

        value = (signed_raw_t)result;
    }
    else
    {
        if (_theBus == nullptr)
            return ksfTkErrBusNotInit;

        uint16_t raw = 0;
        sfTkError_t rc = _theBus->readRegister(reg, raw);
        if (rc != ksfTkErrOk)
            return rc;

        // 16-bit two's complement value.
        value = (signed_raw_t)(int16_t)raw;
    }

    return ksfTkErrOk;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::readMeasurementUnsigned(uint8_t reg, unsigned_raw_t &value)
{
    if (_is24BitMeasurements)
    {
        uint32_t raw = 0;
        sfTkError_t rc = readRegister24(reg, raw);
        if (rc != ksfTkErrOk)
            return rc;

        // Data is in bits [23:4]; shift right by 4. Always positive.
        value = (unsigned_raw_t)(raw >> 4);
    }
    else
    {
        if (_theBus == nullptr)
            return ksfTkErrBusNotInit;

        uint16_t raw = 0;
        sfTkError_t rc = _theBus->readRegister(reg, raw);
        if (rc != ksfTkErrOk)
            return rc;

        value = (unsigned_raw_t)raw;
    }

    return ksfTkErrOk;
}

// ================== Diagnostics & Alert (0x0B) ==============================

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getDiagnosticFlags(sfe_ina2xx_diag_alrt_reg_t &flags)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->readRegister(kRegDiagAlrt, flags.word);
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setAlertLatch(bool latched)
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

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getAlertLatch(bool &latched)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    latched = diag.alatch != 0;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setConversionReadyAlert(bool enable)
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

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getConversionReadyAlert(bool &enabled)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    enabled = diag.cnvr != 0;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setSlowAlert(bool enable)
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

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getSlowAlert(bool &enabled)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    enabled = diag.slowAlert != 0;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setAlertPolarity(bool activeHigh)
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

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getAlertPolarity(bool &activeHigh)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    activeHigh = diag.apol != 0;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::isEnergyOverflow(bool &overflow)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    overflow = diag.energyOF != 0;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::isChargeOverflow(bool &overflow)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    overflow = diag.chargeOF != 0;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::isMathOverflow(bool &overflow)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    overflow = diag.mathOF != 0;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::isTempOverLimit(bool &overLimit)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    overLimit = diag.tmpOL != 0;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::isShuntOverVoltage(bool &overVoltage)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    overVoltage = diag.shntOL != 0;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::isShuntUnderVoltage(bool &underVoltage)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    underVoltage = diag.shntUL != 0;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::isBusOverVoltage(bool &overVoltage)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    overVoltage = diag.busOL != 0;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::isBusUnderVoltage(bool &underVoltage)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    underVoltage = diag.busUL != 0;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::isPowerOverLimit(bool &overLimit)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    overLimit = diag.pol != 0;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::isConversionReady(bool &ready)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    ready = diag.cnvrf != 0;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::isMemoryValid(bool &valid)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfe_ina2xx_diag_alrt_reg_t diag = {};
    sfTkError_t rc = _theBus->readRegister(kRegDiagAlrt, diag.word);
    valid = diag.memStat != 0;
    return rc;
}

// ==================== Threshold Registers (0x0C-0x11) =======================

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setShuntOverVoltageThreshold(int16_t threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->writeRegister(kRegSOVL, (uint16_t)threshold);
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setShuntOverVoltageThreshold_mV(float milliVolts)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    // The SOVL LSB depends on ADCRANGE; refresh the cached range from the device.
    sfTkError_t rc = getADCRange(_adcRange);
    if (rc != ksfTkErrOk)
        return rc;

    float lsb = _adcRange ? kShuntThresholdLSBReduced_mV : kShuntThresholdLSBDefault_mV;
    float counts = milliVolts / lsb;

    // Clamp to the signed 16-bit register range.
    if (counts > 32767.0f)
        counts = 32767.0f;
    else if (counts < -32768.0f)
        counts = -32768.0f;

    return setShuntOverVoltageThreshold((int16_t)counts);
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getShuntOverVoltageThreshold(int16_t &threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    uint16_t value = 0;
    sfTkError_t rc = _theBus->readRegister(kRegSOVL, value);
    threshold = (int16_t)value;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setShuntUnderVoltageThreshold(int16_t threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->writeRegister(kRegSUVL, (uint16_t)threshold);
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getShuntUnderVoltageThreshold(int16_t &threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    uint16_t value = 0;
    sfTkError_t rc = _theBus->readRegister(kRegSUVL, value);
    threshold = (int16_t)value;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setBusOverVoltageThreshold(uint16_t threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    // Bit 15 is reserved; mask to 15 bits.
    return _theBus->writeRegister(kRegBOVL, (uint16_t)(threshold & kBusThresholdMask));
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setBusOverVoltageThreshold_mV(float milliVolts)
{
    float counts = milliVolts / kBusThresholdLSB_mV;

    // Clamp to the unsigned 15-bit register range.
    if (counts < 0.0f)
        counts = 0.0f;
    else if (counts > 32767.0f)
        counts = 32767.0f;

    return setBusOverVoltageThreshold((uint16_t)counts);
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getBusOverVoltageThreshold(uint16_t &threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfTkError_t rc = _theBus->readRegister(kRegBOVL, threshold);
    threshold &= kBusThresholdMask;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setBusUnderVoltageThreshold(uint16_t threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->writeRegister(kRegBUVL, (uint16_t)(threshold & kBusThresholdMask));
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getBusUnderVoltageThreshold(uint16_t &threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfTkError_t rc = _theBus->readRegister(kRegBUVL, threshold);
    threshold &= kBusThresholdMask;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setTempLimitThreshold(int16_t threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->writeRegister(kRegTempLimit, (uint16_t)threshold);
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getTempLimitThreshold(int16_t &threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    uint16_t value = 0;
    sfTkError_t rc = _theBus->readRegister(kRegTempLimit, value);
    threshold = (int16_t)value;
    return rc;
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::setPowerLimitThreshold(uint16_t threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->writeRegister(kRegPowerLimit, threshold);
}

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::getPowerLimitThreshold(uint16_t &threshold)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->readRegister(kRegPowerLimit, threshold);
}

// ========================= Protected Helpers ================================

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::readRegister24(uint8_t reg, uint32_t &value)
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

template <typename signed_raw_t, typename unsigned_raw_t>
sfTkError_t sfDevINA2XX<signed_raw_t, unsigned_raw_t>::readRegister40(uint8_t reg, uint64_t &value)
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

///////////////////////////////////////////////////////////////////////////////
// Explicit template instantiations
///////////////////////////////////////////////////////////////////////////////
// Generate the two concrete classes the library uses. These provide the symbols that the
// extern template declarations in sfDevINA2XX.h reference. Add a line here (and a matching
// extern declaration in the header) to support an additional device.
template class sfDevINA2XX<int32_t, uint32_t>; // INA228 (20-bit values in 32-bit words)
template class sfDevINA2XX<int16_t, uint16_t>; // INA237 (plain 16-bit values)
