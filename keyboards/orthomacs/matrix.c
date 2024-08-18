/******************************************************************************
 * Copyright 2024 Swan (https://github.com/pitworker)                         *
 * Based on 2-way matrix as explained by e3w2q (https://github.com/e3w2q) and *
 * implementing Jun Wako's and Kyrremann's implementation for Bartlesplit     *
 *                                                                            *
 * This program is free software: you can redistribute it and/or modify it    *
 * under the terms of the GNU General Public License as published by the Free *
 * Software Foundation, either version 2 of the License, or (at your option)  *
 * any later version.                                                         *
 *                                                                            *
 * This program is distributed in the hope that it will be useful, but        *
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   *
 * for more details.                                                          *
 *                                                                            *
 * You should have received a copy of the GNU General Public License along    *
 * with this program.  If not, see <http://www.gnu.org/licenses/>.            *
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "wait.h"
#include "util.h"
#include "matrix.h"
#include "quantum.h"
#include "print.h"

#ifndef MATRIX_INPUT_PRESSED_STATE
#  define MATRIX_INPUT_PRESSED_STATE 0
#endif

#define MATRIX_COLS_PIN_COUNT MATRIX_COLS >> 1

static const pin_t row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;
static const pin_t col_pins[MATRIX_COLS_PIN_COUNT] = MATRIX_COL_PINS;

static bool select_row (uint8_t row) {
  pin_t pin = row_pins[row];
  if (pin != NO_PIN) {
    ATOMIC_BLOCK_FORCEON {
      gpio_set_pin_output(pin);
      gpio_write_pin_low(pin);
    }
    return true;
  }
  return false;
}

static void unselect_row (uint8_t row) {
  pin_t pin = row_pins[row];
  if (pin != NO_PIN) {
    ATOMIC_BLOCK_FORCEON {
      gpio_set_pin_output(pin);
      gpio_write_pin_high(pin);
    }
  }
}

static bool select_col (uint8_t col) {
  pin_t pin = col_pins[col];
  if (pin != NO_PIN) {
    ATOMIC_BLOCK_FORCEON {
      gpio_set_pin_output(pin);
      gpio_write_pin_low(pin);
    }
    return true;
  }
  return false;
}

static void unselect_col (uint8_t col) {
  pin_t pin = col_pins[col];
  if (pin != NO_PIN) {
    ATOMIC_BLOCK_FORCEON {
      gpio_set_pin_output(pin);
      gpio_write_pin_high(pin);
    }
  }
}

static void unselect_all_rows (void) {
  for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
    unselect_row(row);
  }
}

static void unselect_all_cols (void) {
  for (uint8_t col = 0; col < MATRIX_COLS_PIN_COUNT; col++) {
    unselect_col(col);
  }
}

static bool read_matrix_pin (pin_t pin) {
  if (pin != NO_PIN) {
    return gpio_read_pin(pin) == MATRIX_INPUT_PRESSED_STATE;
  }
  return false;
}

static void init_pins (void) {
  print("ORTHOMACS initializing\n");
  unselect_all_cols();
  unselect_all_rows();
}

static bool matrix_read_rows_on_col (
  matrix_row_t current_matrix[],
  uint8_t current_col_index
) {
  bool matrix_has_changed = false;

  // Scanning on col only reads even matrix columns
  matrix_row_t col_index_bitmask =
    MATRIX_ROW_SHIFTER << (current_col_index * 2);

  // Select column, skip if NO_PIN
  if (!select_col(current_col_index)) {
    return matrix_has_changed;
  }

  // Wait for column selection to stabilize
  matrix_output_select_delay();

  // For each row...
  for (uint8_t row_index = 0; row_index < MATRIX_ROWS; row_index++) {
    matrix_row_t old_row_values = current_matrix[row_index];
    bool pin_state = read_matrix_pin(row_pins[row_index]);

    // Check row pin state
    if (pin_state) {
      // Pin registered keypress, set col bit
      current_matrix[row_index] |= col_index_bitmask;
    } else {
      // Pin did not register keypress, clear col bit
      current_matrix[row_index] &= ~col_index_bitmask;
    }

    // Check if any changes were made to the matrix
    matrix_has_changed =
      (old_row_values != current_matrix[row_index]) || matrix_has_changed;
  }

  // Unselect col pin
  unselect_col(current_col_index);

  return matrix_has_changed;
}

static bool matrix_read_cols_on_row (
  matrix_row_t current_matrix[],
  uint8_t current_row_index
) {
  bool matrix_has_changed = false;
  matrix_row_t old_row_values = current_matrix[current_row_index];

  // Select row, skip if NO_PIN
  if (!select_row(current_row_index)) {
    return matrix_has_changed;
  }

  // Wait for row selection to stabilize
  matrix_output_select_delay();

  // For each col...
  for (
    uint8_t col_index = 0;
    col_index < MATRIX_COLS_PIN_COUNT;
    col_index++
  ) {
    // Scanning on row only reads even matrix columns
    matrix_row_t col_index_bitmask =
      MATRIX_ROW_SHIFTER << (col_index * 2 + 1);
    bool pin_state = read_matrix_pin(col_pins[col_index]);

    // Check row pin state
    if (pin_state) {
      // Pin registered keypress, set row bit
      current_matrix[current_row_index] |= col_index_bitmask;
    } else {
      // Pin did not register keypress, clear col bit
      current_matrix[current_row_index] &= ~col_index_bitmask;
    }
  }

  // Unselect row pin
  unselect_row(current_row_index);

  // Check if any changes are being made to the matrix
  matrix_has_changed = current_matrix[current_row_index] != old_row_values;

  return matrix_has_changed;
}

void matrix_init_custom (void) {
  init_pins();
}

bool matrix_scan_custom (matrix_row_t current_matrix[]) {
  bool matrix_has_changed = false;

  // Read columns on all rows
  for (uint8_t row_index = 0; row_index < MATRIX_ROWS; row_index++) {
    matrix_has_changed =
      matrix_read_cols_on_row(current_matrix, row_index) ||
      matrix_has_changed;
  }

  // Read rows on all columns
  for (
    uint8_t col_index = 0;
    col_index < MATRIX_COLS_PIN_COUNT;
    col_index++
  ) {
    matrix_has_changed =
      matrix_read_rows_on_col(current_matrix, col_index) ||
      matrix_has_changed;
  }

# ifndef NO_DEBUG
    if (matrix_has_changed) {
      for (uint8_t row_index = 0; row_index < MATRIX_ROWS; row_index++) {
        uprintf("%X\n", current_matrix[row_index]);
      }
      print("\n");
    }
# endif
  return matrix_has_changed;
}
