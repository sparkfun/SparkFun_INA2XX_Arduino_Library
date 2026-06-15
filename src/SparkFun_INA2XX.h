/**
 * @file SparkFun_INA2XX.h
 * @brief Arduino-specific implementation for the SparkFun INA228/INA237 Power Monitors.
 *
 * @details
 * This file provides the Arduino-specific wrappers for the INA228 and INA237 driver classes.
 * Each wrapper class inherits from its platform-independent device driver and implements
 * I2C communication using Arduino's Wire library via the SparkFun Toolkit.
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

// clang-format off
#include <SparkFun_Toolkit.h>
#include "sfTk/sfDevINA228.h"
#include "sfTk/sfDevINA237.h"
#include <Arduino.h>
// clang-format on

/**
 * @class SfeINA228ArdI2C
 * @brief Arduino I2C implementation for the INA228 high-precision power monitor.
 *
 * @details
 * This class provides Arduino-specific I2C communication for the INA228.
 * It inherits all register access and measurement methods from sfDevINA228 and adds
 * the begin() method required for Arduino initialization.
 *
 * The class owns an sfTkArdI2C bus object which wraps the Arduino Wire library.
 *
 * @see sfDevINA228
 * @see sfDevINA2XX
 */
class SfeINA228ArdI2C : public sfDevINA228
{
  public:
    SfeINA228ArdI2C()
    {
    }

    /**
     * @brief Initializes the INA228 with I2C communication.
     *
     * @details
     * Performs the following initialization:
     * 1. Initializes the Toolkit I2C bus with the given address and Wire port
     * 2. Calls the base class begin() to set the bus and configure byte order
     * 3. Verifies the device is connected via Device ID check
     *
     * @param address 7-bit I2C address (default: 0x40)
     * @param wirePort TwoWire instance for I2C communication (default: Wire)
     *
     * @return true If initialization is successful
     * @return false If any step fails
     */
    bool begin(uint8_t address = kI2CAddress, TwoWire &wirePort = Wire)
    {
        if (_theI2CBus.init(wirePort, address) != ksfTkErrOk)
            return false;

        if (sfDevINA228::begin(&_theI2CBus) != ksfTkErrOk)
            return false;

        return isConnected();
    }

  private:
    sfTkArdI2C _theI2CBus; ///< Arduino I2C bus interface instance.
};

/**
 * @class SfeINA237ArdI2C
 * @brief Arduino I2C implementation for the INA237 power monitor.
 *
 * @details
 * This class provides Arduino-specific I2C communication for the INA237.
 * It inherits all register access and measurement methods from sfDevINA237 and adds
 * the begin() method required for Arduino initialization.
 *
 * @see sfDevINA237
 * @see sfDevINA2XX
 */
class SfeINA237ArdI2C : public sfDevINA237
{
  public:
    SfeINA237ArdI2C()
    {
    }

    /**
     * @brief Initializes the INA237 with I2C communication.
     *
     * @details
     * Performs the following initialization:
     * 1. Initializes the Toolkit I2C bus with the given address and Wire port
     * 2. Calls the base class begin() to set the bus and configure byte order
     * 3. Verifies the device is connected via Device ID check
     *
     * @param address 7-bit I2C address (default: 0x40)
     * @param wirePort TwoWire instance for I2C communication (default: Wire)
     *
     * @return true If initialization is successful
     * @return false If any step fails
     */
    bool begin(uint8_t address = kI2CAddress, TwoWire &wirePort = Wire)
    {
        if (_theI2CBus.init(wirePort, address) != ksfTkErrOk)
            return false;

        if (sfDevINA237::begin(&_theI2CBus) != ksfTkErrOk)
            return false;

        return isConnected();
    }

  private:
    sfTkArdI2C _theI2CBus; ///< Arduino I2C bus interface instance.
};
