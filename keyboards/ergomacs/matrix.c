__attribute__((weak)) void matrix_init_kb(void) { matrix_init_user(); }
__attribute__((weak)) void matrix_scan_kb(void) { matrix_scan_user(); }
__attribute__((weak)) void matrix_init_user(void) {}
__attribute__((weak)) void matrix_scan_user(void) {}

matrix_row_t matrix_get_row(uint8_t row) {
  // TODO: implement MATRIX_MASKED
  return matrix[row];
}

void matrix_print(void) {
  // TODO: use print() to dump the current matrix state to console
  print_matrix_header();

  for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
    print_hex8(row);
    print(": ");
    print_matrix_row(row);
    print("\n");
  }
}

void matrix_init(void) {
  // TODO: initialize hardware and global matrix state here

  // Unless hardware debouncing - Init the configured debounce routine
  debounce_init(MATRIX_ROWS);

  // This *must* be called for correct keyboard behavior
  matrix_init_kb();
}

uint8_t matrix_scan(void) {
  bool changed = false;

  // TODO: add matrix scanning routine here

  // Unless hardware debouncing - use the configured debounce routine
  changed = debounce(raw_matrix, matrix, MATRIX_ROWS, changed);

  // This *must* be called for correct keyboard behavior
  matrix_scan_kb();

  return changed;
}
