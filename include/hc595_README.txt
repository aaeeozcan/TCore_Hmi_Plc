74HC595 SHIFT REGISTER DRIVER
==============================

FEATURE CONTROL
---------------
Edit hc595.h to enable/disable features:

#define HC595_ENABLE_SPI_MODE    0

  0 = Software mode only (bit-bang, GPIO pins)
  1 = Enable SPI hardware mode support

Default: 0 (Software mode)

CURRENT CONFIGURATION
--------------------
✓ Software mode (bit-bang)
✗ SPI mode (disabled)

This means:
- No SPI peripheral needed
- Only 3 GPIO pins required (DATA, CLOCK, LATCH)
- Works out-of-the-box
- SPI_HandleTypeDef is not used

TO ENABLE SPI MODE
------------------
1. Set HC595_ENABLE_SPI_MODE to 1 in hc595.h
2. Enable SPI peripheral in CubeMX
3. Configure handle:
   hc595_outputs.mode = HC595_MODE_HARDWARE;
   hc595_outputs.hspi = &hspi1;

FILES
-----
Core/Inc/hc595.h              - Driver header
Core/Src/hc595.c              - Driver implementation
Core/Src/main.c               - Integration and test commands
DOC/74HC595_INTEGRATION.md    - Full documentation

PIN CONFIGURATION
-----------------
Edit main.c to set your pins:

#define HC595_DATA_PORT     GPIOC
#define HC595_DATA_PIN      GPIO_PIN_8
#define HC595_CLOCK_PORT    GPIOC
#define HC595_CLOCK_PIN     GPIO_PIN_9
#define HC595_LATCH_PORT    GPIOC
#define HC595_LATCH_PIN     GPIO_PIN_10
#define HC595_NUM_CHIPS     2

TEST COMMANDS
-------------
H  - Test 74HC595 outputs
O  - Show output status
?  - Help menu

API USAGE
---------
// Initialize
Init_74HC595();

// Set single pin
HC595_SetPin(&hc595_outputs, 0, true);
HC595_Update(&hc595_outputs);

// Set entire chip
HC595_SetChip(&hc595_outputs, 0, 0xFF);
HC595_Update(&hc595_outputs);

// Clear all
HC595_Clear(&hc595_outputs);

For detailed documentation, see:
DOC/74HC595_INTEGRATION.md
