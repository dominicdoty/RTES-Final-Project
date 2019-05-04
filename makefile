INCLUDE_DIR = inc
BUILD_DIR = build
BIN_DIR = bin
SRC_DIR = src
BIN_NAME = drive_buggy

CC=g++
CXX=g++
CDEFS=
CFLAGS= -O0 -g -Wall $(CDEFS)
LIB_DIRS = -L /usr/local/lib
LIBS= -lrt -pthread -lpigpio
CPPLIBS= -lopencv_core -lopencv_flann -lopencv_video -lopencv_highgui -lopencv_imgproc

C_SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
CPP_SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)

C_OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRC_FILES))
CPP_OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.opp,$(CPP_SRC_FILES))

INCLUDES = -I $(INCLUDE_DIR) -I /usr/local/include

OBJS = $(C_OBJ_FILES) $(CPP_OBJ_FILES)

all: $(BUILD_DIR)/$(BIN_NAME)

$(BUILD_DIR)/$(BIN_NAME): $(OBJS)
	$(CC) $^ -o $(BIN_DIR)/$(BIN_NAME) $(CFLAGS) $(LIB_DIRS) $(LIBS) $(CPPLIBS)

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c | $(BUILD_DIR) $(BIN_DIR)
	$(CC) $(INCLUDES) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/%.opp : $(SRC_DIR)/%.cpp | $(BUILD_DIR) $(BIN_DIR)
	$(CC) $(INCLUDES) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf build bin

.PHONY: clean all

