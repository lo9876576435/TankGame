#include "game.h"

// các phép tính với vector 2 chiều
Vector2 Vector2::operator+(const Vector2& v) const {
    return Vector2(x + v.x, y + v.y);
}
Vector2 Vector2::operator-(const Vector2& v) const {
    return Vector2(x - v.x, y - v.y);
}
Vector2 Vector2::operator*(float scalar) const {
    return Vector2(x * scalar, y * scalar);
}
float Vector2::length() const {
    return std::sqrt(x * x + y * y);
}
Vector2 Vector2::normalize() const {
    float len = length();
    if (len > 0) return Vector2(x / len, y / len);
    return *this;
}

// vụ nổ nhỏ
Particle::Particle(float x, float y, float vx, float vy, float lifetime, SDL_Color color) 
    : position{x, y}, velocity{vx, vy}, lifetime(lifetime), maxLifetime(lifetime), color(color) {}
// thay đổi toạ độ theo thời gian
bool Particle::update(float deltaTime) {
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
    velocity.y += GRAVITY * deltaTime;
    lifetime -= deltaTime;
    return lifetime > 0;
}

// vụ nổ chính - chatgpt
Explosion::Explosion(float x, float y, float maxRadius) 
    : position{x, y}, radius(5.0f), maxRadius(maxRadius), growing(true), speed(1.5f) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> velDist(-5.0f, 5.0f);
    std::uniform_real_distribution<float> lifeDist(0.5f, 2.0f);
    int numParticles = 30 + gen() % 20;
    for (int i = 0; i < numParticles; i++) {
        float vx = velDist(gen);
        float vy = velDist(gen);
        float lifetime = lifeDist(gen);
        Uint8 r = 255;
        Uint8 g = 100 + gen() % 156;
        Uint8 b = gen() % 50;
        particles.emplace_back(x, y, vx, vy, lifetime, SDL_Color{r, g, b, 255});
    }
}
// vụ nổ - update theo thời gian
bool Explosion::update(float deltaTime) {
    if (growing) {  // mở rộng vụ nổ
        radius += speed * deltaTime * 60.0f;
        if (radius >= maxRadius) growing = false;
    }
    else radius -= speed * deltaTime * 60.0f;   // nhỏ dần
    //hiển thị vụ nổ nhỏ
    for (auto it = particles.begin(); it != particles.end();) {
        if (!it->update(deltaTime)) it = particles.erase(it);
        else ++it;
    }
    return radius > 0 || !particles.empty();
}

//  cơ chế địa hình
Terrain::Terrain(SDL_Renderer* renderer, int width) 
    : renderer(renderer) {
    heightMap.resize(width);
    generateTerrain();
}

void Terrain::generateTerrain() {
    for (int i = 0; i < heightMap.size(); i++) heightMap[i] = SCREEN_HEIGHT - 150;
    std::random_device rd;
    std::mt19937 gen(rd());
    int numFeatures = 4;
    float segmentWidth = heightMap.size() / (float)numFeatures;
    for (int i = 0; i < numFeatures; i++) {
        int featurePos = static_cast<int>(i * segmentWidth + segmentWidth/4 + gen() % static_cast<int>(segmentWidth/2));
        int featureWidth = static_cast<int>(segmentWidth * 0.6f);
        int height = 50 + gen() % 100;
        bool isHill = (i % 2 == 0);
        int flatWidth = featureWidth / 3;
        int startFlat = featurePos + featureWidth / 3;
        for (int j = featurePos; j < startFlat; j++) {
            if (j >= 0 && j < heightMap.size()) {
                float progress = float(j - featurePos) / float(startFlat - featurePos);
                float smoothProgress = (1 - cos(progress * PI)) / 2;
                if (isHill) heightMap[j] = SCREEN_HEIGHT - 150 - height * smoothProgress;
                else heightMap[j] = SCREEN_HEIGHT - 150 + height * smoothProgress;
            }
        }
        for (int j = startFlat; j < startFlat + flatWidth; j++) {
            if (j >= 0 && j < heightMap.size()) {
                if (isHill) heightMap[j] = SCREEN_HEIGHT - 150 - height;
                else heightMap[j] = SCREEN_HEIGHT - 150 + height;
            }
        }
        for (int j = startFlat + flatWidth; j < featurePos + featureWidth; j++) {
            if (j >= 0 && j < heightMap.size()) {
                float progress = float(j - (startFlat + flatWidth)) / float(featurePos + featureWidth - (startFlat + flatWidth));
                float smoothProgress = (1 - cos((1 - progress) * PI)) / 2; 
                if (isHill) heightMap[j] = SCREEN_HEIGHT - 150 - height * smoothProgress;
                else heightMap[j] = SCREEN_HEIGHT - 150 + height * smoothProgress;
            }
        }
    }
    smoothTerrain(0, heightMap.size() - 1);
}

