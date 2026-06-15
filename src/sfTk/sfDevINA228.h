/**
 * @file sfDevINA228.h
 * @brief Header file for the SparkFun INA228 high-precision power monitor driver.
 *
 * @details
 * sfDevINA228 derives directly from the sfDevINA2XX base template instantiated with the
 * INA228 raw register types (@c int32_t / @c uint32_t), plus the features unique to the INA228:
 *   - 20-bit ADC with 24-bit measurement registers (VSHUNT, VBUS, CURRENT)
 *   - 24-bit POWER register
 *   - 40-bit ENERGY and CHARGE accumulation registers
 *   - High-precision LSB scaling (312.5 nV shunt, 195.3125 uV bus, 7.8125 m-deg-C temp)
 *   - Calibration using scale constant 13107.2 x 10^6
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

/// @brief INA228 high-precision power/energy/charge monitor driver.
///
/// @details Derives from the sfDevINA2XX base template (instantiated with the INA228 raw
/// register types: 20-bit values held in 32-bit words) and supplies the INA228 scaling and
/// identity configuration through the base constructor. Adds the INA228-only 40-bit
/// energy/charge accumulator reads. The device-identity check (isConnected) is inherited from
/// the base, driven by the DEVICE_ID values passed below.
class sfDevINA228 : public sfDevINA2XX<int32_t, uint32_t>
{
  public:
    sfDevINA228()
        : sfDevINA2XX<int32_t, uint32_t>(/* is24BitMeasurements */ true, /* tempShift */ 0,
                                         /* calScale */ 13107.2e6f, /* shuntLSBDefault */ 312.5e-9f,
                                         /* shuntLSBReduced */ 78.125e-9f, /* busLSB */ 195.3125e-6f,
                                         /* tempLSB */ 7.8125e-3f, /* currentFullScale */ 524288.0f,
                                         /* powerLSBScale */ 3.2f, /* deviceID */ 0x2280,
                                         /* deviceIDAlt */ 0x2280)
    {
    }

    // ========================= Energy & Charge (INA228 Only) ================

    /// @brief Read the accumulated energy in Joules.
    /// @details Reads the 40-bit ENERGY register (unsigned) and scales by
    /// 16 * 3.2 * CURRENT_LSB.
    /// @note calibrate() must be called before this method returns meaningful values.
    /// @param joules Output reference that receives the energy in Joules.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getEnergy_J(double &joules);

    /// @brief Read the accumulated charge in Coulombs.
    /// @details Reads the 40-bit CHARGE register (two's complement) and scales by
    /// CURRENT_LSB.
    /// @note calibrate() must be called before this method returns meaningful values.
    /// @param coulombs Output reference that receives the charge in Coulombs.
    /// @return ::ksfTkErrOk on success, or an error code on failure.
    sfTkError_t getCharge_C(double &coulombs);

  protected:
    // --- INA228-only scaling constants ---
    static constexpr double kEnergyLSBScale = 16.0 * 3.2; ///< Energy LSB = 16 x 3.2 x CURRENT_LSB.
};
