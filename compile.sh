# Tạo file compile.sh
cat > ~/MyGame/compile.sh << 'EOF'
#!/bin/bash

# Lấy cờ biên dịch từ sdl2-config
SDL_CFLAGS=$(sdl2-config --cflags)
SDL_LIBS=$(sdl2-config --libs)

# Biên dịch với các cờ động
g++ -o game src/graphic.cpp src/logic.cpp src/main.cpp $SDL_CFLAGS $SDL_LIBS -lSDL2_ttf -lSDL2_image -lSDL2_mixer -std=c++17

echo "Biên dịch thành công! Chạy game với lệnh ./game"
EOF

# Cấp quyền thực thi
chmod +x ~/MyGame/compile.sh