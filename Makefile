CXX ?= g++
CPPFLAGS ?= -I.
CXXFLAGS ?= -std=c++11 -O2 -pthread -Wall -Wextra
LDFLAGS ?=
LDLIBS ?=
BUILD_DIR ?= build
TARGET ?= image_filter
SOURCES := $(sort $(wildcard *.cpp))
OBJECTS := $(addprefix $(BUILD_DIR)/,$(SOURCES:.cpp=.o))
FORMAT_FILES := bilateral_filter.h main.cpp stb_impl.cpp

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/stb_impl.o: EXTRA_CXXFLAGS += -w

clean:
	rm -rf $(BUILD_DIR) $(TARGET) teacher_image_filter *.o

format:
	clang-format -i $(FORMAT_FILES)

fmt: format

.PHONY: all clean format fmt
