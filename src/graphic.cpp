#include "game.h"

// Tô màu hình tròn 
void filledCircleRGBA(SDL_Renderer* renderer, int x0, int y0, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    // set up cọ vẽ
    SDL_BlendMode oldBlendMode;
    SDL_GetRenderDrawBlendMode(renderer, &oldBlendMode);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    // the midpoint circle algorithm
    int x = 0, y = radius, d = 3 - 2 * radius;
    auto drawLines = [&](int x, int y) {
        SDL_RenderDrawLine(renderer, x0 - x, y0 - y, x0 + x, y0 - y);
        SDL_RenderDrawLine(renderer, x0 - x, y0 + y, x0 + x, y0 + y);
        SDL_RenderDrawLine(renderer, x0 - y, y0 - x, x0 + y, y0 - x);
        SDL_RenderDrawLine(renderer, x0 - y, y0 + x, x0 + y, y0 + x);
    };
    while (y >= x) {
        drawLines(x, y);
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else d = d + 4 * x + 6;
        x++;
    }
    SDL_SetRenderDrawBlendMode(renderer, oldBlendMode); // hoàn trả cọ vẽ như ban đầu
}
// Vẽ hình chữ nhật bo góc
void roundedRectangleRGBA(SDL_Renderer* renderer, int x, int y, int w, int h, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    // set up cọ vẽ
    SDL_BlendMode oldBlendMode;
    SDL_GetRenderDrawBlendMode(renderer, &oldBlendMode);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    // vẽ từng dòng ngang -> tạo hình chữ nhật
    for (int i = 0; i < h; i++) { // chiều cao
        int currentY = y + i; // toạ độ của dòng đang vẽ
        int startX = x;
        int endX = x + w - 1; // điểm đầu và cuối của dòng cần vẽ
        // nếu dòng nằm trong đoạn bo góc (trên hoặc dưới), dùng công thức tính độ dài (dx) để vẽ
        if (i < radius) {
            int dy = radius - i - 1;
            int dx = (int)(sqrt(radius * radius - dy * dy));
            startX = x + radius - dx;
            endX = x + w - radius + dx - 1;
        } 
        else if (i >= h - radius) {
            int dy = i - (h - radius);
            int dx = (int)(sqrt(radius * radius - dy * dy));
            startX = x + radius - dx;
            endX = x + w - radius + dx - 1;
        }
        // tạo hiệu ứng chuyển màu -> tăng chiều sâu
        float lightFactor = 1.0f;
        if (i < h / 2) lightFactor = 1.0f + 0.3f * (1.0f - (float)i / (h / 2));
        else lightFactor = 1.0f - 0.2f * ((float)(i - h / 2) / (h / 2));
        Uint8 adjustedR = (Uint8)fmin(255, r * lightFactor);
        Uint8 adjustedG = (Uint8)fmin(255, g * lightFactor);
        Uint8 adjustedB = (Uint8)fmin(255, b * lightFactor);
        SDL_SetRenderDrawColor(renderer, adjustedR, adjustedG, adjustedB, a);
        SDL_RenderDrawLine(renderer, startX, currentY, endX, currentY);
    }
    if (a > 100) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 80); // màu sáng - tạo hiệu ứng ánh sáng rọi vào
        for (int i = 0; i < 2; i++) { // vẽ 2 đường sáng phía trên
            int currentY = y + i;
            if (currentY < y + radius) {
                int dy = radius - i - 1;
                int dx = (int)(sqrt(radius * radius - dy * dy));
                int startX = x + radius - dx;
                int endX = x + w - radius + dx - 1;
                SDL_RenderDrawLine(renderer, startX, currentY, endX, currentY);
            } else SDL_RenderDrawLine(renderer, x, currentY, x + w - 1, currentY);
        }
        for (int i = 0; i < h / 3; i++) { // tô sáng vào các pixel hai bên mép
            int currentY = y + i;
            int leftX = x;
            int rightX = x + w - 1;
            if (i < radius) {
                int dy = radius - i - 1;
                int dx = (int)(sqrt(radius * radius - dy * dy));
                leftX = x + radius - dx;
                rightX = x + w - radius + dx - 1;
            }
            Uint8 alpha = 80 - (Uint8)(80 * i / (h / 3));
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
            SDL_RenderDrawPoint(renderer, leftX, currentY);
            SDL_RenderDrawPoint(renderer, rightX, currentY);
        }
    }
    SDL_SetRenderDrawBlendMode(renderer, oldBlendMode);
}
// khởi tạo mây
void Game::initializeClouds() {
    // tạo một số đám mây với chỉ số ngẫu nhiên
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> xDist(0, SCREEN_WIDTH);
    std::uniform_real_distribution<float> yDist(50, 200);
    std::uniform_real_distribution<float> sizeDist(100, 250);
    std::uniform_real_distribution<float> alphaDist(100, 200);

    clouds.clear();                 // reset mây
    int numClouds = 5 + gen() % 4;  // số lượng đám mây
    for (int i = 0; i < numClouds; i++) {
        float x = xDist(gen);
        float y = yDist(gen);
        float width = sizeDist(gen);
        float height = width * 0.6f;
        float alpha = alphaDist(gen);
        clouds.emplace_back(x, y, width, height, alpha); //vector lưu trữ thông tin các đám mây
    }
}
// update toạ độ mây (chuyển động)
void Game::updateClouds(float deltaTime) {
    if (!wind) return; // không có gió -> không có mây
    float windStrength = wind->getStrength();       // tốc độ gió ảnh hưởng đến chuyển động của mây
    for (auto& cloud : clouds) {
        cloud.speed = windStrength * 20.0f;         // tốc độ mây
        cloud.position.x += cloud.speed * deltaTime;// li độ x thay đổi theo thời gian
        // Nếu mây đi ra khỏi màn hình, đưa nó trở lại từ phía bên kia
        if (cloud.position.x > SCREEN_WIDTH + cloud.width) cloud.position.x = -cloud.width;
        else if (cloud.position.x < -cloud.width) cloud.position.x = SCREEN_WIDTH + cloud.width;
    }
}
// vẽ mây - tí chatgpt
void Game::renderClouds() { 
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // cài đặt cọ vẽ
    for (const auto& cloud : clouds) {
        // Vẽ đám mây bằng các hình elip chồng lên nhau
        int centerX = static_cast<int>(cloud.position.x + cloud.width / 2); 
        int centerY = static_cast<int>(cloud.position.y + cloud.height / 2);    // tâm đám mây
        
        // Vẽ phần thân chính của mây
        for (int i = 0; i < 3; i++) {   // vẽ 3 hình elip
            float offsetX = cloud.width * 0.2f * i;
            float offsetY = cloud.height * 0.1f * i;
            
            int cloudWidth = static_cast<int>(cloud.width * (1.0f - 0.2f * i));
            int cloudHeight = static_cast<int>(cloud.height * (1.0f - 0.1f * i));
            
            // Vẽ một hình elip mờ
            for (int y = -cloudHeight / 2; y <= cloudHeight / 2; y++) {
                for (int x = -cloudWidth / 2; x <= cloudWidth / 2; x++) {
                    float distance = (x * x) / float(cloudWidth * cloudWidth / 4) + 
                                    (y * y) / float(cloudHeight * cloudHeight / 4);

                    if (distance <= 1.0f) {
                        float alpha = (1.0f - distance) * (cloud.alpha / 255.0f);   // càng gần rìa alpha càng thấp
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 
                                              static_cast<Uint8>(alpha * 255));
                        
                        SDL_RenderDrawPoint(renderer, 
                                           centerX + x + static_cast<int>(offsetX), 
                                           centerY + y + static_cast<int>(offsetY));
                    }
                }
            }
        }
    }
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, int x, int y, const std::string& text, SDL_Color color) {
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), color);   // vẽ chuỗi ký tự lên một SDL_Surface
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface); // tạo texture từ surface vừa tạo
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;                                                // kích thước
    SDL_FreeSurface(textSurface);                                                   // giải phòng Surface
    SDL_Rect renderRect = {x, y, textWidth, textHeight};                            // tạo vùng vẽ
    SDL_RenderCopy(renderer, textTexture, NULL, &renderRect);                       // vẽ texture vào vùng rect
    SDL_DestroyTexture(textTexture);                                                // giải phóng texture
}

