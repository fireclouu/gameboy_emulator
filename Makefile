CC := g++
INCLUDE := include/
CFLAGS := -Wall -g -I $(INCLUDE)
EXE := gbemuv2
TEST_EXE := gbemuv2_memtest
SRC_DIR := src
TEST_FILES := $(SRC_DIR)/memorytest.cpp
SRC_FILES := $(filter-out $(TEST_FILES), $(wildcard $(SRC_DIR)/*.cpp))

$(EXE): $(SRC_FILES)
	$(CC) $(CFLAGS) -o $@ $^

test: $(TEST_FILES)
	$(CC) $(CFLAGS) -o $(TEST_EXE) $(TEST_FILES) && ./$(TEST_EXE)

.PHONY: clean

clean:
	$(RM) $(TARGET)
