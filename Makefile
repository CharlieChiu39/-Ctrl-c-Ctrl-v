#Compiler
CXX = g++

#Compiler flags (e.g., include paths, warnings)
#We put -I"include" here
CXXFLAGS = -I"include" -Wall -Wextra # Added -Wall -Wextra for more warnings, good practice!

#Linker flags (e.g., library paths)
#We put -L paths here
LDFLAGS = -L"lib" -L"lib_image" -L"lib_mixer" -L"lib_ttf"

#Libraries to link
#We put -l libraries here
LDLIBS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf

#Source files
#List all your .cpp files here
SOURCES = src/main.cpp \
          src/Game.cpp \
          src/Player.cpp \
          src/AnimationData.cpp \
          src/TextureManager.cpp \
          src/AudioManager.cpp

#Object files: Automatically generate .o filenames from .cpp filenames
OBJECTS = $(SOURCES:.cpp=.o)

#Executable name
EXECUTABLE = game.exe

#--- Targets ---
#Default target: Build the executable
#The first target in the file is the default one executed when you just type 'make'
all: $(EXECUTABLE)

#Rule to link the executable
#Depends on all object files
$(EXECUTABLE): $(OBJECTS)
	@echo Linking $@... # Print a message
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)
	@echo Build finished: $@

#Pattern rule to compile .cpp files into .o files
#Creates a .o file from a .cpp file with the same name
%.o: %.cpp
	@echo Compiling $<... # Print a message
	$(CXX) $(CXXFLAGS) -c $< -o $@

#Target to clean up generated files
clean:
	@echo Cleaning up... # Print a message
	rm -f $(OBJECTS) $(EXECUTABLE) # Use rm -f to force remove and ignore errors if files don't exist

#Declare targets that are not actual files
.PHONY: all clean