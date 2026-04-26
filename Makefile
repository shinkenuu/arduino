BOARD = mega
PORT ?= /dev/ttyACM0
SKETCH ?= plantai
BAUD_RATE ?= 115200

.PHONY: compile upload monitor boards clean help install-cores

install-cores:
	@arduino-cli core install arduino:avr && arduino-cli lib update-all

compile:
	@arduino-cli compile -b arduino:avr:$(BOARD) $(SKETCH)

upload: compile
	@echo "Uploading to $(PORT)..."
	@arduino-cli upload -b arduino:avr:$(BOARD) -p $(PORT) $(SKETCH)

monitor:
	@arduino-cli monitor -p $(PORT) --config baudrate=$(BAUD_RATE)

boards:
	@arduino-cli board list

help:
	@echo "Available sketches:"
	@ls -d */ | sed 's|/||' | sort
	@echo ""
	@echo "Usage:"
	@echo "  make compile SKETCH=<name>           Compile sketch"
	@echo "  make upload SKETCH=<name>            Compile and upload"
	@echo "  make upload SKETCH=<name> PORT=/dev/ttyUSB0"
	@echo "  make monitor                         Open serial monitor"
	@echo "  make boards                         List connected boards"
	@echo "  make clean                         Remove temp files on Pi"
	@echo ""
	@echo "Defaults: SKETCH=$(SKETCH), PORT=$(PORT)"
