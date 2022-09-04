CC := g++
CFLAGS := -Wall -g
EXE := z80boy
TEST_EXE := z80boy_memtest
SRC_DIR := src
TEST_FILES := $(SRC_DIR)/memorytest.cpp
SRC_FILES := $(filter-out $(TEST_FILES), $(wildcard $(SRC_DIR)/*.cpp))

$(EXE): $(SRC_FILES)
	$(CC) $(CFLAGS) -o $@ $^

memtest: $(TEST_FILES)
	$(CC) $(CFLAGS) -o $(TEST_EXE) $(TEST_FILES) && ./$(TEST_EXE)

.PHONY: clean

clean:
	$(RM) $(TARGET)
