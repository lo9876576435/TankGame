# Cải thiện CMakeLists.txt
cat > ~/MyGame/CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.10)
project(TankStarGame VERSION 1.0)

# Đặt chuẩn C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tìm các thư viện SDL2
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

# Nếu không tìm thấy các gói, thử tìm thủ công
if(NOT SDL2_FOUND)
    # Các đường dẫn phổ biến cho SDL2
    set(SDL2_SEARCH_PATHS
        /usr/local
        /usr
        /opt/homebrew
        /opt/local
        $ENV{SDL2DIR}
        ${SDL2_DIR}
    )
    
    # Tìm include
    find_path(SDL2_INCLUDE_DIR SDL.h
        HINTS
        PATH_SUFFIXES include/SDL2 include SDL2
        PATHS ${SDL2_SEARCH_PATHS}
    )
    
    # Tìm thư viện
    find_library(SDL2_LIBRARY
        NAMES SDL2
        HINTS
        PATH_SUFFIXES lib64 lib
        PATHS ${SDL2_SEARCH_PATHS}
    )
    
    if(SDL2_INCLUDE_DIR AND SDL2_LIBRARY)
        set(SDL2_FOUND TRUE)
        set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
        set(SDL2_LIBRARIES ${SDL2_LIBRARY})
    endif()
endif()

# Tương tự cho các thư viện khác...

# Thêm thư mục include
include_directories(${SDL2_INCLUDE_DIRS})

# Thêm các file nguồn
set(SOURCES
    src/main.cpp
    src/graphic.cpp
    src/logic.cpp
)

# Tạo executable
add_executable(game ${SOURCES})

# Liên kết các thư viện
target_link_libraries(game
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
)

# Sao chép tài nguyên vào thư mục build
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

# Tạo ứng dụng macOS (.app)
if(APPLE)
    set(APP_NAME "MyGame")
    set(APP_BUNDLE_DIR "${CMAKE_BINARY_DIR}/${APP_NAME}.app")
    
    # Tạo cấu trúc thư mục .app
    file(MAKE_DIRECTORY "${APP_BUNDLE_DIR}/Contents/MacOS")
    file(MAKE_DIRECTORY "${APP_BUNDLE_DIR}/Contents/Resources/assets")
    
    # Sao chép executable
    add_custom_command(TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:game> "${APP_BUNDLE_DIR}/Contents/MacOS/"
    )
    
    # Sao chép tài nguyên
    add_custom_command(TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" "${APP_BUNDLE_DIR}/Contents/Resources/assets"
    )
    
    # Tạo Info.plist
    configure_file(
        "${CMAKE_SOURCE_DIR}/Info.plist.in"
        "${APP_BUNDLE_DIR}/Contents/Info.plist"
    )
endif()
EOF

# Tạo file Info.plist.in cho macOS
cat > ~/MyGame/Info.plist.in << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>game</string>
    <key>CFBundleIdentifier</key>
    <string>com.yourdomain.mygame</string>
    <key>CFBundleName</key>
    <string>MyGame</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>NSHighResolutionCapable</key>
    <true/>
</dict>
</plist>
EOF