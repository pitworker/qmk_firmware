// Copyright 2023 Swan (@pitworker)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/* diode direction */
// #define DIODE_DIRECTION CUSTOM_MATRIX
// I don't think I need this ^ (?)

/* key matrix size */

// Rows span both hands, with four keywell rows and one dedicated thumb cluster
// row, so 4 + 1 = 5 rows total
#define MATRIX_ROWS 5

// Each hand has six columns, but every pair of columns shares a pin due to the
// duplex matrix design, so 2 * 6 / 2 = 6 matrix columns total
#define MATRIX_COLS 6

// Due to the paired matrix columns, the actual keyboard design has twice as
// many actual columns as exist in the matrix, so 6 * 2 = 12
#define DESIGN_COLS 12

// RP Pico GPIO pinouts for rows
// Rows ordered bottom-top
#define MATRIX_ROW_PINS { GP3, GP4, GP5, GP6, GP7 }

// RP Pico GPIO pinouts for columns
// Lefthand GP0-GP2, righthand GP11-GP12
// Columns (both hands) ordered left-right
// Using a duplex (2-way) matrix, each column in the electrical layout accounts
// for two columns in the actual matrix, which is why there are 12 columns but
// only 6 column pins.
#define MATRIX_COL_PINS { GP0, GP1, GP2, GP10, GP11, GP12 }

/*
 * Feature disable options
 *  These options are also useful to firmware size reduction.
 */

/* disable debug print */
//#define NO_DEBUG

/* disable print */
//#define NO_PRINT

/* disable action features */
//#define NO_ACTION_LAYER
//#define NO_ACTION_TAPPING
//#define NO_ACTION_ONESHOT