int Terrain::getHeightAt(int x) const {
    if (x >= 0 && x < heightMap.size()) return heightMap[x];
    return SCREEN_HEIGHT;
}

void Terrain::destroyAt(int x, int radius) {
    for (int i = x - radius; i <= x + radius; i++) {
        if (i >= 0 && i < heightMap.size()) {
            float distance = std::abs(i - x);
            if (distance <= radius) {
                float depthFactor = std::sqrt(1.0f - (distance * distance) / (radius * radius));
                int depth = static_cast<int>(radius * depthFactor * 0.8f);
                heightMap[i] += depth;
            }
        }
    }
    smoothTerrain(x - radius - 5, x + radius + 5);  
}
// làm mịn đất = trung bình cộng của độ cao xung quanh
void Terrain::smoothTerrain(int startX, int endX) {
    startX = std::max(0, startX);
    endX = std::min(static_cast<int>(heightMap.size() - 1), endX);
    std::vector<int> smoothed = heightMap; 
    for (int i = startX + 1; i < endX - 1; i++) {
        for(int j = 1; j <= 3; j++) smoothed[i] += heightMap[i - j] + heightMap[i + j];
        smoothed[i] /= 7;
    }
    for (int i = startX + 1; i < endX - 1; i++) heightMap[i] = smoothed[i];
}

// tạo gió ngẫu nhiên
Wind::Wind() : strength(0) {
    randomize();
}
void Wind::randomize() {
    int rate = (rand() % 100 + 1);
    if(rate <= 20) strength = level[3];
    else if(rate <= 40) strength = level[2];
    else if(rate <= 70) strength = level[1];
    else strength = level[0];
    if(rate % 2) strength *= -1;
}
float Wind::getStrength() const { 
    return strength; 
}
// cơ chế đạn
Projectile::Projectile() : active(false) {}
void Projectile::fire(const Vector2& startPos, const Vector2& initialVelocity) {    // khai báo đạn
    position = startPos;
    velocity = initialVelocity;
    active = true;
}
// update quỹ đạo đạn
void Projectile::update(float deltaTime, float windStrength, const Terrain& terrain) {
    if (!active) return;
    velocity.y += GRAVITY * deltaTime;
    velocity.x += windStrength * deltaTime;
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
    int terrainHeight = terrain.getHeightAt(static_cast<int>(position.x));
    if (position.y >= terrainHeight || position.x < 0 || position.x >= SCREEN_WIDTH || position.y >= SCREEN_HEIGHT) active = false;
}

bool Projectile::isActive() const {         // trạng thái 
    return active;
}
Vector2 Projectile::getPosition() const {   // get toạ độ
    return position;
}
void Projectile::deactivate() {             // xoá đạn
    active = false;
}

Tank::Tank(SDL_Renderer* renderer, const Vector2& startPos, const SDL_Color& tankColor, bool facingRight) 
    : renderer(renderer), position(startPos), angle(0), firingAngle(45), power(50), 
    health(100), stamina(100), bullet(1), isAiming(false), color(tankColor),
    facingRight(facingRight), width(60), height(30), terrain(nullptr) {
}

Tank::~Tank() {}