//vẽ địa hình
void Terrain::render() {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 25, 25, 112, 200);                     // màu địa hình 
    for (int x = 0; x < heightMap.size(); x++) {
        SDL_RenderDrawLine(renderer, x, heightMap[x], x, SCREEN_HEIGHT);    // vẽ từng cột theo toạ độ x
        for (int y = heightMap[x]; y < SCREEN_HEIGHT; y += 5) {
            if ((x + y) % 7 == 0) {
                SDL_SetRenderDrawColor(renderer, 65, 105, 225, 130);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
    // vẽ viền vàng
    for (int x = 0; x < heightMap.size(); x++) {
        for (int y = -1; y < 7; y++) {
            float alpha = 1.0f - (float)y / 5.0f;
            SDL_SetRenderDrawColor(renderer, 218,  165,  32, static_cast<Uint8>(255 * alpha)); // màu viền
            SDL_RenderDrawPoint(renderer, x, heightMap[x] + y);
        }
    }
}

// vẽ đạn
void Projectile::render(SDL_Renderer* renderer) {
    if (!active) return;
    filledCircleRGBA(renderer, static_cast<int>(position.x), static_cast<int>(position.y), 5, 255, 0, 0, 255);  // vẽ đạn hình tròn
}

void Tank::render() {
    int tankWidth = 40;
    int tankHeight = 20;                        // thân pháo
    int trackHeight = 6;
    int turretSize = 16;                        // tháp pháo
    int cannonLength = 30;
    int cannonWidth = 5;                        // pháo
    int tankX = static_cast<int>(position.x);
    int tankY = static_cast<int>(position.y);   // toạ độ 

    // Màu lấy màu từ TurnInfo
    SDL_Color primaryColor = facingRight ? 
        SDL_Color{0, 255, 255, 255} :  // Cyan cho Player 1 (giống màu chữ trong TurnInfo)
        SDL_Color{255, 100, 100, 255}; // Đỏ cho Player 2 (giống màu chữ trong TurnInfo)
    SDL_Color secondaryColor = facingRight ?
        SDL_Color{0, 200, 200, 255} :  // Cyan đậm hơn cho Player 1
        SDL_Color{200, 50, 50, 255};   // Đỏ đậm hơn cho Player 2

    SDL_Color trackColor = {80, 80, 80, 255};  // Xám cho bánh xích
    
    // Lưu blend mode hiện tại
    SDL_BlendMode oldBlendMode;
    SDL_GetRenderDrawBlendMode(renderer, &oldBlendMode);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Tính góc nòng pháo dựa trên hướng và góc bắn
    float actualAngle = facingRight ? firingAngle : 180.0f - firingAngle;
    actualAngle -= angle;
    float angleRad = actualAngle * PI / 180.0f;

    // Vẽ bánh xích với hiệu ứng kim loại
    for (int i = -tankWidth/2; i <= tankWidth/2; i += 3) {
        int x1 = tankX + i;
        int y1 = tankY + tankHeight/2 - trackHeight;
        int y2 = tankY + tankHeight/2;
        // Hiệu ứng kim loại cho bánh xích
        SDL_Color trackHighlight = {120, 120, 120, 255};
        if (i % 6 == 0) SDL_SetRenderDrawColor(renderer, trackHighlight.r, trackHighlight.g, trackHighlight.b, trackHighlight.a);
        else SDL_SetRenderDrawColor(renderer, trackColor.r, trackColor.g, trackColor.b, trackColor.a);
        SDL_RenderDrawLine(renderer, x1, y1, x1, y2);
    }

    // Vẽ thân xe với hiệu ứng kim loại và đường viền mạnh mẽ
    for (int i = 0; i < tankHeight - trackHeight; i++) {
        float factor = static_cast<float>(i) / (tankHeight - trackHeight);
        // Tạo hiệu ứng kim loại với gradient
        SDL_Color currentColor = {
            static_cast<Uint8>(primaryColor.r * (1 - factor) + secondaryColor.r * factor),
            static_cast<Uint8>(primaryColor.g * (1 - factor) + secondaryColor.g * factor),
            static_cast<Uint8>(primaryColor.b * (1 - factor) + secondaryColor.b * factor),
            255
        };
        // Thêm hiệu ứng ánh sáng
        if (i < 3) {
            // Phần trên sáng hơn
            currentColor.r = std::min(255, currentColor.r + 50);
            currentColor.g = std::min(255, currentColor.g + 50);
            currentColor.b = std::min(255, currentColor.b + 50);
        }
        SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
        SDL_RenderDrawLine(
            renderer,
            tankX - tankWidth/2, tankY - tankHeight/2 + i,
            tankX + tankWidth/2, tankY - tankHeight/2 + i
        );
    }
    // Vẽ đường viền thân xe 
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawLine(
        renderer,
        tankX - tankWidth/2, tankY - tankHeight/2,
        tankX + tankWidth/2, tankY - tankHeight/2
    );
    SDL_RenderDrawLine(
        renderer,
        tankX - tankWidth/2, tankY - tankHeight/2,
        tankX - tankWidth/2, tankY + tankHeight/2 - trackHeight
    );
    SDL_RenderDrawLine(
        renderer,
        tankX + tankWidth/2, tankY - tankHeight/2,
        tankX + tankWidth/2, tankY + tankHeight/2 - trackHeight
    );
    // Vẽ tháp pháo với hiệu ứng kim loại
    int turretX = tankX;
    int turretY = tankY - tankHeight/2;
    // Vẽ tháp pháo với hiệu ứng gradient
    for (int i = -turretSize; i <= turretSize; i++) {
        for (int j = -turretSize; j <= turretSize; j++) {
            float dist = sqrt(i*i + j*j);
            if (dist <= turretSize) {
                float factor = dist / turretSize;
                SDL_Color turretColor = {
                    static_cast<Uint8>(primaryColor.r * (1 - factor) + secondaryColor.r * factor),
                    static_cast<Uint8>(primaryColor.g * (1 - factor) + secondaryColor.g * factor),
                    static_cast<Uint8>(primaryColor.b * (1 - factor) + secondaryColor.b * factor),
                    255
                };
                // Thêm hiệu ứng ánh sáng từ trên xuống
                if (j < -turretSize/2) {
                    turretColor.r = std::min(255, turretColor.r + 30);
                    turretColor.g = std::min(255, turretColor.g + 30);
                    turretColor.b = std::min(255, turretColor.b + 30);
                }
                SDL_SetRenderDrawColor(renderer, turretColor.r, turretColor.g, turretColor.b, turretColor.a);
                SDL_RenderDrawPoint(renderer, turretX + i, turretY + j);
            }
        }
    }
    // Tính toán điểm đầu và cuối của nòng pháo
    int cannonStartX = turretX;
    int cannonStartY = turretY;
    int cannonEndX = cannonStartX + static_cast<int>(cannonLength * cos(angleRad));
    int cannonEndY = cannonStartY - static_cast<int>(cannonLength * sin(angleRad));
    // Vẽ nòng pháo với hiệu ứng kim loại
    for (int i = -cannonWidth/2; i <= cannonWidth/2; i++) {
        int offsetX = static_cast<int>(-i * sin(angleRad));
        int offsetY = static_cast<int>(-i * cos(angleRad));
        // Gradient màu cho nòng pháo
        float factor = std::abs(static_cast<float>(i) / (cannonWidth/2));
        SDL_Color cannonColor = {
            static_cast<Uint8>(primaryColor.r * (1 - factor) + secondaryColor.r * factor),
            static_cast<Uint8>(primaryColor.g * (1 - factor) + secondaryColor.g * factor),
            static_cast<Uint8>(primaryColor.b * (1 - factor) + secondaryColor.b * factor),
            255
        };
        SDL_SetRenderDrawColor(renderer, cannonColor.r, cannonColor.g, cannonColor.b, 255);
        SDL_RenderDrawLine(
            renderer,
            cannonStartX + offsetX, cannonStartY + offsetY,
            cannonEndX + offsetX, cannonEndY + offsetY
        );
    }
    // Vẽ điểm sáng ở đầu nòng
    filledCircleRGBA(renderer, cannonEndX, cannonEndY, cannonWidth/2 + 1, 255, 255, 255, 200);               
    // Thêm hiệu ứng kim loại ở đầu nòng
    filledCircleRGBA(renderer, cannonEndX, cannonEndY, cannonWidth/2, primaryColor.r, primaryColor.g, primaryColor.b, 230);
    // Vẽ đường dự đoán đạn khi đang ngắm
    if (isAiming) {
        SDL_SetRenderDrawColor(renderer, primaryColor.r, primaryColor.g, primaryColor.b, 150);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        // Vẽ đường dự đoán
        for (int i = 1; i < TRAJECTORY_POINTS; i++) {
            SDL_RenderDrawLine(
                renderer,
                static_cast<int>(trajectoryPoints[i-1].x),
                static_cast<int>(trajectoryPoints[i-1].y),
                static_cast<int>(trajectoryPoints[i].x),
                static_cast<int>(trajectoryPoints[i].y)
            );
        }
    }

    SDL_SetRenderDrawBlendMode(renderer, oldBlendMode); // khôi phục blend mode
    // vẽ thanh máu và mana
    int healthBarWidth = 400;  
    int healthBarHeight = 20;
    int manaBarWidth = 400;    
    int manaBarHeight = 10;    
    int barSpacing = 8; // khoảng cách giữa 2 thanh    
    int barY = 20;             
    int barX = (facingRight) ? 80 : SCREEN_WIDTH - healthBarWidth - 80;  // xác định vị trí theo hướng nhân vật

    // vẽ hcn lớn hơn -> tạo viền trắng
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // màu trắng
    SDL_Rect healthBarBorder = {barX - 2, barY - 2, healthBarWidth + 4, healthBarHeight + 4};
    SDL_RenderFillRect(renderer, &healthBarBorder);
    // vẽ nền thanh máu màu đen (hiển thị khi hết máu)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // nền đen
    SDL_Rect healthBarBg = {barX, barY, healthBarWidth, healthBarHeight};
    SDL_RenderFillRect(renderer, &healthBarBg);
    // Sử dụng màu chính của xe tăng cho thanh máu
    SDL_SetRenderDrawColor(renderer, primaryColor.r, primaryColor.g, primaryColor.b, 255);
    SDL_Rect healthBar = {barX, barY, static_cast<int>(healthBarWidth * health / 100.0f), healthBarHeight};
    SDL_RenderFillRect(renderer, &healthBar);
    // vẽ thanh mana
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect manaBarBorder = {barX - 1, barY + healthBarHeight + barSpacing - 1, manaBarWidth + 2, manaBarHeight + 2};
    SDL_RenderFillRect(renderer, &manaBarBorder);
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);  
    SDL_Rect manaBar = {
        barX,
        barY + healthBarHeight + barSpacing,
        static_cast<int>(manaBarWidth * stamina / 100.0f),
        manaBarHeight
    };
    SDL_RenderFillRect(renderer, &manaBar);

    SDL_SetRenderDrawBlendMode(renderer, oldBlendMode);
    if (isAiming) renderTrajectory(); // vẽ thanh dự đoán đường đạn
}

void Tank::renderTrajectory() { //dự đoán đường đạn
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (int i = 1; i < TRAJECTORY_POINTS; i++) {
        SDL_RenderDrawLine(renderer, 
                          static_cast<int>(trajectoryPoints[i-1].x), 
                          static_cast<int>(trajectoryPoints[i-1].y), 
                          static_cast<int>(trajectoryPoints[i].x), 
                          static_cast<int>(trajectoryPoints[i].y));
    }
}

// Game render
void Game::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    if (backgroundTexture) {
        SDL_Rect backgroundRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(renderer, backgroundTexture, NULL, &backgroundRect);
    }
    if (state == GameState::MENU) renderMenu();
    else if (state == GameState::PLAYING || state == GameState::GAME_OVER) {
        terrain->render(); 
        renderClouds();
        for (auto& tank : tanks) tank->render(); 
        for (auto& projectile : projectiles) projectile.render(renderer); 
        renderExplosions(); 
        // render vòng thời gian
        int centerX = SCREEN_WIDTH / 2;
        int centerY = 60;
        int radius = 50;
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 20, 230);
        filledCircleRGBA(renderer, centerX, centerY, radius, 0, 0, 20, 230);
        // vòng thời gian chuyển động
        float timeProgress = turnTimer / TURN_TIME_LIMIT;
        int segments = 60;
        float angleStep = 2 * PI / segments;
        float startAngle = -PI / 2; 
        for(int i = 0; i < segments; i++) {
            float angle = startAngle + i * angleStep;
            float nextAngle = angle + angleStep;
            float progress = (float)i / segments;
            if(progress <= timeProgress) {
                SDL_SetRenderDrawColor(renderer, 144, 238, 144, 255);
                for(int r = radius - 3; r <= radius; r++) {
                    int x1 = centerX + r * cos(angle);
                    int y1 = centerY + r * sin(angle);
                    int x2 = centerX + r * cos(nextAngle);
                    int y2 = centerY + r * sin(nextAngle);
                    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
                }
            }
        }
        // render player turn info trong vòng tròn
        if (fontMedium) {
            std::string playerText = "Player " + std::to_string(currentPlayerIndex + 1);
            SDL_Color playerColor = currentPlayerIndex == 0 ? 
                SDL_Color{0, 255, 255, 255} : SDL_Color{255, 100, 100, 255};
            float time = SDL_GetTicks() / 1000.0f;
            int glowIntensity = static_cast<int>(128 + 127 * sin(time * 3.0f));
            SDL_Color glowColor = {
                static_cast<Uint8>(playerColor.r * 0.5f),
                static_cast<Uint8>(playerColor.g * 0.5f),
                static_cast<Uint8>(playerColor.b * 0.5f),
                static_cast<Uint8>(glowIntensity)
            };
            renderText(renderer, fontMedium, 
                      centerX - 45, centerY - 15,
                      playerText, glowColor);
            renderText(renderer, fontMedium, 
                      centerX - 45, centerY - 15,
                      playerText, playerColor);
        }
        
        renderTurnInfo();   // Render khung turn info
        if (state == GameState::GAME_OVER) renderGameOver();
    }
    if (state == GameState::PAUSED) renderPauseMenu();
    SDL_RenderPresent(renderer);
}

