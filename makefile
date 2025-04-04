# Tạo Makefile cải tiến
cat > ~/MyGame/Makefile << 'EOF'
# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

# Sử dụng sdl2-config để lấy cờ biên dịch
SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LIBS := $(shell sdl2-config --libs)

# Thêm các thư viện SDL2 khác
LIBS = $(SDL_LIBS) -lSDL2_ttf -lSDL2_image -lSDL2_mixer

# Thư mục
SRC_DIR = src
BUILD_DIR = build
RELEASE_DIR = release

# File nguồn và đối tượng
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
EXECUTABLE = game

# Mục tiêu mặc định
all: $(BUILD_DIR) $(RELEASE_DIR) $(RELEASE_DIR)/$(EXECUTABLE)

# Tạo thư mục build
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Tạo thư mục release
$(RELEASE_DIR):
	mkdir -p $(RELEASE_DIR)
	mkdir -p $(RELEASE_DIR)/assets/images
	mkdir -p $(RELEASE_DIR)/assets/sound
	mkdir -p $(RELEASE_DIR)/assets/font
	cp -r assets/* $(RELEASE_DIR)/assets/ 2>/dev/null || :

# Biên dịch file .cpp thành .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -c $< -o $@

# Liên kết các file .o thành executable
$(RELEASE_DIR)/$(EXECUTABLE): $(OBJECTS)
	$(CXX) $^ -o $@ $(LIBS)

# Dọn dẹp
clean:
	rm -rf $(BUILD_DIR) $(RELEASE_DIR)/$(EXECUTABLE)

# Chạy game
run: all
	cd $(RELEASE_DIR) && ./$(EXECUTABLE)

# Tạo ứng dụng macOS (.app)
app: all
	mkdir -p MyGame.app/Contents/MacOS
	mkdir -p MyGame.app/Contents/Resources/assets
	cp -r $(RELEASE_DIR)/$(EXECUTABLE) MyGame.app/Contents/MacOS/
	cp -r assets/* MyGame.app/Contents/Resources/assets/
	
	# Tạo Info.plist
	echo '<?xml version="1.0" encoding="UTF-8"?>' > MyGame.app/Contents/Info.plist
	echo '<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">' >> MyGame.app/Contents/Info.plist
	echo '<plist version="1.0">' >> MyGame.app/Contents/Info.plist
	echo '<dict>' >> MyGame.app/Contents/Info.plist
	echo '    <key>CFBundleExecutable</key>' >> MyGame.app/Contents/Info.plist
	echo '    <string>game</string>' >> MyGame.app/Contents/Info.plist
	echo '    <key>CFBundleIdentifier</key>' >> MyGame.app/Contents/Info.plist
	echo '    <string>com.yourdomain.mygame</string>' >> MyGame.app/Contents/Info.plist
	echo '    <key>CFBundleName</key>' >> MyGame.app/Contents/Info.plist
	echo '    <string>MyGame</string>' >> MyGame.app/Contents/Info.plist
	echo '    <key>CFBundlePackageType</key>' >> MyGame.app/Contents/Info.plist
	echo '    <string>APPL</string>' >> MyGame.app/Contents/Info.plist
	echo '</dict>' >> MyGame.app/Contents/Info.plist
	echo '</plist>' >> MyGame.app/Contents/Info.plist

.PHONY: all clean run app
EOF