void Tank::update(const Terrain& terrain) {
    int terrainHeight = terrain.getHeightAt(static_cast<int>(position.x));
    position.y = terrainHeight - height/2 + 20; 
    int leftHeight = terrain.getHeightAt(static_cast<int>(position.x) - width/2);
    int rightHeight = terrain.getHeightAt(static_cast<int>(position.x) + width/2);
    angle = std::atan2(rightHeight - leftHeight, width) * 180 / PI;
    if (isAiming) calculateTrajectory(terrain);
}

void Tank::calculateTrajectory(const Terrain& terrain) {
    float actualAngle = facingRight ? firingAngle : 180.0f - firingAngle;
    actualAngle -= angle;
    float angleRad = actualAngle * PI / 180.0f;
    
    // Tính toán vị trí đầu nòng
    float cannonLength = 25; // Độ dài nòng pháo
    Vector2 startPos = position;
    startPos.y -= height/2; // Điểm giữa xe tăng
    // Tính toán điểm đầu nòng
    startPos.x += cannonLength * cos(angleRad);
    startPos.y -= cannonLength * sin(angleRad);
    
    // Tính toán quỹ đạo từ điểm đầu nòng
    float v0 = power / 5.0f;
    float v0x = v0 * std::cos(angleRad);
    float v0y = v0 * std::sin(angleRad);
    for (int i = 0; i < TRAJECTORY_POINTS; i++) {
        float t = i * 0.1f; 
        float x = startPos.x + v0x * t;
        float y = startPos.y - v0y * t + 0.5f * GRAVITY * t * t;
        trajectoryPoints[i] = Vector2(x, y);
        if (x >= 0 && x < SCREEN_WIDTH) {
            int terrainHeight = terrain.getHeightAt(static_cast<int>(x));
            if (y >= terrainHeight) {
                for (int j = i + 1; j < TRAJECTORY_POINTS; j++) trajectoryPoints[j] = trajectoryPoints[i];
                break;
            }
        }
        // đường dự đoán quá giới hạn
        if (x < 0 || x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
            for (int j = i + 1; j < TRAJECTORY_POINTS; j++) trajectoryPoints[j] = trajectoryPoints[i];
            break;
        }
    }
}

void Tank::startAiming(int mouseX, int mouseY) {
    if(bullet == 0) return;
    isAiming = true;
    updateAiming(mouseX, mouseY);
}

void Tank::updateAiming(int mouseX, int mouseY) {
    if (!isAiming) return;
    float dx = position.x - mouseX;
    float dy = -(position.y - mouseY);
    if (facingRight) firingAngle = std::atan2(dy, dx) * 180 / PI;
    else firingAngle = std::atan2(dy, -dx) * 180 / PI;
    firingAngle = std::max(0.0f, std::min(90.0f, firingAngle));
    float distance = std::sqrt(dx * dx + dy * dy);
    power = 20 * std::min(100.0f, std::max(10.0f, distance / 5.0f));
}

Projectile Tank::fire() {
    Projectile projectile;
    if (isAiming) {
        Vector2 startPos = position;
        startPos.y -= 25; 
        float actualAngle;
        if (facingRight) {
            actualAngle = firingAngle;
            startPos.x += 25; 
        } else {
            actualAngle = 180.0f - firingAngle; 
            startPos.x -= 25; 
        }
        actualAngle -= angle;
        float angleRad = actualAngle * PI / 180.0f;
        float v0 = power / 5.0f; 
        Vector2 initialVelocity(
            v0 * std::cos(angleRad),
            -v0 * std::sin(angleRad) 
        );
        projectile.fire(startPos, initialVelocity);
        bullet--;
        isAiming = false; 
    }
    return projectile;
}

void Tank::move(int direction, const Terrain& terrain) {
    if (stamina <= 0) return; 
    position.x += direction * 4;
    position.x = std::max(20.0f, std::min(static_cast<float>(SCREEN_WIDTH - 20), position.x));
    stamina = std::max(0, stamina - 2);
    update(terrain);
}

