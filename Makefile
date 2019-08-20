ARTIFACT = AoE
TARGET = $(OUTPUT_DIR)/$(ARTIFACT)

LIBS = -L./SFML/lib -lsfml-graphics -lsfml-window -lsfml-system
INCS = -IInc
CXX = g++
LD = $(CXX)
CCFLAGS = -g -Wall
OUTPUT_DIR = build
SOURCE_DIR := Sources

SOURCES := $(wildcard $(SOURCE_DIR)/*.cpp)
OBJECTS = $(addprefix $(OUTPUT_DIR)/,$(patsubst %.cpp, %.o, $(wildcard *.cpp)))
OBJECTS += $(patsubst $(SOURCE_DIR)/%.cpp,$(OUTPUT_DIR)/%.o,$(SOURCES))

.PHONY: default all clean

default: $(TARGET)
all: default

# Compile files in /Src
$(OUTPUT_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CCFLAGS) $(INCS) -c $< -o $@

# Compile main.cpp
$(OUTPUT_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CCFLAGS) $(INCS) -c $< -o $@

# Link all object files
$(TARGET): $(OBJECTS)
	$(LD) $(OBJECTS) -Wall $(LIBS) $(INCS) -o $@

clean:
	-rm -f $(OUTPUT_DIR)/*.o
	-rm -f $(TARGET)

rebuild: clean all