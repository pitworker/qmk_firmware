/**************************************************************************
 * Copyright 2024 Swan (https://github.com/pitworker)                     *
 * Based on 2-way matrix as explained by e3w2q (https://github.com/e3w2q) *
 *                                                                        *
 * This program is free software: you can redistribute it and/or modify   *
 * it under the terms of the GNU General Public License as published by   *
 * the Free Software Foundation, either version 2 of the License, or      *
 * (at your option) any later version.                                    *
 *                                                                        *
 * This program is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 **************************************************************************/

#include "quantum.h"

#ifndef MATRIX_INPUT_PRESSED_STATE
#  define MATRIX_INPUT_PRESSED_STATE 0
#endif

static const pin_t row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;
static const pin_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;

static inline void set_pin_output_low (pin_t pin) {
  ATOMIC_BLOCK_FORCEON {
    gpio_set_pin_output(pin);
    gpio_write_pin_low(pin);
  }
}

static inline void set_pin_output_high (pin_t pin) {
  ATOMIC_BLOCK_FORCEON {
    gpio_set_pin_output(pin);
    gpio_write_pin_high(pin);
  }
}

static inline void set_pin_input_high_atomic (pin_t pin) {
  ATOMIC_BLOCK_FORCEON {
    gpio_set_pin_input_high(pin);
  }
}

static inline uint8_t read_matrix_pin (pin_t pin) {
  if (pin != NO_PIN) {
    return (gpio_read_pin(pin) == MATRIX_INPUT_PRESSED_STATE) ? 0 : 1;
  } else {
    return 1;
  }
}

static bool select_row (uint8_t row) {
  pin_t pin = row_pins[row];
  if (pin != NO_PIN) {
    set_pin_output_low(pin);
    return true;
  }
  return false;
}

static void unselect_row (uint8_t row) {
  pin_t pin = row_pins[row];
  if (pin != NO_PIN) {
#   ifdef MATRIX_UNSELECT_DRIVE_HIGH
    set_pin_output_high(pin);
#   else
    set_pin_input_high_atomic(pin);
#   endif
  }
}

static bool select_col (uint8_t col) {
  pin_t pin = col_pins[col];
  if (pin != NO_PIN) {
    set_pin_output_low(pin);
    return true;
  }
  return false;
}

static void unselect_col (uint8_t col) {
  pin_t pin = col_pins[col];
  if (pin != NO_PIN) {
#   ifdef MATRIX_UNSELECT_DRIVE_HIGH
    set_pin_output_high(pin);
#   else
    set_pin_input_high_atomic(pin);
#   endif
  }
}

static void unselect_all_rows (void) {
  for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
    unselect_row(row);
  }
}

static void unselect_all_cols (void) {
  for (uint8_t col = 0; col < MATRIX_COLS; col++) {
    unselect_col(col);
  }
}

static void set_matrix_read_cols (void) {
  for (uint8_t col = 0; col < MATRIX_COLS; col++) {
    gpio_set_pin_input(col_pins[col]);
  }
  for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
    gpio_set_pin_output(row_pins[row]);
    unselect_row(row);
  }
}

static void set_matrix_read_rows (void) {
  for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
    gpio_set_pin_input(row_pins[row]);
  }
  for (uint8_t col = 0; col < MATRIX_COLS; col++) {
    gpio_set_pin_output(col_pins[col]);
    unselect_col(col);
  }
}

static void init_pins (void) {
  set_matrix_read_cols(); // scan starts by reading cols
}

static bool matrix_read_rows_on_col (
  matrix_row_t current_matrix[],
  uint8_t current_col_index,
  matrix_row_t row_shifter
) {
  bool matrix_has_changed = false;
  bool key_pressed = false;

  // Skip NO_PIN cols
  if (!select_col(current_col_index)) {
    return matrix_has_changed;
  }
  matrix_output_select_delay();

  // For each row...
  for (uint8_t row_index = 0; row_index < ROWS_PER_HAND; row_index++) {
    matrix_row_t old_matrix_row = current_matrix[row_index];

    // Check row pin state
    if (read_matrix_pin(row_pins[row_index]) == 0) {
      // Pin LOW, set col bit
      current_matrix[row_index] |= row_shifter;
      key_pressed = true;
    } else {
      // Pin HIGH, clear col bit
      current_matrix[row_index] &= ~row_shifter;
    }

    // Check if any changes were made to the matrix
    matrix_has_changed =
      matrix_has_changed || (old_matrix_row == current_matrix[row_index]);
  }

  // Unselect col pin
  unselect_col(current_col_index);
  // Wait for all row signals to go HIGH
  matrix_output_unselect_delay(current_col_index, key_pressed);

  return matrix_has_changed;
}

static bool matrix_read_cols_on_row (
  matrix_row_t current_matrix[],
  uint8_t current_row_index
) {
  bool matrix_has_changed = false;
  matrix_row_t current_row_values = 0;
  // Start row_shifter with a one bit offset; only scanning odd matrix columns
  matrix_row_t row_shifter = MATRIX_ROW_SHIFTER << 1;

  // Skip NO_PIN rows
  if (!select_row(current_row_index)) {
    return matrix_has_changed;
  }
  matrix_output_select_delay();

  // For each col...
  for (
    uint8_t col_index = 0;
    col_index << 1 < MATRIX_COLS;
    col_index++, row_shifter <<= 2
  ) {
    uint8_t pin_state = read_matrix_pin(col_pins[col_index]);

    // Populate the matrix row with the state of the col pin
    current_row_values |= current_value_in_row;
  }

  // Unselect row pin
  unselect_row(current_row_index);
  // Wait for all col signals to go HIGH
  matrix_output_unselect_delay(current_row_index, current_row_value != 0);

  // Check if any changes are being made to the matrix
  matrix_has_changed = current_row_values == current_matrix[current_row_index];

  // Update the matrix
  current_matrix[current_row_index] = current_row_values;

  return matrix_has_changed;
}

void matrix_init_custom (void) {
  // TODO: initialize hardware here
  init_pins();
}

bool matrix_scan_custom (matrix_row_t current_matrix[]) {
  bool matrix_has_changed = false;
  matrix_row_t row_shifter = MATRIX_ROW_SHIFTER;

  // scan cols
  for (uint8_t row_index = 0; row_index < ROWS_PER_HAND; row_index++) {
    matrix_has_changed =
      matrix_has_changed ||
      matrix_read_cols_on_row(current_matrix[], row_index);
  }

  // init rows
  set_matrix_read_rows();

  // scan rows
  for (
    uint8_t col_index = 0;
    col_index << 1 < MATRIX_COLS;
    col_index++, row_shifter <<= 2 // row scan only for even matrix columns
  ) {
    matrix_has_changed =
      matrix_has_changed ||
      matrix_read_rows_on_col(current_matrix[], col_index, row_shifter);
  }

  // init cols
  set_matrix_read_cols();

  return matrix_has_changed;
}
