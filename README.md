## ATtiny85 UV sensor

The project uses LTR390 UV and ambient light sensor to display UV index on SSD1306 OLED screen.

### Quickstart

```console
just install
just build
just flash
```

### Board

The project uses Digispark ATtiny85 USB development board. It comes with the preinstalled
micronucleus bootloader. The project assumes the custom bootloader is flashed with version 2.6 and
FAST_EXIT_NO_USB_MS=300. The custom board [definition](`./boards/attiny85new.json`) is used to make
use of the extra free flash memory, edit it accordingly.

Also see the extensive [guide](https://gist.github.com/Ircama/22707e938e9c8f169d9fe187797a2a2c) on
configuring the board.

### Wiring

```
// SDA - PB0
// SCl - PB2
```

### `"Wire.h"`

The project uses TinyWireM library. The headers are renamed to make LTR390 library work.

### Why deep sleep?

Current consumption at 3V is 15-16 mA with using `delay` and with the deep sleep it's 9-10 mA with
peaks up to 13 mA to refresh the screen.

### LSP support

`compile_commands.json` can be created for clang LSP with the following command:

```console
just compile_commands
```

### Links

- https://esphome.io/components/sensor/ltr390.html
- https://github.com/micronucleus/micronucleus
- https://github.com/datacute/Tiny4kOLED
- https://github.com/adafruit/TinyWireM
- https://github.com/connornishijima/TinySnore
- https://web.archive.org/web/20240225173937/https://digistump.com/wiki/digispark/quickref

### TODO

- [ ] Measure power-down wakeup time
- [ ] Add binary release workflow
