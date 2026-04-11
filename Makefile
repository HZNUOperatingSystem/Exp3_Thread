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
chapter_sources = $(wildcard src/$(1)/*.c) $(if $(filter $(1),$(IMAGE_CHAPTERS)),$(COMMON_SOURCES)) $(if $(filter ch4,$(1)),src/ch3/thread_pool.c)
chapter_objects = $(patsubst %.c,$(BUILD_DIR)/%.o,$(call chapter_sources,$(1)))
tool_target = $(BUILD_DIR)/tools/$(1)

all: ch1 ch2 ch3 ch4 tools

tools: $(TOOLS)

help:
	@printf '%s\n' 'Common targets: all clean format tools help'
	@printf '%s\n' 'Chapter build targets: $(CHAPTERS)'
	@printf '%s\n' 'Aggregate run target: run-all'
	@printf '%s\n' 'Aggregate grade target: grade-all'
	@printf '%s\n' 'Run helpers: $(RUN_TARGETS)'
	@printf '%s\n' 'Grade helpers: $(GRADE_TARGETS)'
	@printf '%s\n' 'Path helpers: $(PATH_TARGETS)'

run-all: $(RUN_TARGETS)

grade-all:
	$(MAKE) grade-ch1
	$(MAKE) grade-ch2
	$(MAKE) grade-ch3
	$(MAKE) grade-ch4

define DEFINE_CHAPTER_RULES
$(1): $(call chapter_target,$(1))

$(call chapter_target,$(1)): $(call chapter_objects,$(1))
	@mkdir -p $$(dir $$@)
	$(CC) $$^ $(LDFLAGS) $(LDLIBS) $(if $(filter ch1,$(1)),$(OMP_LDFLAGS)) $(if $(filter ch3 ch4,$(1)),$(PTHREAD_LDLIBS)) -o $$@

run-$(1): $(1)
	LAB_CHAPTER=$(1) ./$(call chapter_target,$(1))

grade-$(1): run-$(1)
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

$(BUILD_DIR)/src/ch1/main.o: CFLAGS += $(OMP_CFLAGS)
$(BUILD_DIR)/src/ch3/main.o $(BUILD_DIR)/src/ch3/thread_pool.o $(BUILD_DIR)/src/ch4/main.o: CFLAGS += $(PTHREAD_CFLAGS)

$(BUILD_DIR)/src/common/stb_impl.o: CFLAGS += -w
$(BUILD_DIR)/tools/compare_png.o: CFLAGS += -w

clean:
	rm -rf $(BUILD_DIR) $(GENERATED_OUTPUTS)

format:
	clang-format -i $(FORMAT_FILES)

.PHONY: all tools clean format help run-all grade-all $(CHAPTERS) $(TOOLS) $(RUN_TARGETS) $(GRADE_TARGETS) $(PATH_TARGETS)
