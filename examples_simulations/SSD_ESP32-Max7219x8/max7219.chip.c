// Wokwi Custom Chip - For information and examples see:
// https://link.wokwi.com/custom-chips-alpha
//
// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Sean D Johnston / wokwi.com

#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

const int MS = 1000; // micros

// Index for SEG_X
#define SEG_A 7
#define SEG_B 6
#define SEG_C 5
#define SEG_D 4
#define SEG_E 3
#define SEG_F 2
#define SEG_G 1
#define SEG_DP 0

// Chip state
typedef struct {
  pin_t pin_seg[8];      // SEG_X pins
  pin_t pin_dig[8];      // DIG_X pins

  pin_t pin_load;        // LOAD pin
  pin_t pin_dout;        // DOUT pin

  pin_t digit_pin;       // Digit is currently being displayed

  uint8_t spi_buffer[2]; // 16 bit register for the command
  uint32_t spi;          // SPI handle

  uint8_t div;           // Division counter for intensity

  uint16_t decode_mode;  // Bits to for decoded digit 1=decode, 0 normal
  uint8_t scan_limit;    // Scan limit of the display
  uint8_t intensity;     // Intensitry of the display (Not implemented)
  uint8_t display_test;  // Test mode 0 = Normal, 1 = Test mode
  uint8_t shutdown_mode; // Shutown mode 0 = Normal, 1 = Shutdown mode

  uint8_t digits[8];     // Storage for all the values of the digits

} chip_state_t;

// Deoding for Code B font
uint8_t decode_digits[] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111, // 9
  0b01000000, // -
  0b01111001, // E
  0b01110110, // H
  0b00111000, // L
  0b01110011, // P
  0b00000000, // Blank
};

/* 
Sets the segments on the digit. Uses the value in the 
digits array, for the segments, if decode mode is off.
Uses the value as an index, to the  Code B font, if 
decode mode is on 
*/
void set_digit(chip_state_t *chip, uint8_t digit) {

  if (chip->display_test) {
      for (int i = 0; i < 8; i++) {
        pin_write(chip->pin_seg[i],HIGH);
      }
  }
  else {
    // Get digit to display
    uint8_t dig = chip->digits[digit];

    // if decode for this digit
    if (chip->decode_mode & (0x01 << digit)) {

      // Get segments for the digit
      uint8_t d = decode_digits[dig & 0x0f];

      // Set the decimal point, if we have one
      d = d | (dig & 0x80);

      // Set the segments, based on decoded segments
      for (int i = 0; i < 8; i++) {
        pin_write(chip->pin_seg[i], d & 0x80?HIGH:LOW);
        d = d << 1;
      }
    }
    // If not decode for this digit
    else {
      // Set the segment, based on value
      for (int i = 0; i < 8; i++) {
        pin_write(chip->pin_seg[i], dig & 0x01?HIGH:LOW);
        dig = dig >> 1;
      }
    }
  }
}

/*
Unsets all the segments
*/
void unset_digit(void) {
  for (int i = 0; i < 8; i++) {
    pin_write(i, LOW);
  }
}


static void chip_pin_change(void *user_data, pin_t pin, uint32_t value);
static void chip_spi_done(void *user_data, uint8_t *buffer, uint32_t count);
static void chip_timer_event(void *user_data);

/*
Initializes the chip
*/
void chip_init(void) {
  chip_state_t *chip = malloc(sizeof(chip_state_t));

  // Initialize segment chip pins
  chip->pin_seg[SEG_G]  = pin_init("SEG_G" , OUTPUT);
  chip->pin_seg[SEG_F]  = pin_init("SEG_F" , OUTPUT);
  chip->pin_seg[SEG_E]  = pin_init("SEG_E" , OUTPUT);
  chip->pin_seg[SEG_D]  = pin_init("SEG_D" , OUTPUT);
  chip->pin_seg[SEG_C]  = pin_init("SEG_C" , OUTPUT);
  chip->pin_seg[SEG_B]  = pin_init("SEG_B" , OUTPUT);
  chip->pin_seg[SEG_A]  = pin_init("SEG_A" , OUTPUT);
  chip->pin_seg[SEG_DP]  = pin_init("SEG_DP", OUTPUT);

  // Initialize digit chip pins
  chip->pin_dig[0] = pin_init("DIG_7" , OUTPUT);
  chip->pin_dig[1] = pin_init("DIG_6" , OUTPUT);
  chip->pin_dig[2] = pin_init("DIG_5" , OUTPUT);
  chip->pin_dig[3] = pin_init("DIG_4" , OUTPUT);
  chip->pin_dig[4] = pin_init("DIG_3" , OUTPUT);
  chip->pin_dig[5] = pin_init("DIG_2" , OUTPUT);
  chip->pin_dig[6] = pin_init("DIG_1" , OUTPUT);
  chip->pin_dig[7] = pin_init("DIG_0" , OUTPUT);

  // Initialize load and data out chip pins
  chip->pin_load = pin_init("LOAD"  , INPUT);
  chip->pin_dout = pin_init("DOUT"  , OUTPUT);

  // Set the initial state of segment pins
  for (int i=0; i<8; i++) {
    pin_write(chip->pin_seg[i], LOW);
  }

  // Set the initial state of digit pins
  for (int i=0; i<8; i++) {
    pin_write(chip->pin_dig[i], HIGH);
  }

  // Setup watch for the load pin
  const pin_watch_config_t config = {
    .edge = BOTH,
    .pin_change = chip_pin_change,
    .user_data = chip,
  };
  pin_watch(chip->pin_load, &config);

  // Initialize the SPI interface
  const spi_config_t spi_config = {
    .sck = pin_init("CLK", INPUT),
    .miso = NO_PIN,
    .mosi = pin_init("DIN", INPUT),
    .done = chip_spi_done,
    .user_data = chip,
  };
  chip->spi = spi_init(&spi_config);

  // Setup the timer, to update segment and digit pins
  chip->digit_pin = 0;
  const timer_config_t timer_config = {
    .callback = chip_timer_event,
    .user_data = chip,
  };

  //uint32_t microsecs = (10 * MS) / 16; // For intensity
  uint32_t microsecs = (10 * MS); // Without intensity

  timer_t timer = timer_init(&timer_config);
  timer_start(timer, (15 * MS) / 16, true);
  chip->div = 0;

  // Set up other variables
  chip->decode_mode = 0xff;   // On
  chip->scan_limit = 0x07;    // All segments
  chip->intensity = 0x0f;     // Max
  chip->display_test = 0x00;  // Off
  chip->shutdown_mode = 0x01; // Normal Operation

  chip->digits[0] = chip->digits[1] = chip->digits[2] = chip->digits[3] =
  chip->digits[4] = chip->digits[5] = chip->digits[6] = chip->digits[7] = 0x0f;
}

