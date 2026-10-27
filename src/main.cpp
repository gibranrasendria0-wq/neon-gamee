#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

enum class Screen { Menu, Playing, Paused, GameOver };
enum class Difficulty { Easy, Normal, Hard };

struct Bullet {
    Vector2 pos;
    Vector2 vel;
    float radius = 5.0f;
    bool alive = true;
};

struct Enemy {
    Vector2 pos;
    float radius = 13.0f;
    float speed = 80.0f;
    float hp = 1.0f;
    float maxHp = 1.0f;
    bool alive = true;
    Color color = RED;
};

struct Particle {
    Vector2 pos;
    Vector2 vel;
    float life = 0.35f;
    float maxLife = 0.35f;
    float radius = 3.0f;
    Color color = SKYBLUE;
};

static Vector2 Add(Vector2 a, Vector2 b) { return {a.x + b.x, a.y + b.y}; }
static Vector2 Sub(Vector2 a, Vector2 b) { return {a.x - b.x, a.y - b.y}; }
static Vector2 Mul(Vector2 a, float s) { return {a.x * s, a.y * s}; }
static float Len(Vector2 v) { return std::sqrt(v.x * v.x + v.y * v.y); }
static Vector2 Normalize(Vector2 v) {
    float l = Len(v);
    return l > 0.001f ? Vector2{v.x / l, v.y / l} : Vector2{0, 0};
}

