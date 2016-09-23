CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy

CFLAGS = -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
CFLAGS += -Os -ffunction-sections -fdata-sections -MD -std=c99
CFLAGS += -Wall -Wextra -pedantic -DPART_LM4F120H5QR -Iinclude -Dgcc

LDFLAGS = -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

HEADERS  = $(wildcard src/*.h)
ASFILES  = $(wildcard src/*.S)
CFILES   = $(wildcard src/*.c)
CXXFILES = $(wildcard src/*.cc)
OBJECTS  = ${CFILES:src/%.c=build/%.o} ${CXXFILES:src/%.cc=build/%.o} ${ASFILES:src/%.S=build/%.o}

all: build build/main.bin

build:
	mkdir -p build

build/%.o: src/%.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

build/main.axf: ${OBJECTS}
	${LD} -T src/lm4f120h5qr.ld --entry ResetISR --gc-sections -o $@ $^

build/main.bin: build/main.axf
	${OBJCOPY} -O binary $< $@

program: flash

flash: build/main.bin
	lm4flash build/main.bin

clean:
	rm -rf build

.PHONY: all clean flash program
