# Specify the final target name
EXE := res

# Specify the source files
# Effectively list all source files in the current directory
SRC := $(wildcard *.cpp)

# From the source file list, get the corresponding object file list
# This is a clearer syntax for $(patsubst %.cpp,%.o,$(SRC))
OBJ := $(SRC:.cpp=.o)

# From the object file list, get the dependency file list to handle automatic
# recompilation when a header file is modified
DEP := $(OBJ:.o=.d)

# Specify preprocessor flags (this is a built-in variable)
CPPFLAGS := -I../include
# Required flags to enable the automatic dependency generation by the compiler
CPPFLAGS += -MMD -MP

# Specify compiler flags (this is a built-in variable)
# Here some basic warning flags
CXXFLAGS := -std=c++20 -O2

# Specify linker flags (this is a built-in variable)
LDFLAGS := -L../lib

# Specify linker libraries (this is a built-in variable)
# m is the maths library
LDLIBS := -lm

# Tell make that these target are not real files
.PHONY: all clean

# Now the standard primary rule
all: $(EXE)

# How do we make $(EXE) ? Remember the recipe describe the linking phase
$(EXE): $(OBJ)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Let's clean up the mess
clean:
	$(RM) $(EXE) $(OBJ) $(DEP)

# Don't forget to include the dependency files to let make know when to recompile
-include $(DEP)