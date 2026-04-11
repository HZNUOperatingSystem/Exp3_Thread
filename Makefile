# platform-specific openmp resolution

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
    ifeq ($(shell brew list libomp >/dev/null 2>&1; echo $$?),1)
        $(error libomp is not installed on macOS. Please run: brew install libomp)
    endif
    LIBOMP_PREFIX := $(shell brew --prefix libomp)
    OMP_CFLAGS  := -I$(LIBOMP_PREFIX)/include -Xpreprocessor -fopenmp
    OMP_LDFLAGS := -L$(LIBOMP_PREFIX)/lib -lomp
else
    OMP_CFLAGS  := -fopenmp
    OMP_LDFLAGS := -fopenmp
endif

# compile flags

CC ?= gcc # alias to clang on macOS
CPPFLAGS ?= -I.
CFLAGS ?= -std=c11 -O2 -Wall -Wextra
LDFLAGS ?=
LDLIBS ?= -lm -pthread

BUILD_DIR ?= build
PTHREAD_FLAGS := -pthread

COMMON_SOURCES := common/dataset.c common/filter.c common/image_io.c common/metrics.c common/pipeline.c common/stb_impl.c

CH1_SOURCES := ch1/main.c $(COMMON_SOURCES)
CH2_SOURCES := ch2/main.c $(COMMON_SOURCES)
CH3_SOURCES := ch3/test_main.c ch3/thread_pool.c
CH4_SOURCES := ch4/main.c ch3/thread_pool.c $(COMMON_SOURCES)
TOOLS_SOURCES := tools/compare_png.c tools/compare_metrics.c

CH1_TARGET := $(BUILD_DIR)/ch1/image_batch
CH2_TARGET := $(BUILD_DIR)/ch2/ssim_batch
CH3_TARGET := $(BUILD_DIR)/ch3/thread_pool_test
CH4_TARGET := $(BUILD_DIR)/ch4/pool_batch
COMPARE_PNG_TARGET := $(BUILD_DIR)/tools/compare_png
COMPARE_METRICS_TARGET := $(BUILD_DIR)/tools/compare_metrics

CH1_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(CH1_SOURCES))
CH2_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(CH2_SOURCES))
CH3_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(CH3_SOURCES))
CH4_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(CH4_SOURCES))
TOOLS_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(TOOLS_SOURCES))

ALL_SOURCES := $(wildcard *.c) $(wildcard ch1/*.c) $(wildcard ch2/*.c) $(wildcard ch3/*.c) $(wildcard ch4/*.c) $(wildcard common/*.c) $(wildcard tools/*.c)
ALL_HEADERS := $(wildcard *.h) $(wildcard ch1/*.h) $(wildcard ch2/*.h) $(wildcard ch3/*.h) $(wildcard ch4/*.h) $(wildcard common/*.h) $(wildcard tools/*.h)

all: ch1 ch2 ch3 ch4 tools

ch1: $(CH1_TARGET)
ch2: $(CH2_TARGET)
ch3: $(CH3_TARGET)
ch4: $(CH4_TARGET)
tools: $(COMPARE_PNG_TARGET) $(COMPARE_METRICS_TARGET)

define LINK_RULE
$(1): $(2)
	@mkdir -p $$(dir $$@)
	$(CC) $$^ $(LDFLAGS) $(LDLIBS) -o $$@
endef

$(eval $(call LINK_RULE,$(CH1_TARGET),$(CH1_OBJECTS)))
$(eval $(call LINK_RULE,$(CH2_TARGET),$(CH2_OBJECTS)))
$(eval $(call LINK_RULE,$(CH3_TARGET),$(CH3_OBJECTS)))
$(eval $(call LINK_RULE,$(CH4_TARGET),$(CH4_OBJECTS)))
$(eval $(call LINK_RULE,$(COMPARE_PNG_TARGET),$(word 1,$(TOOLS_OBJECTS))))
$(eval $(call LINK_RULE,$(COMPARE_METRICS_TARGET),$(word 2,$(TOOLS_OBJECTS))))

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(CH1_OBJECTS): CFLAGS += $(OMP_CFLAGS)
$(CH2_OBJECTS) $(CH3_OBJECTS) $(CH4_OBJECTS): CFLAGS += $(PTHREAD_FLAGS)

$(CH1_TARGET): LDFLAGS += $(OMP_LDFLAGS)
$(CH2_TARGET) $(CH3_TARGET) $(CH4_TARGET): LDFLAGS += $(PTHREAD_FLAGS)

$(BUILD_DIR)/common/stb_impl.o: CFLAGS += -w
$(BUILD_DIR)/tools/compare_png.o: CFLAGS += -w

clean:
	rm -rf $(BUILD_DIR) output image_batch ssim_batch pool_batch thread_pool_test

format:
	clang-format -i $(ALL_SOURCES) $(ALL_HEADERS)

.PHONY: all ch1 ch2 ch3 ch4 tools clean format
