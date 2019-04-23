INCLUDE_DIR = inc
BUILD_DIR = build
BIN_DIR = bin
SRC_DIR = src
BIN_NAME = drive_buggy

CC=g++
CDEFS=
CFLAGS= -O0 -g -Wall $(CDEFS)
LIB_DIRS = -L/usr/local/lib
LIBS= -lrt -pthread
CPPLIBS= -lopencv_core -lopencv_flann -lopencv_video

$(BUILD_DIR)/$(BIN_NAME): $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(wildcard $(SRC_DIR)/*.c))
	$(CC) $^ -o $(BIN_DIR)/$(BIN_NAME) $(CFLAGS) $(LIB_DIRS) $(LIBS) $(CPPLIBS)

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c $(INCLUDE_DIR)/%.h | $(BUILD_DIR) $(BIN_DIR)
	$(CC) -I $(INCLUDE_DIR) -c $< -o $@ $(CFLAGS) $(LIB_DIRS) $(LIBS) $(CPPLIBS)

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c | $(BUILD_DIR) $(BIN_DIR)
	$(CC) -I $(INCLUDE_DIR) -c $< -o $@ $(CFLAGS) $(LIB_DIRS) $(LIBS) $(CPPLIBS)

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.cpp $(INCLUDE_DIR)/%.hpp | $(BUILD_DIR) $(BIN_DIR)
	$(CC) -I $(INCLUDE_DIR) -c $< -o $@ $(CFLAGS) $(LIB_DIRS) $(LIBS) $(CPPLIBS)

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.cpp | $(BUILD_DIR) $(BIN_DIR)
	$(CC) -I $(INCLUDE_DIR) -c $< -o $@ $(CFLAGS) $(LIB_DIRS) $(LIBS) $(CPPLIBS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
		rm -rf build bin

.PHONY: clean