void Tank::takeDamage(int damage) {
    health = std::max(0, health - damage);
}
void Tank::restoreStamina() {
    stamina = 100;
    bullet = 1;
}
Vector2 Tank::getPosition() const {
    return position;
}
int Tank::getHealth() const {
    return health;
}
bool Tank::isAlive() const {
    return health > 0;
}
void Tank::adjustFiringAngle(float delta) {
    firingAngle += delta;
    firingAngle = std::max(0.0f, std::min(90.0f, firingAngle));
}
void Tank::adjustPower(float delta) {
    power += delta;
    power = std::max(10.0f, std::min(100.0f, power));
}


Game::Game() 
    : isFiring(false), isMoving(false), tankChannel(-1), window(nullptr), renderer(nullptr), state(GameState::MENU), running(false),
    currentPlayerIndex(0), mouseDown(false), mouseX(0), mouseY(0), lastFrameTime(0),
    turnTimer(TURN_TIME_LIMIT), winnerIndex(-1), selectedMenuItem(static_cast<int>(MenuItem::NEW_GAME)),
    showInstructions(false), fontSmall(nullptr), fontMedium(nullptr), fontLarge(nullptr),
    explosionTexture(nullptr), menuBackgroundTexture(nullptr), 
    tankMoveSfx(nullptr), fireSfx(nullptr), explosionSfx(nullptr), backgroundMusic(nullptr), backgroundTexture(nullptr) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    }
Game::~Game() {
    cleanup();
}

bool Game::initialize() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    window = SDL_CreateWindow("Tank Star Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    int imgFlags = IMG_INIT_PNG;
    IMG_Init(imgFlags);
    fontSmall = TTF_OpenFont("../assets/font/arial.ttf", 14);     
    fontMedium = TTF_OpenFont("../assets/font/arial.ttf", 24);
    fontLarge = TTF_OpenFont("../assets/font/arial.ttf", 36);
    menuBackgroundTexture = IMG_LoadTexture(renderer, "../assets/images/background.png");
    tankMoveSfx = Mix_LoadWAV("../assets/sound/tank_move.wav");
    fireSfx = Mix_LoadWAV("../assets/sound/fire.wav");
    explosionSfx = Mix_LoadWAV("../assets/sound/explosion.wav");
    backgroundMusic = Mix_LoadMUS("../assets/sound/background_music.flac");
    musicVolume = 50; 
    sfxVolume = 30;   
    Mix_VolumeMusic(musicVolume);
    Mix_Volume(-1, sfxVolume);
    if (backgroundMusic) Mix_PlayMusic(backgroundMusic, -1);
    lastFrameTime = SDL_GetTicks();
    running = true;
    state = GameState::MENU;
    SDL_Surface* backgroundSurface = IMG_Load("../assets/images/nen.png");
    backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
    SDL_FreeSurface(backgroundSurface);
    return true;
}

void Game::initializeGame() {
    terrain = std::make_unique<Terrain>(renderer, SCREEN_WIDTH);
    wind = std::make_unique<Wind>();
    tanks.clear();
    tanks.push_back(std::make_unique<Tank>(renderer, Vector2(200, 0), SDL_Color{0, 150, 0, 255}, true));
    tanks.push_back(std::make_unique<Tank>(renderer, Vector2(SCREEN_WIDTH - 200, 0), SDL_Color{150, 0, 0, 255}, false));
    for (auto& tank : tanks) tank->update(*terrain);
    projectiles.clear();
    explosions.clear();
    initializeClouds();
    currentPlayerIndex = 0;
    mouseDown = false;
    turnTimer = TURN_TIME_LIMIT;
    winnerIndex = -1;
    state = GameState::PLAYING;
}

void Game::run() {
    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentTime;
        handleEvents();
        update(deltaTime);
        render();
        const int FPS = 60;
        const int frameDelay = 1000 / FPS;
        Uint32 frameTime = SDL_GetTicks() - currentTime;
        if (frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);
    }
}

