/**
 * @file sfDevINA2XX.h
 * @brief Header file for the SparkFun INA2XX family base driver.
 *
 * @details
 * sfDevINA2XX is the platform-independent base class for the Texas Instruments INA228 and INA237
 * digital power/energy monitor ICs. It provides register access for all configuration and
 * diagnostic registers that are shared between both devices:
 *
 *   - CONFIG (0x00)         — system reset, ADC range, conversion delay, temp compensation
 *   - ADC_CONFIG (0x01)     — operating mode, conversion times, averaging
 *   - SHUNT_CAL (0x02)      — shunt calibration value
 *   - SHUNT_TEMPCO (0x03)   — shunt temperature coefficient
 *   - DIETEMP (0x06)        — die temperature measurement
 *   - DIAG_ALRT (0x0B)      — diagnostic flags and alert configuration
 *   - SOVL/SUVL (0x0C/0x0D) — shunt over/undervoltage thresholds
 *   - BOVL/BUVL (0x0E/0x0F) — bus over/undervoltage thresholds
 *   - TEMP_LIMIT (0x10)     — temperature over-limit threshold
 *   - PWR_LIMIT (0x11)      — power over-limit threshold
 *   - MANUFACTURER_ID (0x3E) / DEVICE_ID (0x3F)
 *
 * Device-specific measurement registers (VSHUNT, VBUS, CURRENT, POWER, ENERGY, CHARGE) are
 * implemented in the derived sfDevINA228 and sfDevINA237 classes because they differ in
 * register width and LSB scaling between the two ICs.
 *
 * Both devices use 8-bit register addresses and big-endian byte order over I2C.
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

#include <stdint.h>

// SparkFun Toolkit core headers
#include <sfTk/sfToolkit.h>
#include <sfTk/sfTkII2C.h>

///////////////////////////////////////////////////////////////////////////////
// I2C Addressing
///////////////////////////////////////////////////////////////////////////////
/// Default 7-bit I2C address for the SparkFun Qwiic Power Monitor board.
/// A0 = VS, A1 = GND gives 0x45. Both INA228 and INA237 support 0x40-0x4F.
const uint8_t ksfINA2XXDefaultAddr = 0x45;

///////////////////////////////////////////////////////////////////////////////
// Register Addresses (shared between INA228 and INA237)
///////////////////////////////////////////////////////////////////////////////
const uint8_t ksfINA2XXRegConfig = 0x00;          ///< Configuration register (16-bit, R/W)
const uint8_t ksfINA2XXRegAdcConfig = 0x01;       ///< ADC Configuration register (16-bit, R/W)
const uint8_t ksfINA2XXRegShuntCal = 0x02;        ///< Shunt Calibration register (16-bit, R/W)
const uint8_t ksfINA2XXRegShuntTempCo = 0x03;     ///< Shunt Temperature Coefficient (16-bit, R/W)
const uint8_t ksfINA2XXRegVShunt = 0x04;          ///< Shunt Voltage Measurement (24-bit INA228 / 16-bit INA237, R)
const uint8_t ksfINA2XXRegVBus = 0x05;            ///< Bus Voltage Measurement (24-bit INA228 / 16-bit INA237, R)
const uint8_t ksfINA2XXRegDieTemp = 0x06;         ///< Die Temperature Measurement (16-bit, R)
const uint8_t ksfINA2XXRegCurrent = 0x07;         ///< Current Result (24-bit INA228 / 16-bit INA237, R)
const uint8_t ksfINA2XXRegPower = 0x08;           ///< Power Result (24-bit, R)
const uint8_t ksfINA2XXRegEnergy = 0x09;          ///< Energy Result (40-bit, R) -- INA228 only
const uint8_t ksfINA2XXRegCharge = 0x0A;          ///< Charge Result (40-bit, R) -- INA228 only
const uint8_t ksfINA2XXRegDiagAlrt = 0x0B;        ///< Diagnostic Flags and Alert (16-bit, R/W)
const uint8_t ksfINA2XXRegSOVL = 0x0C;            ///< Shunt Overvoltage Threshold (16-bit, R/W)
const uint8_t ksfINA2XXRegSUVL = 0x0D;            ///< Shunt Undervoltage Threshold (16-bit, R/W)
const uint8_t ksfINA2XXRegBOVL = 0x0E;            ///< Bus Overvoltage Threshold (16-bit, R/W)
const uint8_t ksfINA2XXRegBUVL = 0x0F;            ///< Bus Undervoltage Threshold (16-bit, R/W)
const uint8_t ksfINA2XXRegTempLimit = 0x10;       ///< Temperature Over-Limit Threshold (16-bit, R/W)
const uint8_t ksfINA2XXRegPowerLimit = 0x11;      ///< Power Over-Limit Threshold (16-bit, R/W)
const uint8_t ksfINA2XXRegManufacturerID = 0x3E;  ///< Manufacturer ID (16-bit, R) -- reads 0x5449 ("TI")
const uint8_t ksfINA2XXRegDeviceID = 0x3F;        ///< Device ID (16-bit, R)

///////////////////////////////////////////////////////////////////////////////
// CONFIG Register (0x00) Bit Definitions
///////////////////////////////////////////////////////////////////////////////
const uint16_t ksfINA2XXConfigRst = (1U << 15);          ///< System reset (self-clearing)
const uint16_t ksfINA2XXConfigRstAcc = (1U << 14);       ///< Reset energy/charge accumulators (INA228)
const uint16_t ksfINA2XXConfigConvDlyMask = 0x3FC0;      ///< CONVDLY field mask, bits [13:6]
const uint8_t ksfINA2XXConfigConvDlyShift = 6;            ///< CONVDLY field shift
const uint16_t ksfINA2XXConfigTempComp = (1U << 5);      ///< Shunt temperature compensation enable
const uint16_t ksfINA2XXConfigAdcRange = (1U << 4);      ///< ADC range: 0 = +/-163.84mV, 1 = +/-40.96mV

///////////////////////////////////////////////////////////////////////////////
// ADC_CONFIG Register (0x01) Bit Definitions
///////////////////////////////////////////////////////////////////////////////
const uint16_t ksfINA2XXAdcConfigModeMask = 0xF000;      ///< MODE field mask, bits [15:12]
const uint8_t ksfINA2XXAdcConfigModeShift = 12;           ///< MODE field shift
const uint16_t ksfINA2XXAdcConfigVBusCTMask = 0x0E00;    ///< VBUSCT field mask, bits [11:9]
const uint8_t ksfINA2XXAdcConfigVBusCTShift = 9;          ///< VBUSCT field shift
const uint16_t ksfINA2XXAdcConfigVShCTMask = 0x01C0;     ///< VSHCT field mask, bits [8:6]
const uint8_t ksfINA2XXAdcConfigVShCTShift = 6;           ///< VSHCT field shift
const uint16_t ksfINA2XXAdcConfigVTCTMask = 0x0038;      ///< VTCT field mask, bits [5:3]
const uint8_t ksfINA2XXAdcConfigVTCTShift = 3;            ///< VTCT field shift
const uint16_t ksfINA2XXAdcConfigAvgMask = 0x0007;       ///< AVG field mask, bits [2:0]

///////////////////////////////////////////////////////////////////////////////
// DIAG_ALRT Register (0x0B) Bit Definitions
///////////////////////////////////////////////////////////////////////////////
const uint16_t ksfINA2XXDiagAlatch = (1U << 15);         ///< Alert latch enable
const uint16_t ksfINA2XXDiagCnvr = (1U << 14);           ///< Conversion ready flag on ALERT pin
const uint16_t ksfINA2XXDiagSlowAlert = (1U << 13);      ///< Alert on averaged value
const uint16_t ksfINA2XXDiagApol = (1U << 12);           ///< Alert polarity (0=active-low, 1=active-high)
const uint16_t ksfINA2XXDiagEnergyOF = (1U << 11);       ///< Energy register overflow (INA228 only)
const uint16_t ksfINA2XXDiagChargeOF = (1U << 10);       ///< Charge register overflow (INA228 only)
const uint16_t ksfINA2XXDiagMathOF = (1U << 9);          ///< Math overflow
const uint16_t ksfINA2XXDiagTmpOL = (1U << 7);           ///< Temperature over-limit
const uint16_t ksfINA2XXDiagShntOL = (1U << 6);          ///< Shunt overvoltage
const uint16_t ksfINA2XXDiagShntUL = (1U << 5);          ///< Shunt undervoltage
const uint16_t ksfINA2XXDiagBusOL = (1U << 4);           ///< Bus overvoltage
const uint16_t ksfINA2XXDiagBusUL = (1U << 3);           ///< Bus undervoltage
const uint16_t ksfINA2XXDiagPOL = (1U << 2);             ///< Power over-limit
const uint16_t ksfINA2XXDiagCnvrf = (1U << 1);           ///< Conversion ready flag
const uint16_t ksfINA2XXDiagMemStat = (1U << 0);         ///< Memory checksum (1 = OK)

///////////////////////////////////////////////////////////////////////////////
// SHUNT_TEMPCO Register (0x03) Mask
///////////////////////////////////////////////////////////////////////////////
const uint16_t ksfINA2XXShuntTempCoMask = 0x3FFF;        ///< TEMPCO field mask, bits [13:0]

///////////////////////////////////////////////////////////////////////////////
// Device ID Constants
///////////////////////////////////////////////////////////////////////////////
const uint16_t ksfINA228DeviceIDValue = 0x2280;          ///< INA228 device ID (upper 12 bits)
const uint16_t ksfINA237DeviceIDValue = 0x2370;          ///< INA237 device ID (upper 12 bits)
const uint16_t ksfINA2XXDeviceIDMask = 0xFFF0;           ///< Mask for device ID (ignore revision)
const uint16_t ksfINA2XXManufacturerIDValue = 0x5449;    ///< "TI" in ASCII

///////////////////////////////////////////////////////////////////////////////
// ADC Operating Mode Enumeration
///////////////////////////////////////////////////////////////////////////////
/// @brief ADC operating mode settings for the MODE field in ADC_CONFIG.
/// Modes 0x0 and 0x8 are both shutdown. Modes 0x1-0x7 are triggered (single-shot).
/// Modes 0x9-0xF are continuous.
enum sfe_ina2xx_mode_t : uint8_t
{
    INA2XX_MODE_SHUTDOWN = 0x0,                     ///< Shutdown
    INA2XX_MODE_TRIG_BUS = 0x1,                     ///< Triggered bus voltage, single shot
    INA2XX_MODE_TRIG_SHUNT = 0x2,                   ///< Triggered shunt voltage, single shot
    INA2XX_MODE_TRIG_BUS_SHUNT = 0x3,               ///< Triggered bus + shunt, single shot
    INA2XX_MODE_TRIG_TEMP = 0x4,                    ///< Triggered temperature, single shot
    INA2XX_MODE_TRIG_TEMP_BUS = 0x5,                ///< Triggered temp + bus, single shot
    INA2XX_MODE_TRIG_TEMP_SHUNT = 0x6,              ///< Triggered temp + shunt, single shot
    INA2XX_MODE_TRIG_ALL = 0x7,                     ///< Triggered bus + shunt + temp, single shot
    INA2XX_MODE_SHUTDOWN2 = 0x8,                    ///< Shutdown (alternate)
    INA2XX_MODE_CONT_BUS = 0x9,                     ///< Continuous bus voltage only
    INA2XX_MODE_CONT_SHUNT = 0xA,                   ///< Continuous shunt voltage only
    INA2XX_MODE_CONT_BUS_SHUNT = 0xB,               ///< Continuous bus + shunt
    INA2XX_MODE_CONT_TEMP = 0xC,                    ///< Continuous temperature only
    INA2XX_MODE_CONT_TEMP_BUS = 0xD,                ///< Continuous temp + bus
    INA2XX_MODE_CONT_TEMP_SHUNT = 0xE,              ///< Continuous temp + shunt
    INA2XX_MODE_CONT_ALL = 0xF                      ///< Continuous bus + shunt + temp (default)
};

///////////////////////////////////////////////////////////////////////////////
// Conversion Time Enumeration
///////////////////////////////////////////////////////////////////////////////
/// @brief Conversion time settings shared by VBUSCT, VSHCT, and VTCT fields.
enum sfe_ina2xx_conv_time_t : uint8_t
{
    INA2XX_CONV_50US = 0,    ///< 50 us
    INA2XX_CONV_84US = 1,    ///< 84 us
    INA2XX_CONV_150US = 2,   ///< 150 us
    INA2XX_CONV_280US = 3,   ///< 280 us
    INA2XX_CONV_540US = 4,   ///< 540 us
    INA2XX_CONV_1052US = 5,  ///< 1052 us (default)
    INA2XX_CONV_2074US = 6,  ///< 2074 us
    INA2XX_CONV_4120US = 7   ///< 4120 us
};

///////////////////////////////////////////////////////////////////////////////
// Averaging Count Enumeration
///////////////////////////////////////////////////////////////////////////////
/// @brief ADC averaging count settings for the AVG field in ADC_CONFIG.
enum sfe_ina2xx_avg_count_t : uint8_t
{
    INA2XX_AVG_1 = 0,      ///< 1 sample (no averaging, default)
    INA2XX_AVG_4 = 1,      ///< 4 samples
    INA2XX_AVG_16 = 2,     ///< 16 samples
    INA2XX_AVG_64 = 3,     ///< 64 samples
    INA2XX_AVG_128 = 4,    ///< 128 samples
    INA2XX_AVG_256 = 5,    ///< 256 samples
    INA2XX_AVG_512 = 6,    ///< 512 samples
    INA2XX_AVG_1024 = 7    ///< 1024 samples
};

///////////////////////////////////////////////////////////////////////////////
// Class Declaration
///////////////////////////////////////////////////////////////////////////////

/// @brief Platform-independent base driver for the INA228/INA237 power monitor ICs.
///
/// @details This class implements register-level access for all configuration and diagnostic
/// registers shared between the INA228 and INA237. Measurement registers (VSHUNT, VBUS,
/// CURRENT, POWER, ENERGY, CHARGE) are implemented in the derived sfDevINA228 and sfDevINA237
/// classes because they differ in register width and scaling.
///
/// This class does not depend on Arduino. The Arduino-specific wrappers (SfeINA228ArdI2C,
/// SfeINA237ArdI2C) provide begin() and isConnected() methods.
class sfDevINA2XX
{
  public:
    sfDevINA2XX() : _theBus{nullptr}, _currentLSB{0.0f}, _adcRange{false}, _shuntRes{0.0f}
    {
    }

    /// @brief Initialize the device driver with the given bus.
    /// @details Sets the communication bus and configures byte order for big-endian.
    /// @param theBus Pointer to the initialized bus object.
    /// @return ksfTkErrOk on success, or an error code on failure.
    bool begin(sfTkIBus *theBus = nullptr);

    /// @brief Set the communication bus pointer.
    /// @param theBus Bus to use for all register I/O.
    void setCommunicationBus(sfTkIBus *theBus);

    // ========================= Identity & Reset =============================

    /// @brief Read the Manufacturer ID register (0x3E).
    /// @return Manufacturer ID (0x5449 = "TI"), or 0 on error.
    uint16_t getManufacturerID(void);

    /// @brief Read the Device ID register (0x3F).
    /// @return Full 16-bit device ID (including revision nibble), or 0 on error.
    uint16_t getDeviceID(void);

    /// @brief Perform a full system reset via the RST bit in CONFIG.
    /// @return True on success, false on error.
    bool reset(void);

    /// @brief Reset the energy and charge accumulators (INA228 RSTACC bit).
    /// @return True on success, false on error.
    bool resetAccumulators(void);

    // ========================= CONFIG Register (0x00) =======================

    /// @brief Set the ADC full-scale shunt range.
    /// @param reducedRange False = +/-163.84mV (default), true = +/-40.96mV.
    /// @return True on success, false on error.
    bool setADCRange(bool reducedRange);

    /// @brief Get the current ADC range setting.
    /// @return True if reduced range (+/-40.96mV), false if default (+/-163.84mV).
    bool getADCRange(void);

    /// @brief Set the initial conversion delay in 2ms steps.
    /// @param delay2ms Delay value 0-255 (0 = 0ms, 255 = 510ms).
    /// @return True on success, false on error.
    bool setConversionDelay(uint8_t delay2ms);

    /// @brief Get the current conversion delay setting.
    /// @return Delay value 0-255, or 0 on error.
    uint8_t getConversionDelay(void);

    /// @brief Enable or disable shunt temperature compensation.
    /// @param enable True to enable, false to disable.
    /// @return True on success, false on error.
    bool enableTempCompensation(bool enable);

    /// @brief Get the temperature compensation enable state.
    /// @return True if enabled, false if disabled (or on error).
    bool getTempCompensation(void);

    // ====================== ADC_CONFIG Register (0x01) ======================

    /// @brief Set the ADC operating mode.
    /// @param mode Operating mode (see sfe_ina2xx_mode_t).
    /// @return True on success, false on error.
    bool setADCMode(sfe_ina2xx_mode_t mode);

    /// @brief Get the current ADC operating mode.
    /// @return Operating mode, or INA2XX_MODE_CONT_ALL on error.
    sfe_ina2xx_mode_t getADCMode(void);

    /// @brief Set the bus voltage conversion time.
    /// @param time Conversion time setting (see sfe_ina2xx_conv_time_t).
    /// @return True on success, false on error.
    bool setBusVoltageConvTime(sfe_ina2xx_conv_time_t time);

    /// @brief Get the bus voltage conversion time.
    /// @return Conversion time setting.
    sfe_ina2xx_conv_time_t getBusVoltageConvTime(void);

    /// @brief Set the shunt voltage conversion time.
    /// @param time Conversion time setting (see sfe_ina2xx_conv_time_t).
    /// @return True on success, false on error.
    bool setShuntVoltageConvTime(sfe_ina2xx_conv_time_t time);

    /// @brief Get the shunt voltage conversion time.
    /// @return Conversion time setting.
    sfe_ina2xx_conv_time_t getShuntVoltageConvTime(void);

    /// @brief Set the temperature conversion time.
    /// @param time Conversion time setting (see sfe_ina2xx_conv_time_t).
    /// @return True on success, false on error.
    bool setTempConvTime(sfe_ina2xx_conv_time_t time);

    /// @brief Get the temperature conversion time.
    /// @return Conversion time setting.
    sfe_ina2xx_conv_time_t getTempConvTime(void);

    /// @brief Set the ADC averaging count.
    /// @param count Averaging count (see sfe_ina2xx_avg_count_t).
    /// @return True on success, false on error.
    bool setAveragingCount(sfe_ina2xx_avg_count_t count);

    /// @brief Get the current ADC averaging count.
    /// @return Averaging count setting.
    sfe_ina2xx_avg_count_t getAveragingCount(void);

    // ================== Calibration Registers (0x02-0x03) ===================

    /// @brief Write a raw value to the SHUNT_CAL register.
    /// @param calValue 15-bit calibration value (bit 15 is reserved).
    /// @return True on success, false on error.
    bool setShuntCal(uint16_t calValue);

    /// @brief Read the current SHUNT_CAL register value.
    /// @return Calibration value, or 0 on error.
    uint16_t getShuntCal(void);

    /// @brief Set the shunt temperature coefficient for temperature compensation.
    /// @param ppmPerDegC Temperature coefficient, 0-16383 ppm/deg-C.
    /// @return True on success, false on error.
    bool setShuntTempCoefficient(uint16_t ppmPerDegC);

    /// @brief Get the shunt temperature coefficient.
    /// @return Temperature coefficient in ppm/deg-C, or 0 on error.
    uint16_t getShuntTempCoefficient(void);

    // ================== Diagnostics & Alert (0x0B) ==========================

    /// @brief Read the full DIAG_ALRT register.
    /// @details Reading this register clears latched alert flags when ALATCH = 1.
    /// @return Raw 16-bit DIAG_ALRT value, or 0 on error.
    uint16_t getDiagnosticFlags(void);

    /// @brief Enable or disable alert latching.
    /// @param latched True = latched mode, false = transparent mode.
    /// @return True on success, false on error.
    bool setAlertLatch(bool latched);

    /// @brief Get the alert latch setting.
    /// @return True if latched, false if transparent.
    bool getAlertLatch(void);

    /// @brief Enable conversion ready flag on the ALERT pin.
    /// @param enable True to assert ALERT on conversion complete.
    /// @return True on success, false on error.
    bool setConversionReadyAlert(bool enable);

    /// @brief Get the conversion ready alert enable state.
    /// @return True if enabled.
    bool getConversionReadyAlert(void);

    /// @brief Enable alert comparison on averaged (vs. instantaneous) values.
    /// @param enable True = alert on averaged value, false = alert on ADC value.
    /// @return True on success, false on error.
    bool setSlowAlert(bool enable);

    /// @brief Get the slow alert enable state.
    /// @return True if alert compares on averaged value.
    bool getSlowAlert(void);

    /// @brief Set the ALERT pin polarity.
    /// @param activeHigh True = active-high, false = active-low (default, open-drain).
    /// @return True on success, false on error.
    bool setAlertPolarity(bool activeHigh);

    /// @brief Get the ALERT pin polarity setting.
    /// @return True if active-high, false if active-low.
    bool getAlertPolarity(void);

    /// @brief Check if the energy register has overflowed (INA228 only).
    /// @return True if overflow detected.
    bool isEnergyOverflow(void);

    /// @brief Check if the charge register has overflowed (INA228 only).
    /// @return True if overflow detected.
    bool isChargeOverflow(void);

    /// @brief Check if a math overflow has occurred in current/power calculations.
    /// @return True if overflow detected.
    bool isMathOverflow(void);

    /// @brief Check if the temperature over-limit flag is set.
    /// @return True if temperature exceeds TEMP_LIMIT threshold.
    bool isTempOverLimit(void);

    /// @brief Check if the shunt overvoltage flag is set.
    /// @return True if shunt voltage exceeds SOVL threshold.
    bool isShuntOverVoltage(void);

    /// @brief Check if the shunt undervoltage flag is set.
    /// @return True if shunt voltage is below SUVL threshold.
    bool isShuntUnderVoltage(void);

    /// @brief Check if the bus overvoltage flag is set.
    /// @return True if bus voltage exceeds BOVL threshold.
    bool isBusOverVoltage(void);

    /// @brief Check if the bus undervoltage flag is set.
    /// @return True if bus voltage is below BUVL threshold.
    bool isBusUnderVoltage(void);

    /// @brief Check if the power over-limit flag is set.
    /// @return True if power exceeds PWR_LIMIT threshold.
    bool isPowerOverLimit(void);

    /// @brief Check if a conversion has completed.
    /// @return True if conversion ready.
    bool isConversionReady(void);

    /// @brief Check the memory checksum status.
    /// @return True if internal memory checksum is valid.
    bool isMemoryValid(void);

    // ==================== Threshold Registers (0x0C-0x11) ===================

    /// @brief Set the shunt overvoltage threshold.
    /// @param threshold Two's complement value. LSB = 5uV (ADCRANGE=0) or 1.25uV (ADCRANGE=1).
    /// @return True on success, false on error.
    bool setShuntOverVoltageThreshold(int16_t threshold);

    /// @brief Get the shunt overvoltage threshold.
    /// @return Threshold value, or 0 on error.
    int16_t getShuntOverVoltageThreshold(void);

    /// @brief Set the shunt undervoltage threshold.
    /// @param threshold Two's complement value. Same LSB as shunt overvoltage.
    /// @return True on success, false on error.
    bool setShuntUnderVoltageThreshold(int16_t threshold);

    /// @brief Get the shunt undervoltage threshold.
    /// @return Threshold value, or 0 on error.
    int16_t getShuntUnderVoltageThreshold(void);

    /// @brief Set the bus overvoltage threshold.
    /// @param threshold Unsigned 15-bit value. LSB = 3.125mV.
    /// @return True on success, false on error.
    bool setBusOverVoltageThreshold(uint16_t threshold);

    /// @brief Get the bus overvoltage threshold.
    /// @return Threshold value, or 0 on error.
    uint16_t getBusOverVoltageThreshold(void);

    /// @brief Set the bus undervoltage threshold.
    /// @param threshold Unsigned 15-bit value. LSB = 3.125mV.
    /// @return True on success, false on error.
    bool setBusUnderVoltageThreshold(uint16_t threshold);

    /// @brief Get the bus undervoltage threshold.
    /// @return Threshold value, or 0 on error.
    uint16_t getBusUnderVoltageThreshold(void);

    /// @brief Set the temperature over-limit threshold.
    /// @param threshold Two's complement value. LSB = 7.8125 m-deg-C.
    /// @return True on success, false on error.
    bool setTempLimitThreshold(int16_t threshold);

    /// @brief Get the temperature over-limit threshold.
    /// @return Threshold value, or 0 on error.
    int16_t getTempLimitThreshold(void);

    /// @brief Set the power over-limit threshold.
    /// @param threshold Unsigned value. LSB = 256 x Power LSB.
    /// @return True on success, false on error.
    bool setPowerLimitThreshold(uint16_t threshold);

    /// @brief Get the power over-limit threshold.
    /// @return Threshold value, or 0 on error.
    uint16_t getPowerLimitThreshold(void);

  protected:
    sfTkIBus *_theBus;    ///< Pointer to the communication bus device.
    float _currentLSB;    ///< Current measurement LSB in Amps, set during calibration.
    bool _adcRange;       ///< Cached ADC range: false = +/-163.84mV, true = +/-40.96mV.
    float _shuntRes;      ///< Shunt resistance in Ohms, stored during calibration.

    /// @brief Helper to read a DIAG_ALRT bit without clearing latched flags.
    /// @details Reads the full register and checks the specified bit.
    /// @param bitMask The bit to check.
    /// @return True if the bit is set.
    bool _getDiagBit(uint16_t bitMask);

    /// @brief Helper to set or clear a single bit in a 16-bit register.
    /// @param reg Register address.
    /// @param bitMask The bit to modify.
    /// @param set True to set the bit, false to clear it.
    /// @return True on success, false on error.
    bool _setRegisterBit(uint8_t reg, uint16_t bitMask, bool set);

    /// @brief Helper to get a single bit from a 16-bit register.
    /// @param reg Register address.
    /// @param bitMask The bit to check.
    /// @return True if the bit is set, false otherwise.
    bool _getRegisterBit(uint8_t reg, uint16_t bitMask);

    /// @brief Helper to set a multi-bit field in a 16-bit register.
    /// @param reg Register address.
    /// @param mask Field mask.
    /// @param shift Field bit shift.
    /// @param value Value to set in the field.
    /// @return True on success, false on error.
    bool _setRegisterField(uint8_t reg, uint16_t mask, uint8_t shift, uint8_t value);

    /// @brief Helper to get a multi-bit field from a 16-bit register.
    /// @param reg Register address.
    /// @param mask Field mask.
    /// @param shift Field bit shift.
    /// @return Field value, or 0 on error.
    uint8_t _getRegisterField(uint8_t reg, uint16_t mask, uint8_t shift);

    /// @brief Read 3 bytes (24-bit) from a register into a uint32_t.
    /// @param reg Register address.
    /// @param value Output value (MSB-first assembly).
    /// @return ksfTkErrOk on success.
    sfTkError_t _readRegister24(uint8_t reg, uint32_t &value);

    /// @brief Read 5 bytes (40-bit) from a register into a uint64_t.
    /// @param reg Register address.
    /// @param value Output value (MSB-first assembly).
    /// @return ksfTkErrOk on success.
    sfTkError_t _readRegister40(uint8_t reg, uint64_t &value);
};
