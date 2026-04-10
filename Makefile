CC ?= gcc
CPPFLAGS ?= -I.
CFLAGS ?= -std=c11 -O2 -Wall -Wextra
LDFLAGS ?=
LDLIBS ?= -lm -pthread
PTHREAD_FLAGS ?= -pthread
BUILD_DIR ?= build
TARGET ?= pool_batch
COMMON_SOURCES := common/dataset.c common/filter.c common/image_io.c common/metrics.c common/pipeline.c common/stb_impl.c
SOURCES := main.c thread_pool.c $(COMMON_SOURCES)
OBJECTS := $(addprefix $(BUILD_DIR)/,$(SOURCES:.c=.o))
FORMAT_FILES := main.c thread_pool.c thread_pool.h $(COMMON_SOURCES)

CFLAGS += $(PTHREAD_FLAGS)
LDFLAGS += $(PTHREAD_FLAGS)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -c $< -o $@

$(BUILD_DIR)/common/stb_impl.o: EXTRA_CFLAGS += -w

clean:
	rm -rf $(BUILD_DIR) $(TARGET) output metrics.csv

format:
	clang-format -i $(FORMAT_FILES)

.PHONY: all clean format