void Game::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) running = false;
        else if (e.type == SDL_KEYDOWN) handleKeyDown(e.key.keysym.sym);
        else if (e.type == SDL_KEYUP) handleKeyUp(e.key.keysym.sym); 
        else if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (e.button.button == SDL_BUTTON_LEFT) handleMouseDown(e.button.x, e.button.y);
        } 
        else if (e.type == SDL_MOUSEBUTTONUP) {
            if (e.button.button == SDL_BUTTON_LEFT) handleMouseUp(e.button.x, e.button.y);
        } 
        else if (e.type == SDL_MOUSEMOTION) handleMouseMotion(e.motion.x, e.motion.y);
    }
}

void Game::handleMouseDown(int x, int y) {
    mouseDown = true;
    mouseX = x;
    mouseY = y;
    if (state == GameState::MENU) handleMenuClick(x, y);
    else if (state == GameState::PLAYING) tanks[currentPlayerIndex]->startAiming(x, y);
}

void Game::handleMouseUp(int x, int y) {
    if (mouseDown && state == GameState::PLAYING && !isFiring && projectiles.empty()) {  // Thêm kiểm tra projectiles.empty()
        Mix_PlayChannel(-1, fireSfx, 0);
        Projectile projectile = tanks[currentPlayerIndex]->fire();
        if (projectile.isActive()) {
            projectiles.push_back(projectile);
            isFiring = true;  // Khóa các thao tác
        }
    }
    mouseDown = false;
}

void Game::handleMouseMotion(int x, int y) {
    mouseX = x;
    mouseY = y;
    if (state == GameState::MENU) {
        int menuWidth = 300;
        int menuX = SCREEN_WIDTH / 2 - menuWidth / 2;
        int menuY = SCREEN_HEIGHT / 2;
        int menuItemHeight = 80;
        int menuSpacing = 20;
        bool foundHover = false;
        for (int i = 0; i < static_cast<int>(MenuItem::MENU_ITEM_COUNT); i++) {
            int itemWidth = menuWidth;
            int itemHeight = menuItemHeight;
            int itemX = SCREEN_WIDTH / 2 - itemWidth / 2;
            int itemY = menuY + i * (menuItemHeight + menuSpacing);
            if (x >= itemX && x < itemX + itemWidth &&
                y >= itemY && y < itemY + itemHeight) {
                selectedMenuItem = i;
                foundHover = true;
                break;
            }
        }
    }
    if (mouseDown && state == GameState::PLAYING && !isFiring && projectiles.empty()) {
        tanks[currentPlayerIndex]->updateAiming(x, y);
    }
}

void Game::handleMenuClick(int x, int y) {
    // Nếu đang ở màn hình Instructions
    if (showInstructions) {
        int panelWidth = 700;
        int panelHeight = 500;
        int panelX = SCREEN_WIDTH / 2 - panelWidth / 2;
        int panelY = SCREEN_HEIGHT / 2 - panelHeight / 2;
        
        int buttonWidth = 200;
        int buttonHeight = 50;
        int buttonX = panelX + panelWidth/2 - buttonWidth/2;
        int buttonY = panelY + panelHeight - 80;
        
        if (x >= buttonX && x < buttonX + buttonWidth &&
            y >= buttonY && y < buttonY + buttonHeight) {
            showInstructions = false;  // Chỉ tắt Instructions
            return;  // Return ngay để không xử lý các click khác
        }
        return;  // Nếu click ngoài nút khi đang ở Instructions thì cũng return
    }
    int menuWidth = 300;
    int menuX = SCREEN_WIDTH / 2 - menuWidth / 2;
    int menuY = SCREEN_HEIGHT / 2;
    int menuItemHeight = 80;
    int menuSpacing = 20;
    for (int i = 0; i < static_cast<int>(MenuItem::MENU_ITEM_COUNT); i++) {
        int itemWidth = menuWidth;
        int itemHeight = menuItemHeight;
        int itemX = SCREEN_WIDTH / 2 - itemWidth / 2;
        int itemY = menuY + i * (menuItemHeight + menuSpacing);
        if (x >= itemX && x < itemX + itemWidth &&
            y >= itemY && y < itemY + itemHeight) {
            selectedMenuItem = i;
            if (i == static_cast<int>(MenuItem::NEW_GAME)) initializeGame();
            else if (i == static_cast<int>(MenuItem::INSTRUCTIONS)) showInstructions = true;
            else if (i == static_cast<int>(MenuItem::EXIT)) running = false;
            break;
        }
    }
}

