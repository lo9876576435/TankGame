# Tạo file README.md
cat > ~/MyGame/README.md << 'EOF'
# Tank Star Game

![Game Screenshot](screenshots/game.png)

Tank Star Game là một trò chơi 2D đơn giản, nơi người chơi điều khiển xe tăng để chiến đấu với nhau trên địa hình đa dạng.

## Tính năng

- Đồ họa 2D sử dụng SDL2
- Hiệu ứng âm thanh và nhạc nền
- Hệ thống vật lý đơn giản
- Chế độ 2 người chơi

## Yêu cầu hệ thống

- macOS 10.15 trở lên
- Các thư viện SDL2, SDL2_image, SDL2_mixer, SDL2_ttf

## Cài đặt

### Cài đặt các thư viện cần thiết

```bash
# Cài đặt Homebrew (nếu chưa có)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Cài đặt các thư viện SDL2
brew install sdl2 sdl2_image sdl2_mixer sdl2_ttf