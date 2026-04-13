# to compile and run in one command type:
# make run

CXX      := g++
OUTPUT   := SparCraft
OS       := $(shell uname)
SRC_DIR  := ./source

# set BWAPI_DIR to your external BWAPI checkout
BWAPI_DIR ?= /where_you_cloned_to/bwapi/bwapi

CXX_FLAGS := -O3 -std=c++23 -Wno-unused-result -Wno-deprecated-declarations
INCLUDES  := -I$(BWAPI_DIR)/include -I$(BWAPI_DIR)/include/BWAPI -I$(BWAPI_DIR) -I$(SRC_DIR)/imgui

ifeq ($(OS), Linux)
    LDFLAGS := -O3 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lGL
endif

ifeq ($(OS), Darwin)
    SFML_DIR ?= /opt/homebrew/Cellar/sfml/3.0.1
    INCLUDES += -I$(SFML_DIR)/include
    LDFLAGS  := -O3 -L$(SFML_DIR)/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -framework OpenGL
endif

SOURCES := \
    $(wildcard $(BWAPI_DIR)/BWAPILIB/Source/*.cpp) \
    $(wildcard $(BWAPI_DIR)/BWAPILIB/*.cpp) \
    $(wildcard $(SRC_DIR)/*.cpp) \
    $(wildcard $(SRC_DIR)/main/*.cpp) \
    $(wildcard $(SRC_DIR)/gui/*.cpp) \
    $(wildcard $(SRC_DIR)/imgui/*.cpp)

OBJECTS := $(SOURCES:.cpp=.o)
DEP_FILES := $(OBJECTS:.o=.d)
-include $(DEP_FILES)

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS) Makefile
	$(CXX) $(OBJECTS) $(LDFLAGS) -o ./bin/$@

%.o: %.cpp
	$(CXX) -MMD -MP -c $(CXX_FLAGS) $(INCLUDES) $< -o $@

clean:
	rm -f $(OBJECTS) $(DEP_FILES) ./bin/$(OUTPUT)

run: $(OUTPUT)
	cd bin && ./$(OUTPUT) ../sample_experiment/sample_exp.txt && cd ..