int main() {
    const int screenW = 1100;
    const int screenH = 700;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(screenW, screenH, "NEON SURVIVOR");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    Screen screen = Screen::Menu;
    Difficulty difficulty = Difficulty::Normal;
    Vector2 player = {screenW / 2.0f, screenH / 2.0f};
    const float playerRadius = 14.0f;
    float hp = 100.0f;
    float score = 0.0f;
    float survived = 0.0f;
    float wave = 1.0f;
    float spawnTimer = 0.0f;
    float shootTimer = 0.0f;
    float invulnTimer = 0.0f;
    int highScore = 0;
    bool rapidFire = false;
    float rapidTimer = 0.0f;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    std::vector<Particle> particles;

    auto resetGame = [&]() {
        player = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
        hp = 100.0f; score = 0.0f; survived = 0.0f; wave = 1.0f;
        spawnTimer = 0.0f; shootTimer = 0.0f; invulnTimer = 0.0f;
        rapidFire = false; rapidTimer = 0.0f;
        bullets.clear(); enemies.clear(); particles.clear();
        screen = Screen::Playing;
    };

    auto burst = [&](Vector2 at, Color color, int amount) {
        for (int i = 0; i < amount; ++i) {
            float a = (float)GetRandomValue(0, 359) * DEG2RAD;
            float s = (float)GetRandomValue(35, 180);
            particles.push_back({at, {std::cos(a) * s, std::sin(a) * s},
                                 GetRandomValue(18, 45) / 100.0f,
                                 0.45f, (float)GetRandomValue(2, 5), color});
        }
    };

    while (!WindowShouldClose()) {
        float dt = std::min(GetFrameTime(), 0.033f);
        int w = GetScreenWidth();
        int h = GetScreenHeight();
        Vector2 mouse = GetMousePosition();

        if (screen == Screen::Menu) {
            if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Rectangle start = {(float)w/2 - 120, (float)h/2 + 35, 240, 54};
                if (IsKeyPressed(KEY_ENTER) || CheckCollisionPointRec(mouse, start)) resetGame();
                Rectangle easy = {(float)w/2 - 150, (float)h/2 + 115, 95, 34};
                Rectangle normal = {(float)w/2 - 47, (float)h/2 + 115, 95, 34};
                Rectangle hard = {(float)w/2 + 56, (float)h/2 + 115, 95, 34};
                if (CheckCollisionPointRec(mouse, easy)) difficulty = Difficulty::Easy;
                if (CheckCollisionPointRec(mouse, normal)) difficulty = Difficulty::Normal;
                if (CheckCollisionPointRec(mouse, hard)) difficulty = Difficulty::Hard;
            }
        } else if (screen == Screen::Playing) {
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) screen = Screen::Paused;
            else {
                survived += dt;
                wave = 1.0f + std::floor(survived / 18.0f);
                invulnTimer = std::max(0.0f, invulnTimer - dt);
                rapidTimer = std::max(0.0f, rapidTimer - dt);
                if (rapidTimer <= 0) rapidFire = false;

                Vector2 move = {0, 0};
                if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) move.y -= 1;
                if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) move.y += 1;
                if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) move.x -= 1;
                if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) move.x += 1;
                move = Normalize(move);
                float playerSpeed = 270.0f;
                player.x += move.x * playerSpeed * dt;
                player.y += move.y * playerSpeed * dt;
                player.x = std::clamp(player.x, playerRadius + 8, (float)w - playerRadius - 8);
                player.y = std::clamp(player.y, playerRadius + 55, (float)h - playerRadius - 8);

                shootTimer -= dt;
                float fireRate = rapidFire ? 0.10f : 0.23f;
                if ((IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsKeyDown(KEY_SPACE)) && shootTimer <= 0) {
                    Vector2 direction = Normalize(Sub(mouse, player));
                    if (Len(direction) < 0.1f) direction = {1, 0};
                    bullets.push_back({Add(player, Mul(direction, playerRadius + 5)), Mul(direction, 560), 5, true});
                    shootTimer = fireRate;
                    burst(Add(player, Mul(direction, 15)), SKYBLUE, 2);
                }

                spawnTimer -= dt;
                float spawnEvery = std::max(0.30f, 1.20f - wave * 0.055f);
                if (spawnTimer <= 0) {
                    int side = GetRandomValue(0, 3);
                    Vector2 p;
                    if (side == 0) p = {(float)GetRandomValue(0, w), 48};
                    else if (side == 1) p = {(float)w - 8, (float)GetRandomValue(55, h)};
                    else if (side == 2) p = {(float)GetRandomValue(0, w), (float)h - 8};
                    else p = {8, (float)GetRandomValue(55, h)};
                    bool tank = ((int)wave >= 4 && GetRandomValue(0, 100) < 24);
                    float enemyHp = tank ? 3.0f : 1.0f;
                    float enemySpeed = (tank ? 48.0f : 72.0f) + (float)wave * 5.0f;
                    if (difficulty == Difficulty::Easy) enemySpeed *= 0.82f;
                    if (difficulty == Difficulty::Hard) { enemySpeed *= 1.22f; enemyHp += tank ? 1.0f : 0.0f; }
                    enemies.push_back({p, tank ? 19.0f : 12.0f, enemySpeed, enemyHp, enemyHp, true,
                                       tank ? Color{255, 90, 160, 255} : Color{170, 88, 255, 255}});
                    spawnTimer = spawnEvery;
                }

                for (auto &b : bullets) {
                    if (!b.alive) continue;
                    b.pos = Add(b.pos, Mul(b.vel, dt));
                    if (b.pos.x < -20 || b.pos.x > w + 20 || b.pos.y < -20 || b.pos.y > h + 20) b.alive = false;
                }

                for (auto &e : enemies) {
                    if (!e.alive) continue;
                    Vector2 dir = Normalize(Sub(player, e.pos));
                    e.pos = Add(e.pos, Mul(dir, e.speed * dt));
                    if (CheckCollisionCircles(player, playerRadius, e.pos, e.radius) && invulnTimer <= 0) {
                        hp -= difficulty == Difficulty::Hard ? 18.0f : 13.0f;
                        invulnTimer = 0.65f;
                        burst(player, RED, 12);
                        Vector2 knock = Normalize(Sub(player, e.pos));
                        player = Add(player, Mul(knock, 24));
                        if (hp <= 0) {
                            hp = 0;
                            highScore = std::max(highScore, (int)score);
                            screen = Screen::GameOver;
                        }
                    }
                    for (auto &b : bullets) {
                        if (e.alive && b.alive && CheckCollisionCircles(b.pos, b.radius, e.pos, e.radius)) {
                            b.alive = false;
                            e.hp -= 1;
                            burst(b.pos, SKYBLUE, 4);
                            if (e.hp <= 0) {
                                e.alive = false;
                                score += e.maxHp > 1 ? 30 : 10;
                                burst(e.pos, e.color, 14);
                                if (GetRandomValue(0, 100) < 8) {
                                    rapidFire = true; rapidTimer = 5.0f;
                                }
                            }
                        }
                    }
                }

                for (auto &p : particles) {
                    p.life -= dt;
                    p.pos = Add(p.pos, Mul(p.vel, dt));
                    p.vel = Mul(p.vel, 0.94f);
                }
                bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& b){return !b.alive;}), bullets.end());
                enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](const Enemy& e){return !e.alive;}), enemies.end());
                particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle& p){return p.life <= 0;}), particles.end());
            }
        } else if (screen == Screen::Paused) {
            if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) screen = Screen::Playing;
            if (IsKeyPressed(KEY_ENTER)) resetGame();
            if (IsKeyPressed(KEY_M)) screen = Screen::Menu;
        } else if (screen == Screen::GameOver) {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_R)) resetGame();
            if (IsKeyPressed(KEY_M)) screen = Screen::Menu;
        }

        BeginDrawing();
        ClearBackground(Color{8, 10, 24, 255});

        // Subtle neon grid.
        for (int x = 0; x < w; x += 40) DrawLine(x, 0, x, h, Color{23, 30, 58, 110});
        for (int y = 0; y < h; y += 40) DrawLine(0, y, w, y, Color{23, 30, 58, 110});

        if (screen == Screen::Menu) {
            DrawText("NEON", w/2 - MeasureText("NEON", 76)/2, h/2 - 170, 76, SKYBLUE);
            DrawText("SURVIVOR", w/2 - MeasureText("SURVIVOR", 44)/2, h/2 - 95, 44, RAYWHITE);
            DrawText("SURVIVE THE NIGHT", w/2 - MeasureText("SURVIVE THE NIGHT", 18)/2, h/2 - 42, 18, Color{156, 164, 200, 255});
            Rectangle start = {(float)w/2 - 120, (float)h/2 + 35, 240, 54};
            DrawRectangleRounded(start, 0.2f, 8, CheckCollisionPointRec(mouse, start) ? Color{40, 220, 240, 255} : Color{22, 80, 110, 255});
            DrawRectangleRoundedLines(start, 0.2f, 8, SKYBLUE);
            DrawText("START GAME", w/2 - MeasureText("START GAME", 20)/2, h/2 + 52, 20, RAYWHITE);
            DrawText("DIFFICULTY", w/2 - MeasureText("DIFFICULTY", 14)/2, h/2 + 100, 14, GRAY);
            Rectangle easy = {(float)w/2 - 150, (float)h/2 + 122, 95, 34};
            Rectangle normal = {(float)w/2 - 47, (float)h/2 + 122, 95, 34};
            Rectangle hard = {(float)w/2 + 56, (float)h/2 + 122, 95, 34};
            Rectangle choices[] = {easy, normal, hard};
            const char* labels[] = {"EASY", "NORMAL", "HARD"};
            for (int i=0;i<3;i++) {
                bool selected = (i==0 && difficulty==Difficulty::Easy) || (i==1 && difficulty==Difficulty::Normal) || (i==2 && difficulty==Difficulty::Hard);
                DrawRectangleRounded(choices[i], 0.2f, 6, selected ? Color{35, 90, 120, 255} : Color{18, 23, 43, 255});
                DrawRectangleRoundedLines(choices[i], 0.2f, 6, selected ? SKYBLUE : Color{57, 67, 99, 255});
                DrawText(labels[i], (int)choices[i].x + ((int)choices[i].width - MeasureText(labels[i], 13))/2, (int)choices[i].y + 10, 13, RAYWHITE);
            }
            DrawText("WASD / ARROWS TO MOVE  -  MOUSE / SPACE TO SHOOT", w/2 - MeasureText("WASD / ARROWS TO MOVE  -  MOUSE / SPACE TO SHOOT", 14)/2, h - 48, 14, Color{126, 139, 174, 255});
            DrawText(TextFormat("BEST SCORE  %04i", highScore), 24, 24, 16, Color{126, 139, 174, 255});
        } else {
            // HUD.
            DrawRectangle(0, 0, w, 48, Color{10, 14, 32, 245});
            DrawText("NEON SURVIVOR", 20, 16, 16, SKYBLUE);
            DrawText(TextFormat("WAVE %.0f", wave), w/2 - 40, 16, 16, RAYWHITE);
            DrawText(TextFormat("SCORE %05i", (int)score), w - 155, 16, 16, RAYWHITE);
            DrawText("HP", 22, 62, 13, RAYWHITE);
            DrawRectangle(54, 62, 180, 12, Color{45, 49, 72, 255});
            DrawRectangle(54, 62, (int)(180 * hp / 100.0f), 12, hp > 30 ? Color{51, 231, 177, 255} : RED);
            DrawText(TextFormat("%i%%", (int)hp), 242, 60, 14, RAYWHITE);

            for (const auto &p : particles) {
                float alpha = std::clamp(p.life / p.maxLife, 0.0f, 1.0f);
                Color c = p.color; c.a = (unsigned char)(255 * alpha);
                DrawCircleV(p.pos, p.radius * alpha, c);
            }
            for (const auto &b : bullets) {
                DrawCircleV(b.pos, 9, Color{40, 210, 255, 55});
                DrawCircleV(b.pos, b.radius, SKYBLUE);
            }
            for (const auto &e : enemies) {
                DrawCircleV(e.pos, e.radius + 5, Color{e.color.r, e.color.g, e.color.b, 35});
                DrawCircleV(e.pos, e.radius, e.color);
                DrawCircleV(e.pos, e.radius * 0.45f, Color{20, 18, 40, 255});
                if (e.maxHp > 1) {
                    DrawRectangle((int)e.pos.x - 18, (int)e.pos.y - e.radius - 9, 36, 4, DARKGRAY);
                    DrawRectangle((int)e.pos.x - 18, (int)e.pos.y - e.radius - 9, (int)(36 * e.hp / e.maxHp), 4, PINK);
                }
            }
            if (screen == Screen::Playing || screen == Screen::Paused) {
                Color pc = (invulnTimer > 0 && ((int)(invulnTimer * 18) % 2 == 0)) ? WHITE : SKYBLUE;
                DrawCircleV(player, playerRadius + 7, Color{40, 220, 255, 35});
                DrawCircleV(player, playerRadius, pc);
                DrawCircleV(player, 5, Color{10, 25, 45, 255});
                DrawLineEx(player, Add(player, Mul(Normalize(Sub(mouse, player)), 22)), 3, RAYWHITE);
            }
            DrawText(TextFormat("TIME %.0fs", survived), 20, h - 28, 14, Color{126, 139, 174, 255});
            DrawText("P / ESC  PAUSE", w - 145, h - 28, 14, Color{126, 139, 174, 255});
            if (rapidFire && screen == Screen::Playing) DrawText(TextFormat("RAPID FIRE %.1fs", rapidTimer), w/2 - 65, 58, 14, GOLD);
        }

        if (screen == Screen::Paused || screen == Screen::GameOver) {
            DrawRectangle(0, 0, w, h, Color{3, 5, 15, 185});
            const char* title = screen == Screen::Paused ? "PAUSED" : "GAME OVER";
            DrawText(title, w/2 - MeasureText(title, 46)/2, h/2 - 95, 46, screen == Screen::Paused ? SKYBLUE : PINK);
            if (screen == Screen::GameOver) {
                DrawText(TextFormat("FINAL SCORE  %05i", (int)score), w/2 - 100, h/2 - 28, 20, RAYWHITE);
                DrawText(TextFormat("SURVIVED  %.0f SECONDS", survived), w/2 - 112, h/2 + 5, 16, Color{156, 164, 200, 255});
                DrawText("ENTER / R  PLAY AGAIN", w/2 - 115, h/2 + 58, 16, SKYBLUE);
            } else {
                DrawText("P / ESC  RESUME", w/2 - 80, h/2 - 20, 16, RAYWHITE);
                DrawText("ENTER  RESTART     M  MAIN MENU", w/2 - 155, h/2 + 18, 16, Color{156, 164, 200, 255});
            }
            if (screen == Screen::GameOver) DrawText("M  MAIN MENU", w/2 - 55, h/2 + 90, 16, Color{156, 164, 200, 255});
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
