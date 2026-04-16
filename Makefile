BASH ?= bash
LAB_META := $(BASH) tools/lab_meta.sh
CHAPTERS := $(shell $(LAB_META) list-chapters)
IMAGE_CHAPTERS := $(shell $(LAB_META) list-image-chapters)
TOOLS := compare_png
CHAPTER_BINARY := lab
RUN_TARGETS := $(addprefix run-,$(CHAPTERS))
GRADE_TARGETS := $(addprefix grade-,$(CHAPTERS))
PATH_TARGETS := $(addprefix path-,$(CHAPTERS))
COMMON_SOURCES := $(wildcard src/common/*.c)

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
GENERATED_OUTPUTS := $(addprefix output/,$(IMAGE_CHAPTERS))
chapter_target = $(BUILD_DIR)/$(1)/$(CHAPTER_BINARY)
chapter_extra_sources = $(if $(filter ch4,$(1)),src/ch3/thread_pool.c)
chapter_sources = $(wildcard src/$(1)/*.c) $(if $(filter $(1),$(IMAGE_CHAPTERS)),$(COMMON_SOURCES)) $(call chapter_extra_sources,$(1))
chapter_objects = $(patsubst %.c,$(BUILD_DIR)/%.o,$(call chapter_sources,$(1)))
chapter_link_libs = $(if $(filter ch1,$(1)),$(OMP_LDFLAGS)) $(if $(filter ch3 ch4,$(1)),$(PTHREAD_LDLIBS)) $(if $(filter ch4x,$(1)),$(ONNX_LDFLAGS))
chapter_run_prefix = $(if $(filter $(1),$(IMAGE_CHAPTERS)),LAB_CHAPTER=$(1) ,)
tool_target = $(BUILD_DIR)/tools/$(1)

all: ch1 ch2 ch3 ch4 ch4x tools

tools: $(TOOLS)

help:
	@printf '%s\n' 'Common targets: all clean format tools help'
	@printf '%s\n' 'Chapter build targets: $(CHAPTERS)'
	@printf '%s\n' 'Run helpers: $(RUN_TARGETS) run-all'
	@printf '%s\n' 'Aggregate grade target: grade-all'
	@printf '%s\n' 'Grade helpers: $(GRADE_TARGETS)'
	@printf '%s\n' 'Path helpers: $(PATH_TARGETS)'

run-all:
	@set -e; for chapter in $(CHAPTERS); do $(MAKE) run-$$chapter; done

grade-all:
	@set -e; for chapter in $(CHAPTERS); do $(MAKE) grade-$$chapter; done

define DEFINE_CHAPTER_RULES
$(1): $(call chapter_target,$(1))

$(call chapter_target,$(1)): $(call chapter_objects,$(1))
	@mkdir -p $$(dir $$@)
	$(CC) $$^ $(LDFLAGS) $(LDLIBS) $(call chapter_link_libs,$(1)) -o $$@

run-$(1): $(1)
	$(call chapter_run_prefix,$(1))./$(call chapter_target,$(1))

grade-$(1): $(1) $(if $(filter $(1),$(IMAGE_CHAPTERS)),tools)
	$(BASH) ./autograde.sh $(1)

path-$(1):
	@printf '%s\n' '$(call chapter_target,$(1))'
endef

$(foreach chapter,$(CHAPTERS),$(eval $(call DEFINE_CHAPTER_RULES,$(chapter))))

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
$(BUILD_DIR)/src/ch4x/onnx_inference.o: CFLAGS += $(ONNX_CFLAGS)

$(BUILD_DIR)/src/common/stb_impl.o: CFLAGS += -w
$(BUILD_DIR)/tools/compare_png.o: CFLAGS += -w

clean:
	rm -rf $(BUILD_DIR) $(GENERATED_OUTPUTS)

format:
	clang-format -i $(FORMAT_FILES)

SUBMIT_ARCHIVE := submit.tar.gz

submit:
	tar czf $(SUBMIT_ARCHIVE) src/

.PHONY: all tools clean format help grade-all submit $(CHAPTERS) $(TOOLS) $(RUN_TARGETS) $(GRADE_TARGETS) $(PATH_TARGETS)
