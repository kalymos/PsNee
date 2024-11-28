# This Makefile can be used to build and upload PSNee on an ATTiny45. It can easily be adapted for
# other AVRs.
# "make upload" will build and upload for ATTiny45
# "make upload ATTINY_NUMBER=85" will build and upload for ATTiny85, etc.

# Change this number to 85 or 25 as needed for ATTiny25 and ATTiny85
ATTINY_NUMBER ?= 45
# Use arduino-cli board details -b $(BOARD) to see other flags.
BOARD = ATTinyCore:avr:attinyx5:chip=$(ATTINY_NUMBER),clock=8internal
CPP_EXTRA_FLAGS = -DATTINY_X5
OUTPUT_DIR = build-t$(ATTINY_NUMBER)

# Use "arduino-cli board list" to find this
SERIAL = /dev/ttyACM0

ATTINYCOREURL = http://drazzy.com/package_drazzy.com_index.json


# This is believed to match how arduino-cli works, i.e. it uses the name of the
# current directory to infer the name of the main .ino file.
PROJECT_FILE = $(notdir $(CURDIR)).ino

# Use 'make DEBUG=1 upload' to build and upload the debug version
DEBUG ?= 0
ifeq ($(DEBUG), 1)
  CPP_EXTRA_FLAGS += -DPSNEEDEBUG
  OUTPUT_DIR = debug-build
endif

all: compile

clean:
	rm -rf $(OUTPUT_DIR)

compile: $(OUTPUT_DIR)/$(PROJECT_FILE).hex

$(OUTPUT_DIR)/$(PROJECT_FILE).hex: $(wildcard *.cpp) $(wildcard *.h) $(PROJECT_FILE)
	arduino-cli compile --build-property compiler.cpp.extra_flags="$(CPP_EXTRA_FLAGS)" --output-dir "$(OUTPUT_DIR)" -b "$(BOARD)" -e

# This worked uploading via ISCP with a Pololu programmer.
# We also set fuses, in particular to unset CKDIV8 to have the full clock speed.
upload: compile
	arduino-cli upload -v --input-dir "$(OUTPUT_DIR)" -P stk500 -b  "$(BOARD)" -p $(SERIAL) -t
	avrdude -c stk500v2 -p t$(ATTINY_NUMBER) -P $(SERIAL) -U lfuse:w:0xE2:m -U hfuse:w:0xDF:m

install-core:
	arduino-cli core update-index --additional-urls "$(ATTINYCOREURL)"
	arduino-cli core install ATTinyCore:avr --additional-urls "$(ATTINYCOREURL)"
