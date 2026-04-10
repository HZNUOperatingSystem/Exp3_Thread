CC ?= gcc
CPPFLAGS ?= -I.
CFLAGS ?= -std=c11 -O2 -Wall -Wextra
LDFLAGS ?=
LDLIBS ?= -lm -pthread
OMPFLAGS ?= -fopenmp
PTHREAD_FLAGS ?= -pthread
BUILD_DIR ?= build

COMMON_SOURCES := common/dataset.c common/filter.c common/image_io.c common/metrics.c common/pipeline.c common/stb_impl.c

CH1_SOURCES := ch1/main.c $(COMMON_SOURCES)
CH2_SOURCES := ch2/test_main.c ch2/thread_pool.c
CH3_SOURCES := ch3/main.c ch2/thread_pool.c $(COMMON_SOURCES)
TOOLS_SOURCES := tools/compare_png.c

CH1_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/ch1/%.o,$(CH1_SOURCES))
CH2_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/ch2/%.o,$(CH2_SOURCES))
CH3_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/ch3/%.o,$(CH3_SOURCES))
TOOLS_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/tools/%.o,$(TOOLS_SOURCES))

CH1_TARGET := $(BUILD_DIR)/ch1/image_batch
CH2_TARGET := $(BUILD_DIR)/ch2/thread_pool_test
CH3_TARGET := $(BUILD_DIR)/ch3/pool_batch
COMPARE_PNG_TARGET := $(BUILD_DIR)/tools/compare_png

FORMAT_FILES := ch1/main.c ch2/thread_pool.c ch2/thread_pool.h ch2/test_main.c ch3/main.c common/dataset.c common/dataset.h common/filter.c common/filter.h common/image_io.c common/image_io.h common/metrics.c common/metrics.h common/pipeline.c common/pipeline.h common/stb_impl.c tools/compare_png.c

all: ch1 ch2 ch3 tools

ch1: $(CH1_TARGET)

ch2: $(CH2_TARGET)

ch3: $(CH3_TARGET)

tools: $(COMPARE_PNG_TARGET)

$(CH1_TARGET): $(CH1_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CH1_OBJECTS) $(LDFLAGS) $(OMPFLAGS) $(LDLIBS) -o $@

$(CH2_TARGET): $(CH2_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CH2_OBJECTS) $(LDFLAGS) $(PTHREAD_FLAGS) $(LDLIBS) -o $@

$(CH3_TARGET): $(CH3_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CH3_OBJECTS) $(LDFLAGS) $(PTHREAD_FLAGS) $(LDLIBS) -o $@

$(COMPARE_PNG_TARGET): $(TOOLS_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(TOOLS_OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR)/ch1/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(OMPFLAGS) $(EXTRA_CFLAGS) -c $< -o $@

$(BUILD_DIR)/ch2/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PTHREAD_FLAGS) $(EXTRA_CFLAGS) -c $< -o $@

$(BUILD_DIR)/ch3/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PTHREAD_FLAGS) $(EXTRA_CFLAGS) -c $< -o $@

$(BUILD_DIR)/tools/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -c $< -o $@

$(BUILD_DIR)/ch1/common/stb_impl.o: EXTRA_CFLAGS += -w
$(BUILD_DIR)/ch3/common/stb_impl.o: EXTRA_CFLAGS += -w
$(BUILD_DIR)/tools/tools/compare_png.o: EXTRA_CFLAGS += -w

clean:
	rm -rf $(BUILD_DIR) output image_batch pool_batch thread_pool_test

format:
	clang-format -i $(FORMAT_FILES)

.PHONY: all ch1 ch2 ch3 tools clean format