void Game::renderExplosions() {
    for (const auto& explosion : explosions) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        for (float r = 0; r <= explosion.radius; r += 0.5f) {
            float factor = r / explosion.radius;
            Uint8 red = 255;
            Uint8 green = static_cast<Uint8>(255 * (1 - factor * 0.7f));
            Uint8 blue = static_cast<Uint8>(100 * (1 - factor));
            Uint8 alpha = static_cast<Uint8>(255 * (1 - factor * 0.8f));
            filledCircleRGBA(renderer, 
                            static_cast<int>(explosion.position.x), 
                            static_cast<int>(explosion.position.y), 
                            r, red, green, blue, alpha);
        }
        for (const auto& particle : explosion.particles) {
            float alpha = 255 * (particle.lifetime / particle.maxLifetime);
            filledCircleRGBA(renderer, 
                            static_cast<int>(particle.position.x), 
                            static_cast<int>(particle.position.y), 
                            2 + 3 * (particle.lifetime / particle.maxLifetime), 
                            particle.color.r, particle.color.g, particle.color.b, 
                            static_cast<Uint8>(alpha));
        }
    }
}

void Game::renderMenu() {
    if (menuBackgroundTexture) {
        SDL_Rect bgRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(renderer, menuBackgroundTexture, NULL, &bgRect);
    }
    if (showInstructions) {
        renderInstructions();
        return;
    }
    const char* menuItems[static_cast<int>(MenuItem::MENU_ITEM_COUNT)] = {
        "New Game",
        "Instructions",
        "Exit"
    };
    int menuWidth = 300;
    int menuY = SCREEN_HEIGHT / 2;
    int menuItemHeight = 80;
    int menuSpacing = 20;
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    for (int i = 0; i < static_cast<int>(MenuItem::MENU_ITEM_COUNT); i++) {
        int itemWidth = menuWidth;
        int itemHeight = menuItemHeight;
        int itemX = SCREEN_WIDTH / 2 - itemWidth / 2;
        int itemY = menuY + i * (menuItemHeight + menuSpacing);
        bool isHovered = (mouseX >= itemX && mouseX <= itemX + itemWidth &&
                        mouseY >= itemY && mouseY <= itemY + itemHeight);
        bool isSelected = (i == selectedMenuItem || isHovered);
        float scale = isSelected ? 1.1f : 1.0f;
        int scaledWidth = static_cast<int>(itemWidth * scale);
        int scaledHeight = static_cast<int>(itemHeight * scale);
        int scaledX = SCREEN_WIDTH / 2 - scaledWidth / 2;
        int scaledY = itemY + (itemHeight - scaledHeight) / 2;
        SDL_Color buttonColor;
        if (isSelected) {
            buttonColor = {100, 180, 240, 255};
        } else {
            buttonColor = {180, 180, 180, 255};
        }
        if (isSelected) {
            SDL_Color glowColor = {buttonColor.r, buttonColor.g, buttonColor.b, 150};
            int glowSize = 15;
            roundedRectangleRGBA(renderer, 
                                scaledX - glowSize, scaledY - glowSize, 
                                scaledWidth + glowSize * 2, scaledHeight + glowSize * 2, 
                                25 + glowSize,
                                glowColor.r, glowColor.g, glowColor.b, glowColor.a);
        }
        roundedRectangleRGBA(renderer, scaledX, scaledY, scaledWidth, scaledHeight, 25,
                            buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
        if (fontMedium) {
            SDL_Color textColor = {255, 255, 255, 255};
            renderText(renderer, fontMedium,
                    scaledX + scaledWidth/2 - 50,  
                    scaledY + scaledHeight/2 - 15, 
                    menuItems[i], textColor);
        }
    }
    if (fontSmall) {
        SDL_Color hintColor = {255, 255, 255, 200};
        renderText(renderer, fontSmall,
                SCREEN_WIDTH / 2 - 200,
                SCREEN_HEIGHT - 50,
                "Use UP/DOWN arrows or mouse to navigate, ENTER or click to select",
                hintColor);
    }
}

// Improved instructions screen
void Game::renderInstructions() {
    // Create a semi-transparent background panel
    int panelWidth = 700;
    int panelHeight = 500;
    int panelX = SCREEN_WIDTH / 2 - panelWidth / 2;
    int panelY = SCREEN_HEIGHT / 2 - panelHeight / 2;
    // Draw panel background with gradient
    for (int y = 0; y < panelHeight; y++) {
        float alpha = 0.8f - 0.2f * (float)y / panelHeight;
        SDL_SetRenderDrawColor(renderer, 20, 30, 50, static_cast<Uint8>(alpha * 255));
        SDL_RenderDrawLine(renderer, panelX, panelY + y, panelX + panelWidth, panelY + y);
    }
    // Draw panel border
    roundedRectangleRGBA(renderer, panelX, panelY, panelWidth, panelHeight, 20, 100, 150, 200, 200);
    // Draw title
    if (fontLarge) {
        SDL_Color titleColor = {255, 215, 0, 255}; // Gold color
        renderText(renderer, fontLarge, panelX + panelWidth/2 - 100, panelY + 30, "HOW TO PLAY", titleColor);
        // Underline
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 150);
        SDL_RenderDrawLine(renderer, panelX + 150, panelY + 75, panelX + panelWidth - 150, panelY + 75);
    }
    
    // Instructions content
    std::vector<std::pair<std::string, std::string>> instructions = {
        {"Movement", "Use LEFT/RIGHT arrow keys or A/D to move your tank"},
        {"Aiming", "Click and hold with the mouse to aim, release to fire"},
        {"Power", "The further you drag the mouse, the more power is applied"},
        {"Wind", "Watch the wind indicator - it affects projectile trajectory"},
        {"Terrain", "Explosions will destroy terrain, creating new tactical options"},
        {"Stamina", "Moving consumes stamina (yellow bar), restored each turn"},
        {"Time", "Each turn has a time limit shown by the circle at the top"},
        {"Victory", "Last tank standing wins the battle!"}
    };
    int startY = panelY + 100;
    int lineHeight = 40;
    
    for (const auto& instruction : instructions) {
        if (fontMedium && fontSmall) {
            // Category
            SDL_Color categoryColor = {100, 200, 255, 255}; // Light blue
            renderText(renderer, fontMedium, panelX + 50, startY, instruction.first + ":", categoryColor);
            
            // Description
            SDL_Color descColor = {255, 255, 255, 255}; // White
            renderText(renderer, fontSmall, panelX + 250, startY, instruction.second, descColor);
        }
        startY += lineHeight;
    }
    // Back button
    int buttonWidth = 200;
    int buttonHeight = 50;
    int buttonX = panelX + panelWidth/2 - buttonWidth/2;
    int buttonY = panelY + panelHeight - 80;
    
    roundedRectangleRGBA(renderer, buttonX, buttonY, buttonWidth, buttonHeight, 15, 80, 120, 180, 220);
    
    if (fontMedium) {
        SDL_Color buttonTextColor = {255, 255, 255, 255};
        renderText(renderer, fontMedium, buttonX + 30, buttonY + 10, "Back to Menu", buttonTextColor);
    }
    // Instruction at the bottom
    if (fontSmall) {
        SDL_Color instructionColor = {200, 200, 200, 200};
        renderText(renderer, fontSmall, panelX + panelWidth/2 - 150, panelY + panelHeight - 30, 
                  "Press ESC or ENTER to return to the menu", instructionColor);
    }
}