/*
Check for the change of the load pin, and turns on and off the SPI data
*/
static void chip_pin_change(void *user_data, pin_t pin, uint32_t value) {
  chip_state_t *chip = (chip_state_t*)user_data;

  // Handle the load pin
  if (pin == chip->pin_load) {
    if (value == LOW) {
      //printf("SPI chip selected\n");
      chip->spi_buffer[0] = 0x00;
      chip->spi_buffer[1] = 0x00;
      spi_start(chip->spi, chip->spi_buffer, sizeof(chip->spi_buffer));
    } else {
      //printf("SPI chip deselected\n");
      spi_stop(chip->spi);
    }
  }
}

/*
Process the command in the registry
*/
static void chip_spi_done(void *user_data, uint8_t *buffer, uint32_t count) {
  chip_state_t *chip = (chip_state_t*)user_data;

  // If count is not 2, incomplete data
  if (count < 2) {
    return;
  }

  //printf("Buffer Count: %d\n", count);
  //printf("%x %x\n", buffer[0],buffer[1]);

  // Get the address and data bytes
  uint16_t addr = buffer[0];
  uint16_t data = buffer[1];
  
  printf("addr: %d - data: %d\n", addr, data);
  uint8_t a;
  
  // Process the command, from the register
  switch(addr & 0b00001111) {
    case 0x00: // No-Op
      break;

    case 0x01: // Digit 0
    case 0x02: // Digit 1
    case 0x03: // Digit 2
    case 0x04: // Digit 3
    case 0x05: // Digit 4
    case 0x06: // Digit 5
    case 0x07: // Digit 6
    case 0x08: // Digit 7
      chip->digits[(addr & 0b00001111) - 1] = data;
      break;

    case 0x09: // Dedode Mode
      chip->decode_mode = data & 0xff;
      break;

    case 0x0a: // Intensity
      chip->intensity = (data & 0x0f);
      break;

    case 0x0b: // Scan Limit
      chip->scan_limit = data & 0x0f;
      break;

    case 0x0c: // Shutdown
      chip->shutdown_mode = data &0x01;
      break;

    case 0x0f: // Display Test
      chip->display_test = data &0x01;
      break;
  };

  // Get ready to read again
  if (pin_read(chip->pin_load) == LOW) {
    spi_start(chip->spi, chip->spi_buffer, sizeof(chip->spi_buffer));
  }
}

/*
Updates the display of the digit on the display. The digits are 
multiplexed, based on the timer
*/
static void chip_timer_event(void *user_data) {
  chip_state_t *chip = (chip_state_t*)user_data;

  if (chip->shutdown_mode == 0x00) {
    for (int i=0; i<8; i++) {
      pin_write(chip->pin_seg[i], LOW);
    }

    for (int i=0; i<8; i++) {
      pin_write(chip->pin_dig[i], HIGH);
    }
  }
  else {
    if (chip->div == 0) {
      // Set the previous digit pin high (off)
      pin_write(chip->pin_dig[chip->digit_pin], HIGH);
      chip->digit_pin++;

      // Reset to the beginning
      if (chip->digit_pin >= chip->scan_limit + 1) {
        chip->digit_pin = 0;
      }

      // Turn on the digit and set the digit information
      pin_write(chip->digit_pin+8, LOW);
      set_digit(chip, chip->digit_pin);
    }

    /* Can't get the intensity to work right.
    if (chip->div == intensity) {
      unset_digit();
    }

    chip->div++;
    if (chip->div >= 0x0f) {
      chip->div = 0;
    }
    */
  }
}
