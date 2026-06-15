/*
  Example 00 - Diagnostics

  Quick debug sketch to find out exactly why the INA237 detection is failing.
  This reads the raw Device ID and Manufacturer ID registers and prints them,
  skipping the auto-detect logic entirely.

  Upload this with either the INA228 or INA237 connected and check the output.
*/

#include <Wire.h>

const uint8_t DEVICE_ADDR = 0x40;

// Read a 16-bit register (big-endian) from an 8-bit register address.
bool readReg16(uint8_t regAddr, uint16_t &value)
{
    Wire.beginTransmission(DEVICE_ADDR);
    Wire.write(regAddr);
    uint8_t err = Wire.endTransmission(false);  // repeated start
    if (err != 0)
    {
        Serial.print("  I2C write error: ");
        Serial.println(err);
        return false;
    }

    uint8_t count = Wire.requestFrom(DEVICE_ADDR, (uint8_t)2);
    if (count != 2)
    {
        Serial.print("  Only got ");
        Serial.print(count);
        Serial.println(" bytes (expected 2)");
        return false;
    }

    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    value = ((uint16_t)msb << 8) | lsb;
    return true;
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== INA2XX Diagnostics ===");
    Serial.println();

    Wire.begin();
    delay(100);

    // --- Step 1: I2C Scan ---
    Serial.println("Step 1: I2C Scan for 0x40...");
    Wire.beginTransmission(DEVICE_ADDR);
    uint8_t scanErr = Wire.endTransmission();
    if (scanErr == 0)
        Serial.println("  Device found at 0x40!");
    else
    {
        Serial.print("  No device at 0x40 (error=");
        Serial.print(scanErr);
        Serial.println(")");
        Serial.println("  Check wiring. Freezing...");
        while (1) delay(1000);
    }
    Serial.println();

    // --- Step 2: Read Manufacturer ID (register 0x3E) ---
    Serial.println("Step 2: Reading Manufacturer ID (reg 0x3E)...");
    uint16_t mfgID = 0;
    if (readReg16(0x3E, mfgID))
    {
        Serial.print("  Manufacturer ID = 0x");
        Serial.println(mfgID, HEX);
        if (mfgID == 0x5449)
            Serial.println("  -> Matches TI ('TI' = 0x5449) - GOOD");
        else
            Serial.println("  -> UNEXPECTED value (should be 0x5449 for TI)");
    }
    else
        Serial.println("  FAILED to read Manufacturer ID!");
    Serial.println();

    // --- Step 3: Read Device ID (register 0x3F) ---
    Serial.println("Step 3: Reading Device ID (reg 0x3F)...");
    uint16_t devID = 0;
    if (readReg16(0x3F, devID))
    {
        Serial.print("  Device ID = 0x");
        Serial.println(devID, HEX);

        uint16_t dieID = (devID >> 4) & 0xFFF;
        uint8_t revID = devID & 0x0F;

        Serial.print("  Die ID = 0x");
        Serial.print(dieID, HEX);
        Serial.print("  Rev = ");
        Serial.println(revID);

        if (dieID == 0x228)
            Serial.println("  -> This is an INA228!");
        else if (dieID == 0x237)
            Serial.println("  -> This is an INA237!");
        else if (dieID == 0x236)
            Serial.println("  -> This is an INA236 (not INA237)! Update library Device ID.");
        else
        {
            Serial.print("  -> UNKNOWN device (Die ID 0x");
            Serial.print(dieID, HEX);
            Serial.println(")");
        }

        // Check what our library expects
        Serial.println();
        Serial.println("  Library check:");
        uint16_t masked = devID & 0xFFF0;
        Serial.print("    devID & 0xFFF0 = 0x");
        Serial.println(masked, HEX);
        Serial.print("    INA228 expects 0x2280: ");
        Serial.println(masked == 0x2280 ? "MATCH" : "no match");
        Serial.print("    INA237 expects 0x2370: ");
        Serial.println(masked == 0x2370 ? "MATCH" : "no match");
    }
    else
        Serial.println("  FAILED to read Device ID!");
    Serial.println();

    // --- Step 4: Read CONFIG (register 0x00) just to confirm comms ---
    Serial.println("Step 4: Reading CONFIG (reg 0x00)...");
    uint16_t config = 0;
    if (readReg16(0x00, config))
    {
        Serial.print("  CONFIG = 0x");
        Serial.println(config, HEX);
        Serial.println("  (Default after POR should be 0x0000)");
    }
    else
        Serial.println("  FAILED to read CONFIG!");
    Serial.println();

    // --- Step 5: Now test the library's detection ---
    Serial.println("Step 5: Testing library auto-detect...");
    Serial.println();

    // Include the library header for this
    // (already included via SparkFun_INA2XX.h in the main sketch)
    Serial.println("  Done! Compare the raw Device ID above with what the library expects.");
    Serial.println("  If they don't match, that's the bug.");
}

void loop()
{
    delay(10000);
}