void Game::handleKeyDown(SDL_Keycode key) {
    if (state == GameState::MENU) handleMenuKeyDown(key);
    else if (state == GameState::PLAYING) handleGameKeyDown(key);
    else if (state == GameState::PAUSED) handlePauseKeyDown(key);
    else if (state == GameState::GAME_OVER) handleGameOverKeyDown(key);
}

void Game::handleMenuKeyDown(SDL_Keycode key) {
    if (showInstructions) {
        if (key == SDLK_ESCAPE || key == SDLK_RETURN) showInstructions = false;
        return;
    }
    switch (key) {
        case SDLK_UP:
            selectedMenuItem = (selectedMenuItem - 1 + static_cast<int>(MenuItem::MENU_ITEM_COUNT)) % static_cast<int>(MenuItem::MENU_ITEM_COUNT);
            break;
        case SDLK_DOWN:
            selectedMenuItem = (selectedMenuItem + 1) % static_cast<int>(MenuItem::MENU_ITEM_COUNT);
            break;
        case SDLK_RETURN:
        if (selectedMenuItem == static_cast<int>(MenuItem::NEW_GAME)) initializeGame();
        else if (selectedMenuItem == static_cast<int>(MenuItem::INSTRUCTIONS)) showInstructions = true;
        else if (selectedMenuItem == static_cast<int>(MenuItem::EXIT)) running = false;
            break;
    }
}

void Game::handleGameKeyDown(SDL_Keycode key) {
    if (isFiring || !projectiles.empty()) return;  // Thêm kiểm tra projectiles
    if (key == SDLK_LEFT || key == SDLK_a) {
        Mix_PlayChannel(-1, tankMoveSfx, 0);
        tanks[currentPlayerIndex]->move(-1, *terrain); 
        tankChannel = Mix_PlayChannel(-1, tankMoveSfx, 0);
    } 
    else if (key == SDLK_RIGHT || key == SDLK_d) {
        Mix_PlayChannel(-1, tankMoveSfx, -1);
        tanks[currentPlayerIndex]->move(1, *terrain); 
        tankChannel = Mix_PlayChannel(-1, tankMoveSfx, -1);
    } 
    else if (key == SDLK_ESCAPE) {
        state = GameState::PAUSED;
        selectedPauseMenuItem = 0; 
    }
}
void Game::handleKeyUp(SDL_Keycode key) {
    if (key == SDLK_LEFT || key == SDLK_RIGHT || key == SDLK_a || key == SDLK_d) Mix_HaltChannel(tankChannel);
}


void Game::handleGameOverKeyDown(SDL_Keycode key) {
    if (key == SDLK_r) initializeGame(); 
    else if (key == SDLK_ESCAPE || key == SDLK_m) state = GameState::MENU; 
}

void Game::handlePauseKeyDown(SDL_Keycode key) {
    switch (key) {
        case SDLK_ESCAPE:
            state = GameState::PLAYING;
            break;
        case SDLK_UP:
            selectedPauseMenuItem = (selectedPauseMenuItem - 1 + 4) % 4;
            break;
        case SDLK_DOWN:
            selectedPauseMenuItem = (selectedPauseMenuItem + 1) % 4;
            break;
        case SDLK_LEFT:
            if (selectedPauseMenuItem == 1) { 
                musicVolume = std::max(0, musicVolume - 10);
                Mix_VolumeMusic(musicVolume);
            } else if (selectedPauseMenuItem == 2) { 
                sfxVolume = std::max(0, sfxVolume - 10);
                Mix_Volume(-1, sfxVolume);
            }
            break;
        case SDLK_RIGHT:
            if (selectedPauseMenuItem == 1) { 
                musicVolume = std::min(128, musicVolume + 10);
                Mix_VolumeMusic(musicVolume);
            } else if (selectedPauseMenuItem == 2) { 
                sfxVolume = std::min(128, sfxVolume + 10);
                Mix_Volume(-1, sfxVolume);
            }
            break;
        case SDLK_RETURN:
            switch (selectedPauseMenuItem) {
                case 0: 
                    state = GameState::PLAYING;
                    break;
                case 3: 
                    state = GameState::MENU;
                    break;
            }
            break;
    }
}