void Game::renderTurnInfo() {
    int infoWidth = 200;
    int infoHeight = 80;
    int infoX = 20;
    int infoY = 100;
    
    // Draw panel
    roundedRectangleRGBA(renderer, infoX, infoY, infoWidth, infoHeight, 15, 0, 0, 0, 180);
    
    // Player turn info
    std::string turnStr = "Player " + std::to_string(currentPlayerIndex + 1) + "'s turn";
    if (fontMedium) {
        SDL_Color playerColor = currentPlayerIndex == 0 ? 
            SDL_Color{0, 255, 255, 255} : SDL_Color{255, 100, 100, 255};
        renderText(renderer, fontMedium, infoX + 20, infoY + 15, turnStr, playerColor);
    }
    
    // Wind info
    if (fontSmall && wind) {
        float windStrength = wind->getStrength();
        std::string windStr = "Wind: ";
        SDL_Color windColor;
        if (windStrength == 0) {
            windStr += "---";
            windColor = {200, 200, 200, 255}; // Gray
        } else if (0 < windStrength && windStrength <= 12.0f) {
            windStr += ">";
            windColor = {100, 255, 100, 255}; // Green
        } else if (12.0f < windStrength && windStrength <= 20.0f) {
            windStr += ">>>";
            windColor = {255, 255, 0, 255}; // Yellow
        } else if (20.0f < windStrength) {
            windStr += ">>>>>";
            windColor = {255, 100, 100, 255}; // Red
        } else if (0 > windStrength && windStrength >= -12.0f) {
            windStr += "<";
            windColor = {100, 255, 100, 255}; // Green
        } else if (-12.0f > windStrength && windStrength >= -20.0f) {
            windStr += "<<<";
            windColor = {255, 255, 0, 255}; // Yellow
        } else if (-20.0f > windStrength) {
            windStr += "<<<<<";
            windColor = {255, 100, 100, 255}; // Red
        }
        renderText(renderer, fontSmall, infoX + 20, infoY + 50, windStr, windColor);
    }
}

