CXX      := em++
CXXFLAGS := -std=c++11 -Wall -Wextra -O3 -msimd128

# -sALLOW_MEMORY_GROWTH
# USE_PTHREADS + ALLOW_MEMORY_GROWTH may run non-wasm code slowly, see https://github.com/WebAssembly/design/issues/1271

LDFLAGS  := --bind -lembind -pthread -lwebsocket.js -sTOTAL_MEMORY=128MB --profiling -sNO\_FILESYSTEM=1 -flto -s"ENVIRONMENT=web,worker" -sPTHREAD_POOL_SIZE=8 
LDFLAGS += -L./libs/opencv/ ./libs/opencv/libopencv_features2d.a ./libs/opencv/libopencv_core.a ./libs/opencv/libopencv_imgproc.a 
BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
APP_DIR  := $(BUILD)/wasm
TARGET   := main.js
INCLUDE  := -I./include/
SRC      :=                     \
   $(wildcard src/util/*.cc)    \
   $(wildcard src/feature/*.cc) \
   $(wildcard src/*.cc)         \

OBJECTS  := $(SRC:%.cc=$(OBJ_DIR)/%.o)
DEPENDENCIES := $(OBJECTS:.o=.d)

all: build $(APP_DIR)/$(TARGET)

$(OBJ_DIR)/%.o: %.cc
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -MMD -o $@

$(APP_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(APP_DIR)/$(TARGET) $^ $(LDFLAGS)

-include $(DEPENDENCIES)

.PHONY: all build clean debug release info

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: all

release: CXXFLAGS += -O2
release: all

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*