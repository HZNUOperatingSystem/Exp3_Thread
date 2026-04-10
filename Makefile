CC ?= gcc
CPPFLAGS ?= -I.
CFLAGS ?= -std=c11 -O2 -Wall -Wextra
LDFLAGS ?=
LDLIBS ?= -pthread
PTHREAD_FLAGS ?= -pthread
BUILD_DIR ?= build
TARGET ?= thread_pool_test
SOURCES := test_main.c thread_pool.c
OBJECTS := $(addprefix $(BUILD_DIR)/,$(SOURCES:.c=.o))
FORMAT_FILES := test_main.c thread_pool.c thread_pool.h

CFLAGS += $(PTHREAD_FLAGS)
LDFLAGS += $(PTHREAD_FLAGS)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

format:
	clang-format -i $(FORMAT_FILES)

.PHONY: all clean format
