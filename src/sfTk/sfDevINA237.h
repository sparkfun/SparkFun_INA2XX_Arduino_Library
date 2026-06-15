/**
 * @file sfDevINA237.h
 * @brief Header file for the SparkFun INA237 power monitor driver.
 *
 * @details
 * sfDevINA237 derives directly from the sfDevINA2XX base template instantiated with the
 * INA237 raw register types (@c int16_t / @c uint16_t):
 *   - 16-bit ADC with 16-bit measurement registers (VSHUNT, VBUS, CURRENT)
 *   - 24-bit POWER register
 *   - No energy or charge accumulation (registers 0x09/0x0A do not exist)
 *   - Lower-precision LSB scaling (5 uV shunt, 3.125 mV bus, 125 m-deg-C temp)
 *   - Calibration using scale constant 819.2 x 10^6
 *
 * Like the base class, every method returns a SparkFun Toolkit error code; measured values are
 * returned through reference (output) parameters.
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

/// @brief INA237 power monitor driver.
///
/// @details Derives from the sfDevINA2XX base template (instantiated with the INA237 raw
/// register types: plain 16-bit values) and supplies the INA237 scaling and identity
/// configuration through the base constructor. Unlike the INA228, the INA237 has no energy or
/// charge accumulation registers. The device-identity check (isConnected) is inherited from the
/// base; it accepts both the INA237 (0x237x) and the register-compatible INA238 (0x238x) via the
/// primary and alternate DEVICE_ID values passed below.
class sfDevINA237 : public sfDevINA2XX<int16_t, uint16_t>
{
  public:
    sfDevINA237()
        : sfDevINA2XX<int16_t, uint16_t>(/* is24BitMeasurements */ false, /* tempShift */ 4,
                                         /* calScale */ 819.2e6f, /* shuntLSBDefault */ 5.0e-6f,
                                         /* shuntLSBReduced */ 1.25e-6f, /* busLSB */ 3.125e-3f,
                                         /* tempLSB */ 125.0e-3f, /* currentFullScale */ 32768.0f,
                                         /* powerLSBScale */ 0.2f, /* deviceID */ 0x2370,
                                         /* deviceIDAlt */ 0x2380)
    {
    }
};
