/*************************************************************************
 * Copyright 2024 Swan (https://github.com/pitworker/)                   *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

#include "quantum.h"

static const pin_t row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;
static const pin_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;

static inline uint8_t read_matrix_pin(pin_t pin) {
  if (pin != NO_PIN) {
    return (gpio_read_pin(pin) == MATRIX_INPUT_PRESSED_STATE) ? 0 : 1;
  } else {
    return 1;
  }
}

static void select_row (uint8_t row) {
  gpio_write_pin_low(row_pins[row]);
}

static void unselect_row (uint8_t row) {
  gpio_write_pin_high(row_pins[row]);
}

static void select_col (uint8_t col) {
  gpio_write_pin_low(col_pins[col]);
}

static void unselect_col (uint8_t col) {
  gpio_write_pin_high(col_pins[col]);
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
  uint8_t current_col,
  matrix_row_t row_shifter
) {
  bool matrix_has_changed = false;
  bool key_pressed = false;

  // Select col
  if (!select_col(current_col)) { // select col
    return;                       // skip NO_PIN col
  }
  matrix_output_select_delay();

  // For each row...
  for (uint8_t row_index = 0; row_index < ROWS_PER_HAND; row_index++) {
    // Check row pin state
    if (readMatrixPin(row_pins[row_index]) == 0) {
      // Pin LO, set col bit
      current_matrix[row_index] |= row_shifter;
      key_pressed = true;
    } else {
      // Pin HI, clear col bit
      current_matrix[row_index] &= ~row_shifter;
    }
  }

  // Unselect col
  unselect_col(current_col);
  matrix_output_unselect_delay(current_col, key_pressed); // wait for all Row signals to go HIGH

  return matrix_has_changed;
}

static bool matrix_read_cols_on_row (
  matrix_row_t current_matrix[],
  uint8_t current_row
) {
  bool matrix_has_changed = false;
  matrix_row_t current_row_values = 0;
  matrix_row_t row_shifter = MATRIX_ROW_SHIFTER;

  for (
    uint8_t col_index = 0;
    col_index < MATRIX_COLS;
    col_index++, row_shifter <<= 1
  ) {
    pin_t pin = direct_pins[current_row][col_index];
    matrix_row_t current_value_in_row = read_matrix_pin(pin) ? 0 : row_shifter;
    matrix_has_changed |=
      current_value_in_row != current_matrix[current_row] & row_shifter;
    current_row_values |= current_value_in_row;
  }

  // Update the matrix
  current_matrix[current_row] = current_row_values;

  return matrix_has_changed;
}

void matrix_init_custom (void) {
  // TODO: initialize hardware here
  init_pins();
}

bool matrix_scan_custom (matrix_row_t current_matrix[]) {
  bool matrix_has_changed = false;
  matrix_row_t row_shifter = MATRIX_ROW_SHIFTER;

  // TODO: add matrix scanning routine here

  // scan cols
  for (uint8_t row_index = 0; row_index < ROWS_PER_HAND; row_index++) {
    matrix_has_changed |= matrix_read_cols_on_row(current_matrix[], row_index);
  }

  // init rows
  set_matrix_read_rows();

  // scan rows
  for (
    uint8_t col_index = 0;
    col_index < MATRIX_COLS;
    col_index++, row_shifter <<= 1
  ) {
    matrix_has_changed |= matrix_read_rows_on_col(
      current_matrix[],
      col_index,
      row_shifter
    );
  }

  // init cols
  set_matrix_read_cols();

  return matrix_has_changed;
}
