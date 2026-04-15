BASH ?= bash
LAB_META := $(BASH) tools/lab_meta.sh
CHAPTERS := $(shell $(LAB_META) list-chapters)
IMAGE_CHAPTERS := $(shell $(LAB_META) list-image-chapters)
EXTRA_CHAPTERS := ch4x
TOOLS := compare_png
CHAPTER_BINARY := lab
GRADE_TARGETS := $(addprefix grade-,$(CHAPTERS))
PATH_TARGETS := $(addprefix path-,$(CHAPTERS))
EXTRA_PATH_TARGETS := $(addprefix path-,$(EXTRA_CHAPTERS))
COMMON_SOURCES := $(filter-out src/common/onnx_inference.c,$(wildcard src/common/*.c))

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

# onnx runtime platform detection
ONNX_DETECT := $(BASH) tools/onnx_detect.sh
ONNX_CFLAGS := $(shell $(ONNX_DETECT) cflags)
ONNX_LDFLAGS := $(shell $(ONNX_DETECT) ldflags)

# compile flags

CC ?= gcc # alias to clang on macOS
CPPFLAGS ?= -Isrc -Ithird_party
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wno-unused-variable
LDFLAGS ?=
LDLIBS ?= -lm

BUILD_DIR ?= build
PTHREAD_CFLAGS := -pthread
PTHREAD_LDLIBS := -pthread

FORMAT_FILES := $(shell find src tools -type f \( -name '*.c' -o -name '*.h' \) -print)
GENERATED_OUTPUTS := $(addprefix output/,$(IMAGE_CHAPTERS)) $(addprefix output/,$(EXTRA_CHAPTERS))
chapter_target = $(BUILD_DIR)/$(1)/$(CHAPTER_BINARY)
chapter_sources = $(wildcard src/$(1)/*.c) $(if $(filter $(1),$(IMAGE_CHAPTERS)),$(COMMON_SOURCES)) $(if $(filter ch4,$(1)),src/ch3/thread_pool.c src/common/onnx_inference.c)
chapter_objects = $(patsubst %.c,$(BUILD_DIR)/%.o,$(call chapter_sources,$(1)))
tool_target = $(BUILD_DIR)/tools/$(1)
ch4x_target = $(BUILD_DIR)/ch4x/$(CHAPTER_BINARY)
ch4x_sources = $(wildcard src/ch4x/*.c) $(COMMON_SOURCES) src/common/onnx_inference.c
ch4x_objects = $(patsubst %.c,$(BUILD_DIR)/%.o,$(ch4x_sources))

all: ch1 ch2 ch3 ch4 ch4x tools

tools: $(TOOLS)

help:
	@printf '%s\n' 'Common targets: all clean format tools help'
	@printf '%s\n' 'Chapter build targets: $(CHAPTERS)'
	@printf '%s\n' 'Extra build targets: $(EXTRA_CHAPTERS)'
	@printf '%s\n' 'Aggregate grade target: grade-all'
	@printf '%s\n' 'Grade helpers: $(GRADE_TARGETS)'
	@printf '%s\n' 'Path helpers: $(PATH_TARGETS)'
	@printf '%s\n' 'Extra path helpers: $(EXTRA_PATH_TARGETS)'

grade-all:
	$(MAKE) grade-ch1
	$(MAKE) grade-ch2
	$(MAKE) grade-ch3
	$(MAKE) grade-ch4

define DEFINE_CHAPTER_RULES
$(1): $(call chapter_target,$(1))

$(call chapter_target,$(1)): $(call chapter_objects,$(1))
	@mkdir -p $$(dir $$@)
	$(CC) $$^ $(LDFLAGS) $(LDLIBS) $(if $(filter ch1,$(1)),$(OMP_LDFLAGS)) $(if $(filter ch3 ch4,$(1)),$(PTHREAD_LDLIBS)) $(if $(filter ch4,$(1)),$(ONNX_LDFLAGS)) -o $$@

grade-$(1): $(1) $(if $(filter $(IMAGE_CHAPTERS),$(1)),tools)
	$(BASH) ./autograde.sh $(1)

path-$(1):
	@printf '%s\n' '$(call chapter_target,$(1))'
endef

$(foreach chapter,$(CHAPTERS),$(eval $(call DEFINE_CHAPTER_RULES,$(chapter))))

ch4x: $(ch4x_target)

$(ch4x_target): $(ch4x_objects)
	@mkdir -p $(dir $@)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) $(ONNX_LDFLAGS) -o $@

run-ch4x: ch4x
	LAB_CHAPTER=ch4x ./$(ch4x_target)

path-ch4x:
	@printf '%s\n' '$(ch4x_target)'

define DEFINE_TOOL_RULES
$(1): $(call tool_target,$(1))

$(call tool_target,$(1)): $(BUILD_DIR)/tools/$(1).o
	@mkdir -p $$(dir $$@)
	$(CC) $$^ $(LDFLAGS) $(LDLIBS) -o $$@
endef

$(foreach tool,$(TOOLS),$(eval $(call DEFINE_TOOL_RULES,$(tool))))

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Target-specific flags (must be defined before the rules are expanded)
$(BUILD_DIR)/src/ch1/main.o: CFLAGS += $(OMP_CFLAGS)
$(BUILD_DIR)/src/ch3/main.o $(BUILD_DIR)/src/ch3/thread_pool.o $(BUILD_DIR)/src/ch4/main.o: CFLAGS += $(PTHREAD_CFLAGS)
$(BUILD_DIR)/src/common/onnx_inference.o: CFLAGS += $(ONNX_CFLAGS)
$(BUILD_DIR)/src/ch4x/filter_cnn.o: CFLAGS += $(ONNX_CFLAGS)

$(BUILD_DIR)/src/common/stb_impl.o: CFLAGS += -w
$(BUILD_DIR)/tools/compare_png.o: CFLAGS += -w

clean:
	rm -rf $(BUILD_DIR) $(GENERATED_OUTPUTS)

format:
	clang-format -i $(FORMAT_FILES)

.PHONY: all tools clean format help grade-all ch4x $(CHAPTERS) $(TOOLS) $(GRADE_TARGETS) $(PATH_TARGETS) $(EXTRA_PATH_TARGETS)
