# ergomacs

![ergomacs](imgur.com image replace me!)

Ergomacs is a split ortholinear keyboard with 4x6 keywells, 5-key thumb clusters, and a 2u vertical enter. The keyboard was designed by Swan to be ergonomically optimized for Emacs. See the [project writeup](Add link here) for more information.

* Keyboard Maintainer: [Swan](https://github.com/pitworker)
* Hardware Supported: Raspberry Pi Pico (RP2040)
* Hardware Availability: This keyboard is handwired. More information about hardware and design can be found in the project writeup.

Make example for this keyboard (after setting up your build environment):

    make ergomacs:default

Flashing example for this keyboard:

    make ergomacs:default:flash

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in 3 ways:

* **Bootmagic reset**: Hold down the key at (0,0) in the matrix (usually the top left key or Escape) and plug in the keyboard
* **Physical reset button**: Briefly press the button on the back of the PCB - some may have pads you must short instead
* **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available
