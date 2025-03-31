default: build

build:
    pio run

flash:
    pio run -t upload

install:
    pio pkg install
    pio run -t compiledb

compile_commands:
    pio run -t compiledb