void Game::update(float deltaTime) {
    if (state == GameState::PLAYING) {
        updateClouds(deltaTime);
        if (!isFiring) {  // Chỉ đếm thời gian khi không trong trạng thái bắn
            turnTimer -= deltaTime;
            if (turnTimer <= 0) {
                nextTurn();
                isFiring = false;  // Reset trạng thái bắn
            }
        }
        for (auto& tank : tanks) tank->update(*terrain); 
        for (auto& projectile : projectiles) {
            if (projectile.isActive()) {
                projectile.update(deltaTime, wind->getStrength(), *terrain);
                for (size_t i = 0; i < tanks.size(); i++) {
                    Vector2 tankPos = tanks[i]->getPosition();
                    Vector2 projectilePos = projectile.getPosition();
                    float distance = (tankPos - projectilePos).length();
                    if (distance < 38) { 
                        tanks[i]->takeDamage(20); 
                        explosions.emplace_back(projectilePos.x, projectilePos.y, 30); 
                        terrain->destroyAt(projectilePos.x, 35); 
                        projectile.deactivate(); 
                        nextTurn();
                        break;
                    }
                }                   
                if (!projectile.isActive()) {
                    Vector2 projectilePos = projectile.getPosition();
                    explosions.emplace_back(projectilePos.x, projectilePos.y, 30); 
                    terrain->destroyAt(projectilePos.x, 30); 
                }
            }
        }
        for (auto it = explosions.begin(); it != explosions.end();) {
            if(it == explosions.begin()) Mix_PlayChannel(-1, explosionSfx, 0);
            if (!it->update(deltaTime)) {
                it = explosions.erase(it);
                nextTurn();
            }
            else ++it;
        }
        projectiles.erase(
            std::remove_if(projectiles.begin(), projectiles.end(), [](const Projectile& p) { return !p.isActive(); }),
            projectiles.end()
        );
        checkGameOver(); 
    }
}

void Game::checkGameOver() {
    int aliveCount = 0;
    winnerIndex = -1;
    for (size_t i = 0; i < tanks.size(); i++) {
        if (tanks[i]->isAlive()) {
            aliveCount++;
            winnerIndex = i;
        }
    }
    if (aliveCount <= 1) state = GameState::GAME_OVER;
}

void Game::nextTurn() {
    if (tanks.empty()) return; 
    tanks[currentPlayerIndex]->restoreStamina();
    do {
        currentPlayerIndex = (currentPlayerIndex + 1) % tanks.size();
    } while (!tanks[currentPlayerIndex]->isAlive()); 
    wind->randomize();
    turnTimer = TURN_TIME_LIMIT;
    isFiring = 0;
}

void Game::cleanup() {
    if (menuBackgroundTexture) {
        SDL_DestroyTexture(menuBackgroundTexture);
        menuBackgroundTexture = nullptr;
    }
    if (fontSmall) {
        TTF_CloseFont(fontSmall);
        fontSmall = nullptr;
    }
    if (fontMedium) {
        TTF_CloseFont(fontMedium);
        fontMedium = nullptr;
    }
    if (fontLarge) {
        TTF_CloseFont(fontLarge);
        fontLarge = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    Mix_FreeChunk(tankMoveSfx);
    Mix_FreeChunk(fireSfx);
    Mix_FreeChunk(explosionSfx);
    Mix_FreeMusic(backgroundMusic);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    Mix_CloseAudio();
    if (backgroundTexture) {
        SDL_DestroyTexture(backgroundTexture);
        backgroundTexture = nullptr;
    }
}