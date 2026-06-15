/**
 * @file sfDevINA228.cpp
 * @brief Implementation file for the SparkFun INA228 high-precision power monitor driver.
 *
 * @details
 * This file implements the INA228-only features: the 40-bit energy/charge accumulator reads.
 * Calibration, the shared measurement methods, and the device-identity check are inherited from
 * the sfDevINA2XX base template (see sfDevINA2XX.h), instantiated with the INA228 raw register
 * types (int32_t / uint32_t).
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
 * @date June 2026
 * @copyright Copyright (c) 2026, SparkFun Electronics Inc. This project is released under the MIT License.
 *
 * SPDX-License-Identifier: MIT
 *
 * @see https://github.com/sparkfun/SparkFun_INA2XX_Arduino_Library
 */

#include "sfDevINA228.h"

// ========================= Energy & Charge ==================================

sfTkError_t sfDevINA228::getEnergy_J(double &joules)
{
    uint64_t raw;
    sfTkError_t rc = readRegister40(kRegEnergy, raw);
    if (rc != ksfTkErrOk)
        return rc;

    // Energy LSB = 16 x 3.2 x CURRENT_LSB.
    joules = (double)raw * kEnergyLSBScale * (double)_currentLSB;
    return ksfTkErrOk;
}

sfTkError_t sfDevINA228::getCharge_C(double &coulombs)
{
    uint64_t raw;
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
