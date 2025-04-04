#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <cmath>
#include <algorithm>
#include <string>
#include <ctime>

using namespace std;

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;       // kích thước màn hình
const float PI = 3.14159265f;  
const float GRAVITY = 9.8f * 10.5;   // hệ số trọng lực 
const int TRAJECTORY_POINTS = 15;    // số điểm của đường dự đoán
const float TURN_TIME_LIMIT = 30.0f; // thời gian lượt
const float FREEZE_TIME = 1.0f;      // thời gian ngưng đọng

// thao tác xử lí vector 2 chiều
struct Vector2 { 
    float x, y;
    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) : x(x), y(y) {}  

    Vector2 operator+(const Vector2& v) const;
    Vector2 operator-(const Vector2& v) const;
    Vector2 operator*(float scalar) const;
    float length() const;
    Vector2 normalize() const;
};
// cấu trúc mây
struct Cloud {
    Vector2 position;
    float width;
    float height;  // kích thước mây
    float speed;   // tốc độ bay
    float alpha;   // độ mờ
    
    Cloud(float x, float y, float w, float h, float a) 
        : position{x, y}, width(w), height(h), alpha(a), speed(0) {}
};
// cấu trúc vụ nổ con
struct Particle {
    Vector2 position;   // toạ độ (theo 2 trục xy)
    Vector2 velocity;   // vận tốc (theo 2 trục xy)
    float lifetime;     // thời gian hiển thị - từ lúc sinh ra đến hiện tại
    float maxLifetime;  // thời gian tồn tại max
    SDL_Color color;    // màu sắc 
    
    Particle(float x, float y, float vx, float vy, float lifetime, SDL_Color color);
    bool update(float deltaTime);   // update chuyển động theo thời gian
};
// cấu trúc vụ nổ
struct Explosion {
    Vector2 position;                   // toạ độ vụ nổ
    float radius;                       // bán kính hiện tại của vụ nổ
    float maxRadius;                    // bán kính max
    bool growing;                       // trạng thái của vụ nổ - có còn đang phát triển (mở rộng) không ?
    float speed;                        // tốc độ mở rộng
    std::vector<Particle> particles;    //vector lưu những vụ nổ con
    
    Explosion(float x, float y, float maxRadius);
    bool update(float deltaTime);       // update ...
};
// Lớp Địa hình
class Terrain {
private:
    SDL_Renderer* renderer;     
    SDL_Texture* terrainTexture;
    std::vector<int> heightMap;
    bool needsUpdate;
    
public:
    Terrain(SDL_Renderer* renderer, int width);
    void generateTerrain();
    int getHeightAt(int x) const;
    void destroyAt(int x, int radius);
    void smoothTerrain(int startX, int endX);
    void render();
};

class Wind {
private:
    float strength;
    const float level[4] = {0.0f, 12.0f, 20.0f, 30.0f};
    
public:
    Wind();
    void randomize();
    float getStrength() const;
};

class Projectile {
private:
    Vector2 position;
    Vector2 velocity;
    bool active;
    
public:
    Projectile();
    void fire(const Vector2& startPos, const Vector2& initialVelocity);
    void update(float deltaTime, float windStrength, const Terrain& terrain);
    bool isActive() const;
    Vector2 getPosition() const;
    void deactivate();
    void render(SDL_Renderer* renderer);
};

class Tank {
private:
    SDL_Renderer* renderer;
    Vector2 position;
    float angle;
    float firingAngle;
    float power;
    int health;
    int stamina;
    int bullet;
    bool isAiming;
    SDL_Color color;
    bool facingRight;
    int width;
    int height;
    SDL_Texture* tankTexture;
    Vector2 trajectoryPoints[TRAJECTORY_POINTS];
    const Terrain* terrain;
    
public:
    Tank(SDL_Renderer* renderer, const Vector2& startPos, const SDL_Color& tankColor, bool facingRight);
    ~Tank();
    void setTerrain(const Terrain* t);
    void update(const Terrain& terrain);
    void calculateTrajectory(const Terrain& terrain);
    void startAiming(int mouseX, int mouseY);
    void updateAiming(int mouseX, int mouseY);
    Projectile fire();
    void move(int direction, const Terrain& terrain);
    void takeDamage(int damage);
    void restoreStamina();
    Vector2 getPosition() const;
    int getHealth() const;
    bool isAlive() const;
    void adjustFiringAngle(float delta);
    void adjustPower(float delta);
    void render();
    void renderTrajectory();
};

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

enum class MenuItem {
    NEW_GAME,
    INSTRUCTIONS,
    EXIT,
    MENU_ITEM_COUNT
};

class Game {
private:
    std::vector<Cloud> clouds;
    bool isFiring;
    bool isMoving;
    int tankChannel;
    SDL_Window* window;
    SDL_Renderer* renderer;
    GameState state;
    bool running;
    std::unique_ptr<Terrain> terrain;
    std::unique_ptr<Wind> wind;
    std::vector<std::unique_ptr<Tank>> tanks;
    std::vector<Projectile> projectiles;
    std::vector<Explosion> explosions;
    size_t currentPlayerIndex;
    bool mouseDown;
    int mouseX, mouseY;
    Uint32 lastFrameTime;
    float turnTimer;
    int winnerIndex;
    int selectedMenuItem;
    int selectedPauseMenuItem;
    bool showInstructions;
    TTF_Font* fontSmall;
    TTF_Font* fontMedium;
    TTF_Font* fontLarge;
    SDL_Texture* explosionTexture;
    SDL_Texture* menuBackgroundTexture;
    Mix_Chunk* tankMoveSfx;
    Mix_Chunk* fireSfx;
    Mix_Chunk* explosionSfx;
    Mix_Music* backgroundMusic;
    int musicVolume;
    int sfxVolume;
    SDL_Texture* backgroundTexture;
    
public:
    Game();
    ~Game();
    void initializeClouds();
    void updateClouds(float deltaTime);
    void renderClouds();
    bool initialize();
    void loadAssets();
    void initializeGame();
    void run();
    void handleEvents();
    void handleMouseDown(int x, int y);
    void handleMouseUp(int x, int y);
    void handleMouseMotion(int x, int y);
    void handleMenuClick(int x, int y);
    void handleKeyDown(SDL_Keycode key);
    void handleKeyUp(SDL_Keycode key);
    void handleMenuKeyDown(SDL_Keycode key);
    void handleGameKeyDown(SDL_Keycode key);
    void handleGameOverKeyDown(SDL_Keycode key);
    void handlePauseKeyDown(SDL_Keycode key);
    void update(float deltaTime);
    void checkGameOver();
    void nextTurn();
    void cleanup();
    void render();
    void renderExplosions();
    void renderMenu();
    void renderInstructions();
    void renderTurnInfo();
    void renderGameOver();
    void renderPauseMenu();
    void createExplosionTexture();
};

#endif // GAME_H

