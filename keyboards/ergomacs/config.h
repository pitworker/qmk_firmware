// Copyright 2023 Swan Carpenter (@pitworker)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/* diode direction */
#define DIODE_DIRECTION CUSTOM_MATRIX

/* key matrix size */

// Each of 2 hands has 5 dedicated rows (4 main rows, 1 row for thumb cluster),
// so 2 * 5 = 10 rows total
#define MATRIX_ROWS 5 * 2

// Using a duplex matrix, each of 3 ports accounts for 2 columns, with the same
// columns read across both hands, so 3 * 2 = 6 columns total
#define MATRIX_COLS 3 * 2

// RP Pico GPIO pinouts for rows; left hand GP0-GP4, right hand GP8-GP12
#define MATRIX_ROW_PINS { GP0, GP1, GP2, GP3, GP4, GP8, GP9, GP10, GP11, GP12 }

// RP Pico GPIO pinouts for columns; note duplicated pinouts for duplex matrix
// Also note that the left/right column pinouts mirror each other
#define MATRIX_COL_PINS { GP7, GP6, GP5, GP5, GP6, GP7 }

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
