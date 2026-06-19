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

// The INA237 has no device-specific source: every public method in sfDevINA237.h is a thin
// forwarder onto the shared, individually templated helpers in sfDevINA2XX. This translation
// unit is intentionally empty (aside from the include) so the library build still finds a
// matching .cpp for the header.
