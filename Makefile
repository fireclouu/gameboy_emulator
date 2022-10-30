CC := g++
INCLUDE := include/
CFLAGS := -Wall -I $(INCLUDE)
EXE := gbemuv2
SRC_DIR := src
SRC_FILES := $(filter-out $(TEST_FILES), $(wildcard $(SRC_DIR)/*.cpp))

$(EXE): $(SRC_FILES)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean

clean:
	$(RM) $(TARGET)
