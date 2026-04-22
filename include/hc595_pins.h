/*
 * hc595_pins.h
 * HC595 Pin Definitions for Control and LED Groups
 */

#ifndef HC595_PINS_H
#define HC595_PINS_H

// =============================================================================
// HC595 SHARED PIN CONFIGURATION
// =============================================================================

// Shared DATA and CLOCK pins (used by both groups)
#define HC595_SHARED_DATA_PORT     GPIOB
#define HC595_SHARED_DATA_PIN      GPIO_PIN_4
#define HC595_SHARED_CLOCK_PORT    GPIOB
#define HC595_SHARED_CLOCK_PIN     GPIO_PIN_5

// LED Group LATCH pin (4x HC595 for 32 LEDs)
#define HC595_LED_LATCH_PORT       GPIOB
#define HC595_LED_LATCH_PIN        GPIO_PIN_3

// Control Group LATCH pin (2x HC595 for PT100 CS, EEPROM, etc.)
#define HC595_CTRL_LATCH_PORT      GPIOC
#define HC595_CTRL_LATCH_PIN       GPIO_PIN_4

// =============================================================================
// CONTROL HC595 PIN MAPPING (U4, U5)
// =============================================================================

// U4 outputs (Chip 0, bits 0-7) - First in cascade
#define CTRL_PT_OUT1        0    // U4 QA - PT_OUT1
#define CTRL_PT_OUT6        1    // U4 QB - PT_OUT6
#define CTRL_PT_OUT5        2    // U4 QC - PT_OUT5
#define CTRL_PT_OUT2        3    // U4 QD - PT_OUT2
#define CTRL_PT_OUT3        4    // U4 QE - PT_OUT3
#define CTRL_PT_OUT4        5    // U4 QF - PT_OUT4
#define CTRL_PT_CS2         6    // U4 QG - PT_CS2
#define CTRL_SPI2_CS        7    // U4 QH - SPI2_CS (ENC424J600)

// U5 outputs (Chip 1, bits 8-15) - Second in cascade
#define CTRL_OUT_G          8    // U5 QA - OUT_G
#define CTRL_U5_QB          9    // U5 QB - (unused)
#define CTRL_EEP3           10   // U5 QC - EEP3
#define CTRL_EEP2           11   // U5 QD - EEP2
#define CTRL_EEP1           12   // U5 QE - EEP1
#define CTRL_U5_QF          13   // U5 QF - (unused)
#define CTRL_PT_CS1         14   // U5 QG - PT_CS1
#define CTRL_U5_QH          15   // U5 QH - (unused)

// =============================================================================
// CONVENIENCE ALIASES
// =============================================================================

// PT100 Chip Select bits
#define PT_CS1_BIT          CTRL_PT_CS1   // U5 QG = bit 14
#define PT_CS2_BIT          CTRL_PT_CS2   // U4 QG = bit 6

// EEPROM Select bits
#define EEP1_BIT            CTRL_EEP1     // U5 QE = bit 12
#define EEP2_BIT            CTRL_EEP2     // U5 QD = bit 11
#define EEP3_BIT            CTRL_EEP3     // U5 QC = bit 10

// SPI2 Chip Select (ENC424J600 Ethernet)
#define SPI2_CS_BIT         CTRL_SPI2_CS  // U4 QH = bit 7

// Output Gate
#define OUT_G_BIT           CTRL_OUT_G    // U5 QA = bit 8

// =============================================================================
// LED HC595 PIN MAPPING (32 LEDs across 4x HC595)
// =============================================================================

// Input LEDs (bits 0-15)
#define LED_INPUT_START     0
#define LED_INPUT_COUNT     16

// Output LEDs (bits 16-23)
#define LED_OUTPUT_START    16
#define LED_OUTPUT_COUNT    8

// Status LEDs (bits 24-31)
#define LED_STATUS_START    24
#define LED_STATUS_COUNT    8

#endif // HC595_PINS_H