// Game over
void Game::renderGameOver() {
    // Create a semi-transparent overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &overlay);
    
    // Create a victory panel
    int panelWidth = 600;
    int panelHeight = 400;
    int panelX = SCREEN_WIDTH / 2 - panelWidth / 2;
    int panelY = SCREEN_HEIGHT / 2 - panelHeight / 2;
    
    // Draw panel with gradient
    for (int y = 0; y < panelHeight; y++) {
        float progress = (float)y / panelHeight;
        Uint8 r = static_cast<Uint8>(20 + 30 * progress);
        Uint8 g = static_cast<Uint8>(40 + 20 * progress);
        Uint8 b = static_cast<Uint8>(80 - 20 * progress);
        SDL_SetRenderDrawColor(renderer, r, g, b, 220);
        SDL_RenderDrawLine(renderer, panelX, panelY + y, panelX + panelWidth, panelY + y);
    }
    
    // Draw panel border with glow
    for (int i = 0; i < 5; i++) {
        roundedRectangleRGBA(renderer, panelX - i, panelY - i, panelWidth + i*2, panelHeight + i*2, 
                            20 + i, 255, 215, 0, 50 - i*10);
    }
    roundedRectangleRGBA(renderer, panelX, panelY, panelWidth, panelHeight, 20, 255, 215, 0, 100);
    // Draw victory text
    if (fontLarge) {
        // Create pulsing effect for the title
        float time = SDL_GetTicks() / 1000.0f;
        int glowIntensity = static_cast<int>(200 + 55 * sin(time * 2.0f));
        
        SDL_Color victoryGlow = {255, 215, 0, static_cast<Uint8>(glowIntensity)};
        SDL_Color victoryColor = {255, 215, 0, 255}; // Gold
        
        std::string victoryText = "VICTORY!";
        
        // Draw glow effect
        for (int i = 1; i <= 3; i++) {
            renderText(renderer, fontLarge, 
                      panelX + panelWidth/2 - 100 - i, panelY + 60 - i, 
                      victoryText, victoryGlow);
            renderText(renderer, fontLarge, 
                      panelX + panelWidth/2 - 100 + i, panelY + 60 - i, 
                      victoryText, victoryGlow);
            renderText(renderer, fontLarge, 
                      panelX + panelWidth/2 - 100 - i, panelY + 60 + i, 
                      victoryText, victoryGlow);
            renderText(renderer, fontLarge, 
                      panelX + panelWidth/2 - 100 + i, panelY + 60 + i, 
                      victoryText, victoryGlow);
        }
        
        // Draw main text
        renderText(renderer, fontLarge, 
                  panelX + panelWidth/2 - 100, panelY + 60, 
                  victoryText, victoryColor);
    }
    
    // Draw winner text
    if (fontMedium && winnerIndex >= 0) {
        SDL_Color winnerColor = winnerIndex == 0 ? 
            SDL_Color{100, 255, 255, 255} : SDL_Color{255, 100, 100, 255};
        
        std::string winnerText = "Player " + std::to_string(winnerIndex + 1) + " is victorious!";
        renderText(renderer, fontMedium, 
                  panelX + panelWidth/2 - 150, panelY + 150, 
                  winnerText, winnerColor);
        
        std::string congratsText = "Congratulations on your tactical brilliance!";
        renderText(renderer, fontMedium, 
                  panelX + panelWidth/2 - 220, panelY + 200, 
                  congratsText, SDL_Color{255, 255, 255, 255});
    }
    
    // Draw buttons
    int buttonWidth = 200;
    int buttonHeight = 50;
    int buttonSpacing = 40;
    int buttonsY = panelY + panelHeight - 100;
    
    // New Game button
    int newGameX = panelX + panelWidth/2 - buttonWidth - buttonSpacing/2;
    roundedRectangleRGBA(renderer, newGameX, buttonsY, buttonWidth, buttonHeight, 15, 0, 120, 0, 220);
    
    if (fontMedium) {
        renderText(renderer, fontMedium, 
                  newGameX + 40, buttonsY + 10, 
                  "New Game", SDL_Color{255, 255, 255, 255});
    }
    
    // Main Menu button
    int menuX = panelX + panelWidth/2 + buttonSpacing/2;
    roundedRectangleRGBA(renderer, menuX, buttonsY, buttonWidth, buttonHeight, 15, 120, 0, 120, 220);
    
    if (fontMedium) {
        renderText(renderer, fontMedium, 
                  menuX + 40, buttonsY + 10, 
                  "Main Menu", SDL_Color{255, 255, 255, 255});
    }
    if (fontSmall) {
        renderText(renderer, fontSmall, 
                  panelX + panelWidth/2 - 150, panelY + panelHeight - 40, 
                  "Press R for new game or M for main menu", SDL_Color{200, 200, 200, 255});
    }
    
    // Draw decorative elements - stars or confetti
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> xDist(0, SCREEN_WIDTH);
    std::uniform_int_distribution<> yDist(0, SCREEN_HEIGHT);
    std::uniform_int_distribution<> sizeDist(1, 4);
    
    float time = SDL_GetTicks() / 1000.0f;
    
    for (int i = 0; i < 100; i++) {
        int x = (xDist(gen) + static_cast<int>(50 * sin(time + i * 0.1f))) % SCREEN_WIDTH;
        int y = (yDist(gen) + static_cast<int>(time * 50)) % SCREEN_HEIGHT;
        int size = sizeDist(gen);
        
        // Cycle through colors
        float hue = fmod(time * 0.2f + i * 0.01f, 1.0f);
        SDL_Color color;
        if (hue < 0.33f) color = {255, static_cast<Uint8>(255 * hue * 3), 0, 200};
        else if (hue < 0.66f) color = {static_cast<Uint8>(255 * (1 - (hue - 0.33f) * 3)), 255, 0, 200};
        else color = {0, static_cast<Uint8>(255 * (1 - (hue - 0.66f) * 3)), 255, 200};
        filledCircleRGBA(renderer, x, y, size, color.r, color.g, color.b, color.a);
    }
}

