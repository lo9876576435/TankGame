# Trình biên dịch và cờ
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
INCLUDES = -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib
LIBS = -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer

# Thư mục
SRC_DIR = src
OBJ_DIR = build

# Tập tin
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
TARGET = game

# Mục tiêu mặc định
all: $(TARGET)

# Tạo thực thi
$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

# Biên dịch từng file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Dọn dẹp
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Chạy
run: all
	./$(TARGET)

.PHONY: all clean run
