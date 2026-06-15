/**
 * @file sfDevINA2XX.h
 * @brief Header file for the SparkFun INA2XX family base driver.
 *
 * @details
 * sfDevINA2XX is the platform-independent base class for the Texas Instruments INA228 and INA237
 * digital power/energy monitor ICs. It is a class template parameterized by the two raw register
 * types that differ between the devices:
 *
 *   - @c signed_raw_t   — type returned by raw shunt-voltage and current reads (signed).
 *   - @c unsigned_raw_t — type returned by raw bus-voltage reads (unsigned).
 *
 * The INA228 instantiates the template with @c <int32_t, uint32_t> (20-bit values held in 32-bit
 * words); the INA237 with @c <int16_t, uint16_t> (plain 16-bit values). See sfDevINA228.h and
 * sfDevINA237.h.
 *
 * The class provides register access for all configuration and diagnostic registers shared
 * between both devices:
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
 * It also implements calibration, the engineering-unit measurement reads (VSHUNT, VBUS, CURRENT,
 * POWER, DIETEMP), and the device-identity check (isConnected). These differ between the two ICs
 * only in register width, LSB scaling, and expected DEVICE_ID; the width is selected by the
 * template types plus the @c is24BitMeasurements flag, and the scaling values and device IDs are
 * supplied by the derived device class through the constructor. INA228-only features (ENERGY,
 * CHARGE) live in the derived sfDevINA228 class.
 *
 * Both devices use 8-bit register addresses and big-endian byte order over I2C.
 *
 * The class is a template, but its member functions are *not* defined inline in this header.
 * They are defined out-of-line in sfDevINA2XX.cpp and explicitly instantiated there for the two
 * type combinations the library uses. The @c extern @c template declarations at the bottom of
 * this header tell every other translation unit to use those instantiations rather than generate
 * its own, so the template still compiles exactly once. Adding a new device means adding its
 * @c <signed, unsigned> pair to both the @c extern declarations here and the explicit
 * instantiations in the .cpp.
 *
 * Most methods return a SparkFun Toolkit error code (::ksfTkErrOk on success, a negative value on
 * error); values read from the device are returned through reference (output) parameters. Callers
 * that do not care about error handling can simply ignore the returned code.
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

#include <stdint.h>

// SparkFun Toolkit core headers
#include <sfTk/sfToolkit.h>
#include <sfTk/sfTkII2C.h>

///////////////////////////////////////////////////////////////////////////////
// ADC Operating Mode Enumeration
///////////////////////////////////////////////////////////////////////////////
/// @brief ADC operating mode settings for the MODE field in ADC_CONFIG (bits [15:12]).
/// @details Modes 0x0 and 0x8 are both shutdown. Modes 0x1-0x7 are triggered (single-shot).
/// Modes 0x9-0xF are continuous. A typedef is provided so the type can be used without the
/// @c enum keyword in environments where that matters.
typedef enum sfe_ina2xx_mode_t : uint8_t
{
    INA2XX_MODE_SHUTDOWN = 0x0,       ///< Shutdown
    INA2XX_MODE_TRIG_BUS = 0x1,       ///< Triggered bus voltage, single shot
    INA2XX_MODE_TRIG_SHUNT = 0x2,     ///< Triggered shunt voltage, single shot
    INA2XX_MODE_TRIG_BUS_SHUNT = 0x3, ///< Triggered bus + shunt, single shot
    INA2XX_MODE_TRIG_TEMP = 0x4,      ///< Triggered temperature, single shot
    INA2XX_MODE_TRIG_TEMP_BUS = 0x5,  ///< Triggered temp + bus, single shot
    INA2XX_MODE_TRIG_TEMP_SHUNT = 0x6, ///< Triggered temp + shunt, single shot
    INA2XX_MODE_TRIG_ALL = 0x7,       ///< Triggered bus + shunt + temp, single shot
    INA2XX_MODE_SHUTDOWN2 = 0x8,      ///< Shutdown (alternate)
    INA2XX_MODE_CONT_BUS = 0x9,       ///< Continuous bus voltage only
    INA2XX_MODE_CONT_SHUNT = 0xA,     ///< Continuous shunt voltage only
    INA2XX_MODE_CONT_BUS_SHUNT = 0xB, ///< Continuous bus + shunt
    INA2XX_MODE_CONT_TEMP = 0xC,      ///< Continuous temperature only
    INA2XX_MODE_CONT_TEMP_BUS = 0xD,  ///< Continuous temp + bus
    INA2XX_MODE_CONT_TEMP_SHUNT = 0xE, ///< Continuous temp + shunt
    INA2XX_MODE_CONT_ALL = 0xF        ///< Continuous bus + shunt + temp (default)
} sfe_ina2xx_mode_t;

///////////////////////////////////////////////////////////////////////////////
// Conversion Time Enumeration
///////////////////////////////////////////////////////////////////////////////
/// @brief Conversion time settings shared by the VBUSCT, VSHCT, and VTCT fields.
typedef enum sfe_ina2xx_conv_time_t : uint8_t
{
    INA2XX_CONV_50US = 0,   ///< 50 us
    INA2XX_CONV_84US = 1,   ///< 84 us
    INA2XX_CONV_150US = 2,  ///< 150 us
    INA2XX_CONV_280US = 3,  ///< 280 us
    INA2XX_CONV_540US = 4,  ///< 540 us
    INA2XX_CONV_1052US = 5, ///< 1052 us (default)
    INA2XX_CONV_2074US = 6, ///< 2074 us
    INA2XX_CONV_4120US = 7  ///< 4120 us
} sfe_ina2xx_conv_time_t;

///////////////////////////////////////////////////////////////////////////////
// Averaging Count Enumeration
///////////////////////////////////////////////////////////////////////////////
/// @brief ADC averaging count settings for the AVG field in ADC_CONFIG (bits [2:0]).
typedef enum sfe_ina2xx_avg_count_t : uint8_t
{
    INA2XX_AVG_1 = 0,    ///< 1 sample (no averaging, default)
    INA2XX_AVG_4 = 1,    ///< 4 samples
    INA2XX_AVG_16 = 2,   ///< 16 samples
    INA2XX_AVG_64 = 3,   ///< 64 samples
    INA2XX_AVG_128 = 4,  ///< 128 samples
    INA2XX_AVG_256 = 5,  ///< 256 samples
    INA2XX_AVG_512 = 6,  ///< 512 samples
    INA2XX_AVG_1024 = 7  ///< 1024 samples
} sfe_ina2xx_avg_count_t;

///////////////////////////////////////////////////////////////////////////////
// CONFIG Register Bitfield (Address 0x00)
///////////////////////////////////////////////////////////////////////////////
/// @brief Bitfield union for the 16-bit CONFIG register (0x00).
union sfe_ina2xx_config_reg_t
{
    struct
    {
        uint16_t : 4;          ///< Reserved [bits 3:0]
        uint16_t adcRange : 1; ///< ADC range: 0 = +/-163.84mV, 1 = +/-40.96mV [bit 4]
        uint16_t tempComp : 1; ///< Shunt temperature compensation enable [bit 5]
        uint16_t convDly : 8;  ///< Initial conversion delay, 2ms steps [bits 13:6]
        uint16_t rstAcc : 1;   ///< Reset ENERGY/CHARGE accumulators (INA228) [bit 14]
        uint16_t rst : 1;      ///< System reset (self-clearing) [bit 15]
    };
    uint16_t word; ///< Raw register value for I2C read/write.
};

///////////////////////////////////////////////////////////////////////////////
// ADC_CONFIG Register Bitfield (Address 0x01)
///////////////////////////////////////////////////////////////////////////////
/// @brief Bitfield union for the 16-bit ADC_CONFIG register (0x01).
union sfe_ina2xx_adc_config_reg_t
{
    struct
    {
        uint16_t avg : 3;    ///< ADC sample averaging count (see sfe_ina2xx_avg_count_t) [bits 2:0]
        uint16_t vtct : 3;   ///< Temperature conversion time (see sfe_ina2xx_conv_time_t) [bits 5:3]
        uint16_t vshct : 3;  ///< Shunt voltage conversion time [bits 8:6]
        uint16_t vbusct : 3; ///< Bus voltage conversion time [bits 11:9]
        uint16_t mode : 4;   ///< Operating mode (see sfe_ina2xx_mode_t) [bits 15:12]
    };
    uint16_t word; ///< Raw register value for I2C read/write.
};

///////////////////////////////////////////////////////////////////////////////
// DIAG_ALRT Register Bitfield (Address 0x0B)
///////////////////////////////////////////////////////////////////////////////
/// @brief Bitfield union for the 16-bit DIAG_ALRT diagnostic flags and alert register (0x0B).
/// @details Reading this register clears latched alert flags when ALATCH = 1.
union sfe_ina2xx_diag_alrt_reg_t
{
    struct
    {
        uint16_t memStat : 1;   ///< Memory checksum status (1 = OK) [bit 0]
        uint16_t cnvrf : 1;     ///< Conversion ready flag [bit 1]
        uint16_t pol : 1;       ///< Power over-limit [bit 2]
        uint16_t busUL : 1;     ///< Bus undervoltage [bit 3]
        uint16_t busOL : 1;     ///< Bus overvoltage [bit 4]
        uint16_t shntUL : 1;    ///< Shunt undervoltage [bit 5]
        uint16_t shntOL : 1;    ///< Shunt overvoltage [bit 6]
        uint16_t tmpOL : 1;     ///< Temperature over-limit [bit 7]
        uint16_t : 1;           ///< Reserved [bit 8]
        uint16_t mathOF : 1;    ///< Math overflow [bit 9]
        uint16_t chargeOF : 1;  ///< Charge register overflow (INA228 only) [bit 10]
        uint16_t energyOF : 1;  ///< Energy register overflow (INA228 only) [bit 11]
        uint16_t apol : 1;      ///< Alert polarity (0=active-low, 1=active-high) [bit 12]
        uint16_t slowAlert : 1; ///< Alert on averaged value [bit 13]
        uint16_t cnvr : 1;      ///< Conversion-ready flag on ALERT pin [bit 14]
        uint16_t alatch : 1;    ///< Alert latch enable [bit 15]
    };
    uint16_t word; ///< Raw register value for I2C read/write.
};

///////////////////////////////////////////////////////////////////////////////
// Class Declaration
///////////////////////////////////////////////////////////////////////////////

/// @brief Platform-independent base driver for the INA228/INA237 power monitor ICs.
///
/// @details This class template implements register-level access for all configuration and
/// diagnostic registers shared between the INA228 and INA237, plus calibration, the
/// engineering-unit measurement reads, and the device-identity check. It is parameterized by the
/// two raw register types that differ between the devices, and the per-device LSB scaling values
/// and expected DEVICE_ID(s) are supplied by the derived class through the constructor.
///
/// @tparam signed_raw_t   Type returned by raw shunt-voltage and current reads (e.g., int32_t).
/// @tparam unsigned_raw_t Type returned by raw bus-voltage reads (e.g., uint32_t).
///
/// Most methods return a SparkFun Toolkit error code (::ksfTkErrOk on success, a negative value
/// on error); values read from the device are returned through reference (output) parameters.
///
/// The member functions are defined and explicitly instantiated in sfDevINA2XX.cpp; see the file
/// header for details. This class does not depend on Arduino. The Arduino-specific wrappers
/// (SfeINA228ArdI2C, SfeINA237ArdI2C) provide begin() methods.
template <typename signed_raw_t, typename unsigned_raw_t>
class sfDevINA2XX
{
  public:
    /// @brief Construct the base driver with the device-specific scaling and identity configuration.
    /// @param is24BitMeasurements True if VSHUNT/VBUS/CURRENT are 24-bit registers with data in
    /// bits [23:4] (INA228); false for plain 16-bit registers (INA237).
    /// @param tempShift Right-shift applied to the raw DIETEMP word (0 on the INA228, 4 on the
    /// INA237 where data is in bits [15:4]).
    /// @param calScale Calibration scale constant used to compute SHUNT_CAL.
    /// @param shuntLSBDefault Shunt voltage LSB in volts at the default ADC range (ADCRANGE = 0).
    /// @param shuntLSBReduced Shunt voltage LSB in volts at the reduced ADC range (ADCRANGE = 1).
    /// @param busLSB Bus voltage LSB in volts.
    /// @param tempLSB Die temperature LSB in deg-C.
    /// @param currentFullScale Positive half of the ADC range, used to derive CURRENT_LSB.
    /// @param powerLSBScale Power LSB scale factor (Power LSB = powerLSBScale x CURRENT_LSB).
    /// @param deviceID Expected DEVICE_ID (upper 12 bits) for the identity check.
    /// @param deviceIDAlt Alternate accepted DEVICE_ID for a register-compatible part; pass the
    /// same value as @p deviceID if the device has no compatible alternate.
    sfDevINA2XX(bool is24BitMeasurements, uint8_t tempShift, float calScale, float shuntLSBDefault,
                float shuntLSBReduced, float busLSB, float tempLSB, float currentFullScale,
                float powerLSBScale, uint16_t deviceID, uint16_t deviceIDAlt)
        : _theBus{nullptr}, _currentLSB{0.0f}, _adcRange{false}, _shuntRes{0.0f},
          _is24BitMeasurements{is24BitMeasurements}, _tempShift{tempShift}, _calScale{calScale},
          _shuntLSBDefault{shuntLSBDefault}, _shuntLSBReduced{shuntLSBReduced}, _busLSB{busLSB},
          _tempLSB{tempLSB}, _currentFullScale{currentFullScale}, _powerLSBScale{powerLSBScale},
          _deviceID{deviceID}, _deviceIDAlt{deviceIDAlt}
    {
    }

    /// @brief Initialize the device driver with the given bus.
    /// @details Sets the communication bus and configures byte order for big-endian.
    /// @param theBus Pointer to the initialized bus object. If null, a bus set by a prior call is used.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t begin(sfTkIBus *theBus = nullptr);

    /// @brief Set the communication bus pointer.
    /// @param theBus Bus to use for all register I/O.
    void setCommunicationBus(sfTkIBus *theBus);

    // ========================= Identity & Reset =============================

    /// @brief Check if the device is connected and responding.
    /// @details Reads the DEVICE_ID register and checks that the upper 12 bits match the expected
    /// device ID (or the alternate register-compatible ID) supplied at construction.
    /// @return true if the device responds and the identity check passes; false otherwise.
    bool isConnected(void);

    /// @brief Read the Manufacturer ID register (0x3E).
    /// @param id Output reference that receives the manufacturer ID (0x5449 = "TI").
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getManufacturerID(uint16_t &id);

    /// @brief Read the Device ID register (0x3F).
    /// @param id Output reference that receives the full 16-bit device ID (including revision nibble).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getDeviceID(uint16_t &id);

    /// @brief Perform a full system reset via the RST bit in CONFIG.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t reset(void);

    /// @brief Reset the energy and charge accumulators (INA228 RSTACC bit).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t resetAccumulators(void);

    // ========================= CONFIG Register (0x00) =======================

    /// @brief Set the ADC full-scale shunt range.
    /// @param reducedRange False = +/-163.84mV (default), true = +/-40.96mV.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setADCRange(bool reducedRange);

    /// @brief Get the current ADC range setting.
    /// @param reducedRange Output reference set true if reduced range (+/-40.96mV), false if default.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getADCRange(bool &reducedRange);

    /// @brief Set the initial conversion delay in 2ms steps.
    /// @param delay2ms Delay value 0-255 (0 = 0ms, 255 = 510ms).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setConversionDelay(uint8_t delay2ms);

    /// @brief Get the current conversion delay setting.
    /// @param delay2ms Output reference that receives the delay value 0-255.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getConversionDelay(uint8_t &delay2ms);

    /// @brief Enable or disable shunt temperature compensation.
    /// @param enable True to enable, false to disable.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t enableTempCompensation(bool enable);

    /// @brief Get the temperature compensation enable state.
    /// @param enabled Output reference set true if enabled.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getTempCompensation(bool &enabled);

    // ====================== ADC_CONFIG Register (0x01) ======================

    /// @brief Set the ADC operating mode.
    /// @param mode Operating mode (see sfe_ina2xx_mode_t).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setADCMode(sfe_ina2xx_mode_t mode);

    /// @brief Get the current ADC operating mode.
    /// @param mode Output reference that receives the operating mode.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getADCMode(sfe_ina2xx_mode_t &mode);

    /// @brief Set the bus voltage conversion time.
    /// @param time Conversion time setting (see sfe_ina2xx_conv_time_t).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setBusVoltageConvTime(sfe_ina2xx_conv_time_t time);

    /// @brief Get the bus voltage conversion time.
    /// @param time Output reference that receives the conversion time setting.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getBusVoltageConvTime(sfe_ina2xx_conv_time_t &time);

    /// @brief Set the shunt voltage conversion time.
    /// @param time Conversion time setting (see sfe_ina2xx_conv_time_t).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setShuntVoltageConvTime(sfe_ina2xx_conv_time_t time);

    /// @brief Get the shunt voltage conversion time.
    /// @param time Output reference that receives the conversion time setting.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getShuntVoltageConvTime(sfe_ina2xx_conv_time_t &time);

    /// @brief Set the temperature conversion time.
    /// @param time Conversion time setting (see sfe_ina2xx_conv_time_t).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setTempConvTime(sfe_ina2xx_conv_time_t time);

    /// @brief Get the temperature conversion time.
    /// @param time Output reference that receives the conversion time setting.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getTempConvTime(sfe_ina2xx_conv_time_t &time);

    /// @brief Set the ADC averaging count.
    /// @param count Averaging count (see sfe_ina2xx_avg_count_t).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setAveragingCount(sfe_ina2xx_avg_count_t count);

    /// @brief Get the current ADC averaging count.
    /// @param count Output reference that receives the averaging count setting.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getAveragingCount(sfe_ina2xx_avg_count_t &count);

    // ================== Calibration Registers (0x02-0x03) ===================

    /// @brief Calibrate the device for current and power measurements.
    /// @details Computes CURRENT_LSB from maxCurrent and the device ADC range, then
    /// calculates SHUNT_CAL = calScale * CURRENT_LSB * Rshunt (x4 if ADCRANGE=1).
    /// Stores _currentLSB and _shuntRes internally for engineering-unit conversions.
    /// @param shuntResOhms Shunt resistance in Ohms (e.g., 0.015 for 15 mOhm).
    /// @param maxCurrentA Maximum expected current in Amps.
    /// @return ::ksfTkErrOk on success, ::ksfTkErrFail for invalid arguments, or an error code on
    /// communication failure.
    sfTkError_t calibrate(float shuntResOhms, float maxCurrentA);

    /// @brief Write a raw value to the SHUNT_CAL register.
    /// @param calValue 15-bit calibration value (bit 15 is reserved).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setShuntCal(uint16_t calValue);

    /// @brief Read the current SHUNT_CAL register value.
    /// @param calValue Output reference that receives the calibration value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getShuntCal(uint16_t &calValue);

    /// @brief Set the shunt temperature coefficient for temperature compensation.
    /// @param ppmPerDegC Temperature coefficient, 0-16383 ppm/deg-C.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setShuntTempCoefficient(uint16_t ppmPerDegC);

    /// @brief Get the shunt temperature coefficient.
    /// @param ppmPerDegC Output reference that receives the temperature coefficient in ppm/deg-C.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getShuntTempCoefficient(uint16_t &ppmPerDegC);

    // ================== Measurements (Engineering Units) ====================

    /// @brief Read the shunt voltage in millivolts.
    /// @details Reads the VSHUNT register and scales by the device shunt LSB
    /// (selected by the current ADCRANGE setting).
    /// @param milliVolts Output reference that receives the shunt voltage in mV.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getShuntVoltage_mV(float &milliVolts);

    /// @brief Read the bus voltage in Volts.
    /// @details Reads the VBUS register and scales by the device bus voltage LSB.
    /// @param volts Output reference that receives the bus voltage in V.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getBusVoltage_V(float &volts);

    /// @brief Read the calculated current in Amps.
    /// @details Reads the CURRENT register (two's complement) and scales by
    /// CURRENT_LSB (set during calibrate()).
    /// @note calibrate() must be called before this method returns meaningful values.
    /// @param amps Output reference that receives the current in A.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getCurrent_A(float &amps);

    /// @brief Read the calculated power in Watts.
    /// @details Reads the 24-bit POWER register (unsigned) and scales by the device
    /// power LSB scale x CURRENT_LSB (set during calibrate()).
    /// @note calibrate() must be called before this method returns meaningful values.
    /// @param watts Output reference that receives the power in W.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getPower_W(float &watts);

    /// @brief Read the die temperature in degrees Celsius.
    /// @details Reads the 16-bit DIETEMP register (two's complement) and scales by the
    /// device temperature LSB. On devices where the data occupies only the upper bits
    /// (INA237: bits [15:4]), the reserved low bits are shifted out first.
    /// @param celsius Output reference that receives the temperature in deg-C.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getDieTemp_C(float &celsius);

    // ========================= Raw Register Access ===========================

    /// @brief Read the raw shunt voltage value.
    /// @param value Output reference that receives the signed raw value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getShuntVoltageRaw(signed_raw_t &value);

    /// @brief Read the raw bus voltage value.
    /// @param value Output reference that receives the unsigned raw value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getBusVoltageRaw(unsigned_raw_t &value);

    /// @brief Read the raw current value.
    /// @param value Output reference that receives the signed raw value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getCurrentRaw(signed_raw_t &value);

    /// @brief Read the raw 24-bit power value.
    /// @param value Output reference that receives the unsigned 24-bit value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getPowerRaw(uint32_t &value);

    // ================== Diagnostics & Alert (0x0B) ==========================

    /// @brief Read the full DIAG_ALRT register.
    /// @details Reading this register clears latched alert flags when ALATCH = 1.
    /// @param flags Output reference that receives the diagnostic flags bitfield.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getDiagnosticFlags(sfe_ina2xx_diag_alrt_reg_t &flags);

    /// @brief Enable or disable alert latching.
    /// @param latched True = latched mode, false = transparent mode.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setAlertLatch(bool latched);

    /// @brief Get the alert latch setting.
    /// @param latched Output reference set true if latched, false if transparent.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getAlertLatch(bool &latched);

    /// @brief Enable conversion ready flag on the ALERT pin.
    /// @param enable True to assert ALERT on conversion complete.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setConversionReadyAlert(bool enable);

    /// @brief Get the conversion ready alert enable state.
    /// @param enabled Output reference set true if enabled.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getConversionReadyAlert(bool &enabled);

    /// @brief Enable alert comparison on averaged (vs. instantaneous) values.
    /// @param enable True = alert on averaged value, false = alert on ADC value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setSlowAlert(bool enable);

    /// @brief Get the slow alert enable state.
    /// @param enabled Output reference set true if alert compares on averaged value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getSlowAlert(bool &enabled);

    /// @brief Set the ALERT pin polarity.
    /// @param activeHigh True = active-high, false = active-low (default, open-drain).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setAlertPolarity(bool activeHigh);

    /// @brief Get the ALERT pin polarity setting.
    /// @param activeHigh Output reference set true if active-high, false if active-low.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getAlertPolarity(bool &activeHigh);

    /// @brief Check if the energy register has overflowed (INA228 only).
    /// @param overflow Output reference set true if overflow detected.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t isEnergyOverflow(bool &overflow);

    /// @brief Check if the charge register has overflowed (INA228 only).
    /// @param overflow Output reference set true if overflow detected.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t isChargeOverflow(bool &overflow);

    /// @brief Check if a math overflow has occurred in current/power calculations.
    /// @param overflow Output reference set true if overflow detected.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t isMathOverflow(bool &overflow);

    /// @brief Check if the temperature over-limit flag is set.
    /// @param overLimit Output reference set true if temperature exceeds TEMP_LIMIT threshold.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t isTempOverLimit(bool &overLimit);

    /// @brief Check if the shunt overvoltage flag is set.
    /// @param overVoltage Output reference set true if shunt voltage exceeds SOVL threshold.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t isShuntOverVoltage(bool &overVoltage);

    /// @brief Check if the shunt undervoltage flag is set.
    /// @param underVoltage Output reference set true if shunt voltage is below SUVL threshold.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t isShuntUnderVoltage(bool &underVoltage);

    /// @brief Check if the bus overvoltage flag is set.
    /// @param overVoltage Output reference set true if bus voltage exceeds BOVL threshold.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t isBusOverVoltage(bool &overVoltage);

    /// @brief Check if the bus undervoltage flag is set.
    /// @param underVoltage Output reference set true if bus voltage is below BUVL threshold.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t isBusUnderVoltage(bool &underVoltage);

    /// @brief Check if the power over-limit flag is set.
    /// @param overLimit Output reference set true if power exceeds PWR_LIMIT threshold.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t isPowerOverLimit(bool &overLimit);

    /// @brief Check if a conversion has completed.
    /// @param ready Output reference set true if conversion ready.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t isConversionReady(bool &ready);

    /// @brief Check the memory checksum status.
    /// @param valid Output reference set true if the internal memory checksum is valid.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t isMemoryValid(bool &valid);

    // ==================== Threshold Registers (0x0C-0x11) ===================

    /// @brief Set the shunt overvoltage threshold.
    /// @param threshold Two's complement value. LSB = 5uV (ADCRANGE=0) or 1.25uV (ADCRANGE=1).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setShuntOverVoltageThreshold(int16_t threshold);

    /// @brief Set the shunt overvoltage threshold in millivolts.
    /// @details Converts millivolts to the SOVL register value using the threshold LSB
    /// (5 uV at the default ADC range, 1.25 uV at the reduced range). The current ADCRANGE
    /// setting is read from the device, so call setADCRange() before this method.
    /// Values beyond the register range are clamped.
    /// @param milliVolts Threshold in mV. Max +/-163.84 mV (default range) or +/-40.96 mV (reduced).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setShuntOverVoltageThreshold_mV(float milliVolts);

    /// @brief Get the shunt overvoltage threshold.
    /// @param threshold Output reference that receives the threshold value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getShuntOverVoltageThreshold(int16_t &threshold);

    /// @brief Set the shunt undervoltage threshold.
    /// @param threshold Two's complement value. Same LSB as shunt overvoltage.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setShuntUnderVoltageThreshold(int16_t threshold);

    /// @brief Get the shunt undervoltage threshold.
    /// @param threshold Output reference that receives the threshold value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getShuntUnderVoltageThreshold(int16_t &threshold);

    /// @brief Set the bus overvoltage threshold.
    /// @param threshold Unsigned 15-bit value. LSB = 3.125mV.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setBusOverVoltageThreshold(uint16_t threshold);

    /// @brief Set the bus overvoltage threshold in millivolts.
    /// @details Converts millivolts to the BOVL register value (LSB = 3.125 mV).
    /// Values beyond the register range (0 to ~102.4 V) are clamped.
    /// @param milliVolts Threshold in mV (e.g., 14000.0 for 14 V).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setBusOverVoltageThreshold_mV(float milliVolts);

    /// @brief Get the bus overvoltage threshold.
    /// @param threshold Output reference that receives the threshold value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getBusOverVoltageThreshold(uint16_t &threshold);

    /// @brief Set the bus undervoltage threshold.
    /// @param threshold Unsigned 15-bit value. LSB = 3.125mV.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setBusUnderVoltageThreshold(uint16_t threshold);

    /// @brief Get the bus undervoltage threshold.
    /// @param threshold Output reference that receives the threshold value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getBusUnderVoltageThreshold(uint16_t &threshold);

    /// @brief Set the temperature over-limit threshold.
    /// @param threshold Two's complement value. LSB = 7.8125 m-deg-C.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setTempLimitThreshold(int16_t threshold);

    /// @brief Get the temperature over-limit threshold.
    /// @param threshold Output reference that receives the threshold value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getTempLimitThreshold(int16_t &threshold);

    /// @brief Set the power over-limit threshold.
    /// @param threshold Unsigned value. LSB = 256 x Power LSB.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t setPowerLimitThreshold(uint16_t threshold);

    /// @brief Get the power over-limit threshold.
    /// @param threshold Output reference that receives the threshold value.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getPowerLimitThreshold(uint16_t &threshold);

  protected:
    sfTkIBus *_theBus; ///< Pointer to the communication bus device.
    float _currentLSB; ///< Current measurement LSB in Amps, set during calibration.
    bool _adcRange;    ///< Cached ADC range: false = +/-163.84mV, true = +/-40.96mV.
    float _shuntRes;   ///< Shunt resistance in Ohms, stored during calibration.

    // --- Per-device scaling and identity configuration, supplied by the derived class constructor ---
    bool _is24BitMeasurements; ///< True if VSHUNT/VBUS/CURRENT are 24-bit registers (data in [23:4]).
    uint8_t _tempShift;        ///< Right-shift applied to the raw DIETEMP word.
    float _calScale;           ///< Calibration scale constant.
    float _shuntLSBDefault;    ///< Shunt LSB in volts (ADCRANGE = 0).
    float _shuntLSBReduced;    ///< Shunt LSB in volts (ADCRANGE = 1).
    float _busLSB;             ///< Bus voltage LSB in volts.
    float _tempLSB;            ///< Die temperature LSB in deg-C.
    float _currentFullScale;   ///< Positive half of the ADC range, used to derive CURRENT_LSB.
    float _powerLSBScale;      ///< Power LSB = _powerLSBScale x CURRENT_LSB.
    uint16_t _deviceID;        ///< Expected DEVICE_ID (upper 12 bits) for the identity check.
    uint16_t _deviceIDAlt;     ///< Alternate accepted DEVICE_ID (register-compatible part).

    /// @brief Read 3 bytes (24-bit) from a register into a uint32_t.
    /// @param reg Register address.
    /// @param value Output value (MSB-first assembly).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t readRegister24(uint8_t reg, uint32_t &value);

    /// @brief Read 5 bytes (40-bit) from a register into a uint64_t.
    /// @param reg Register address.
    /// @param value Output value (MSB-first assembly).
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t readRegister40(uint8_t reg, uint64_t &value);

    ///////////////////////////////////////////////////////////////////////////
    // I2C Addressing
    ///////////////////////////////////////////////////////////////////////////
    /// Default 7-bit I2C address for the SparkFun Qwiic Power Monitor board.
    /// A0 = GND, A1 = GND gives 0x40. Both INA228 and INA237 support 0x40-0x4F.
    static const uint8_t kI2CAddress = 0x40;

    // Register Addresses (shared between INA228 and INA237)
    static const uint8_t kRegConfig = 0x00;         ///< Configuration register (16-bit, R/W)
    static const uint8_t kRegAdcConfig = 0x01;      ///< ADC Configuration register (16-bit, R/W)
    static const uint8_t kRegShuntCal = 0x02;       ///< Shunt Calibration register (16-bit, R/W)
    static const uint8_t kRegShuntTempCo = 0x03;    ///< Shunt Temperature Coefficient (16-bit, R/W)
    static const uint8_t kRegVShunt = 0x04;         ///< Shunt Voltage Measurement (24/16-bit, R)
    static const uint8_t kRegVBus = 0x05;           ///< Bus Voltage Measurement (24/16-bit, R)
    static const uint8_t kRegDieTemp = 0x06;        ///< Die Temperature Measurement (16-bit, R)
    static const uint8_t kRegCurrent = 0x07;        ///< Current Result (24/16-bit, R)
    static const uint8_t kRegPower = 0x08;          ///< Power Result (24-bit, R)
    static const uint8_t kRegEnergy = 0x09;         ///< Energy Result (40-bit, R) -- INA228 only
    static const uint8_t kRegCharge = 0x0A;         ///< Charge Result (40-bit, R) -- INA228 only
    static const uint8_t kRegDiagAlrt = 0x0B;       ///< Diagnostic Flags and Alert (16-bit, R/W)
    static const uint8_t kRegSOVL = 0x0C;           ///< Shunt Overvoltage Threshold (16-bit, R/W)
    static const uint8_t kRegSUVL = 0x0D;           ///< Shunt Undervoltage Threshold (16-bit, R/W)
    static const uint8_t kRegBOVL = 0x0E;           ///< Bus Overvoltage Threshold (16-bit, R/W)
    static const uint8_t kRegBUVL = 0x0F;           ///< Bus Undervoltage Threshold (16-bit, R/W)
    static const uint8_t kRegTempLimit = 0x10;      ///< Temperature Over-Limit Threshold (16-bit, R/W)
    static const uint8_t kRegPowerLimit = 0x11;     ///< Power Over-Limit Threshold (16-bit, R/W)
    static const uint8_t kRegManufacturerID = 0x3E; ///< Manufacturer ID (16-bit, R) -- reads 0x5449 ("TI")
    static const uint8_t kRegDeviceID = 0x3F;       ///< Device ID (16-bit, R)

    // Field masks
    static const uint16_t kShuntCalMask = 0x7FFF;     ///< SHUNT_CAL valid bits [14:0] (bit 15 reserved)
    static const uint16_t kBusThresholdMask = 0x7FFF; ///< BOVL/BUVL valid bits [14:0] (bit 15 reserved)
    static const uint16_t kShuntTempCoMask = 0x3FFF;  ///< TEMPCO field mask, bits [13:0]
    static const uint16_t kDeviceIDMask = 0xFFF0;     ///< DEVICE_ID mask (ignore revision nibble)

    // Threshold register LSBs (shared between INA228 and INA237)
    static constexpr float kBusThresholdLSB_mV = 3.125f;        ///< BOVL/BUVL LSB in millivolts.
    static constexpr float kShuntThresholdLSBDefault_mV = 5.0e-3f;    ///< SOVL/SUVL LSB in mV (ADCRANGE = 0).
    static constexpr float kShuntThresholdLSBReduced_mV = 1.25e-3f;   ///< SOVL/SUVL LSB in mV (ADCRANGE = 1).

    // Identity constant (shared between INA228 and INA237)
    static const uint16_t kManufacturerIDValue = 0x5449;  ///< "TI" in ASCII

  private:
    /// @brief Read a signed measurement register (VSHUNT or CURRENT) per the device layout.
    sfTkError_t readMeasurementSigned(uint8_t reg, signed_raw_t &value);

    /// @brief Read an unsigned measurement register (VBUS) per the device layout.
    sfTkError_t readMeasurementUnsigned(uint8_t reg, unsigned_raw_t &value);
};

///////////////////////////////////////////////////////////////////////////////
// Explicit instantiation declarations
///////////////////////////////////////////////////////////////////////////////
// The member functions are defined and explicitly instantiated in sfDevINA2XX.cpp for the two
// type combinations below. These declarations stop every other translation unit from generating
// its own copy. Add a new device's <signed, unsigned> pair here and in the .cpp to support it.
extern template class sfDevINA2XX<int32_t, uint32_t>; ///< INA228 (20-bit values in 32-bit words)
extern template class sfDevINA2XX<int16_t, uint16_t>; ///< INA237 (plain 16-bit values)
