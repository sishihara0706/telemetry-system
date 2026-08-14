CC := cc
CFLAGS := -Wall -Wextra -Werror

BUILD_DIR := build
SERVER := $(BUILD_DIR)/telemetry_server
SIMULATOR := $(BUILD_DIR)/device_simulator

.PHONY: all clean

all: $(SERVER) $(SIMULATOR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(SERVER): src/server/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(SIMULATOR): src/simulator/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(BUILD_DIR)
