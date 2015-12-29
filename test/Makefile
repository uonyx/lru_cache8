CC=clang++
SOURCE=test.cpp
BUILD_DIR=build
CXXFLAGS=-Wall -Wextra -Werror
TARGET=$(BUILD_DIR)/test
all: test 

test: $(SOURCE)
	rm -rf $(BUILD_DIR)
	mkdir $(BUILD_DIR) 
	$(CC) $(SOURCE) $(CXXFLAGS) -o $(TARGET)

clean:
	rm -rf $(BUILD_DIR)
