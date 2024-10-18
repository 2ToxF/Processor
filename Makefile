# modules dirs
STACK_DIR := Stack

# main dirs
SRC_DIR := source
INC_DIR := include
OBJ_DIR := object

# sources in project and submodules
SOURCES   := $(wildcard $(SRC_DIR)/*.cpp)
STACK_SRC := $(wildcard $(STACK_DIR)/$(SRC_DIR)/*.cpp)

# includes in project and submodules
INCLUDES := $(wildcard $(INC_DIR)/*.h) $(wildcard $(STACK_DIR)/$(INC_DIR)/*.h)

# object-files in project and submodules
OBJECTS   := $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
STACK_OBJ := $(STACK_SRC:$(STACK_DIR)/$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
ALL_OBJ   := $(OBJECTS) $(STACK_OBJ)

# name of exe file
EXE := SPU.exe

# name of file with configuration for Doxygen
DOCS_NAME := Docs_config

# flags for compiler
CC := g++
DED_FLAGS := -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code \
	-Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra \
	-Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security \
	-Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor \
	-Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel \
	-Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE
ADD_FLAGS := -mrdrnd
INC_FLAGS := -I$(INC_DIR) -I$(STACK_DIR)/$(INC_DIR)

# -------------------------------------------------------------------------------------------------------------------------

# compile code
all: $(OBJ_DIR) $(EXE)

$(OBJ_DIR):
	@mkdir $@

$(EXE): $(OBJECTS) $(STACK_OBJ)
	@$(CC) $(ALL_OBJ) -o $@

$(OBJECTS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(INCLUDES)
	@$(CC) -c $(DED_FLAGS) $(ADD_FLAGS) $(INC_FLAGS) $< -o $@

$(STACK_OBJ): $(OBJ_DIR)/%.o: $(STACK_DIR)/$(SRC_DIR)/%.cpp $(INCLUDES)
	@$(CC) -c $(DED_FLAGS) $(ADD_FLAGS) $(INC_FLAGS) $< -o $@

# -------------------------------------------------------------------------------------------------------------------------

# run code
run: $(EXE)
	@$(EXE)

# make documentation
docs:
	doxygen $(DOCS_NAME)

# -------------------------------------------------------------------------------------------------------------------------

# clean folders
clean:
	rm $(OBJ_DIR)/*.o
	rm *.exe

clean_exe:
	rm *.exe

clean_obj:
	rm $(OBJ_DIR)/*.o