void Game::renderPauseMenu() {
    SDL_Rect pauseRect = {SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 150, 400, 400};
    roundedRectangleRGBA(renderer, pauseRect.x, pauseRect.y, pauseRect.w, pauseRect.h, 20, 0, 0, 0, 200);
    roundedRectangleRGBA(renderer, pauseRect.x, pauseRect.y, pauseRect.w, pauseRect.h, 20, 255, 255, 255, 100);
    std::string title = "PAUSED";
    if (fontLarge) {
        SDL_Color titleColor = {255, 255, 0, 255};
        renderText(renderer, fontLarge, SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 - 130, title, titleColor);
    }
    std::vector<std::string> menuItems = {
        "Resume",
        "Music Volume: " + std::to_string(musicVolume),
        "SFX Volume: " + std::to_string(sfxVolume),
        "Main Menu"
    };
    int menuY = SCREEN_HEIGHT / 2 - 30;
    for (size_t i = 0; i < menuItems.size(); i++) {
        SDL_Color textColor;
        if (i == selectedPauseMenuItem) textColor = {255, 255, 0, 255}; 

        else textColor = {255, 255, 255, 255}; 
        if (fontMedium) renderText(renderer, fontMedium, SCREEN_WIDTH / 2 - 150, menuY, menuItems[i], textColor);
        menuY += 40;
    }
    std::vector<std::string> controls = {
        "Use UP/DOWN to navigate",
        "Use LEFT/RIGHT to adjust volume",
        "Press ESC to resume",
        "Press ENTER to select"
    };
    menuY = SCREEN_HEIGHT / 2 + 150;
    for (const auto& control : controls) {
        if (fontSmall) {
            SDL_Color textColor = {200, 200, 200, 255};
            renderText(renderer, fontSmall, SCREEN_WIDTH / 2 - 150, menuY, control, textColor);
        }
        menuY += 20;
    }
}