#include <SDL2/SDL.h>
#include <iostream>
#include <vector>

// Constantes de la pantalla
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 600;

// Constantes de la paleta
const int PADDLE_WIDTH = 100;
const int PADDLE_HEIGHT = 15;
const int PADDLE_SPEED = 10;

// Constantes de la pelota
const int BALL_SIZE = 10;
const int BALL_SPEED_X = 8;
const int BALL_SPEED_Y = -8;

// Constantes de los ladrillos
const int BRICK_ROWS = 20;
const int BRICK_COLS = 40;
const int BRICK_WIDTH = SCREEN_WIDTH / BRICK_COLS;  // 80
const int BRICK_HEIGHT = 15;

// Colores (formato SDL_Color)
SDL_Color BACKGROUND_COLOR = {0, 0, 0, 255};        // Negro
SDL_Color PADDLE_COLOR = {255, 255, 255, 255};      // Blanco
SDL_Color BALL_COLOR = {255, 255, 0, 255};          // Amarillo
SDL_Color BRICK_COLORS[BRICK_ROWS] = {
    {255, 0, 0, 255},     // Rojo (fila 0)
    {255, 165, 0, 255},   // Naranja (fila 1)
    {255, 255, 0, 255},   // Amarillo (fila 2)
    {0, 255, 0, 255},     // Verde (fila 3)
    {0, 0, 255, 255}      // Azul (fila 4)
};

// Estructura para representar un ladrillo
struct Brick {
    SDL_Rect rect;
    bool active;
    SDL_Color color;
};

// Función para inicializar ladrillos
void initBricks(std::vector<Brick>& bricks) {
    bricks.clear();
    for (int row = 0; row < BRICK_ROWS; ++row) {
        for (int col = 0; col < BRICK_COLS; ++col) {
            Brick brick;
            brick.rect.x = col * BRICK_WIDTH;
            brick.rect.y = row * BRICK_HEIGHT;
            brick.rect.w = BRICK_WIDTH;
            brick.rect.h = BRICK_HEIGHT;
            brick.active = true;
            brick.color = BRICK_COLORS[row%5];
            bricks.push_back(brick);
        }
    }
}

// Función para manejar colisiones entre la pelota y los ladrillos
// Devuelve true si se destruyó algún ladrillo
bool handleBrickCollision(SDL_Rect& ball, int& vx, int& vy, std::vector<Brick>& bricks, int& score) {
    bool hit = false;
    for (auto& brick : bricks) {
        if (!brick.active) continue;

        // Detección de colisión AABB
        SDL_Rect overlap;
        if (SDL_IntersectRect(&ball, &brick.rect, &overlap)) {
            // Desactivar ladrillo
            brick.active = false;
            hit = true;
            score += 10;
            // Actualizar título con puntaje
            // (se hará fuera de la función para no llamar muchas veces)

            // Determinar el eje de colisión basado en el área de superposición
            if (overlap.w > overlap.h) {
                // Colisión vertical: cambiar dirección Y
                vy = -vy;
            } else {
                // Colisión horizontal: cambiar dirección X
                vx = -vx;
            }
            // Salir del bucle (solo una colisión por frame para evitar múltiples detecciones)
            break;
        }
    }
    return hit;
}

int main(int argc, char* argv[]) {
    // 1. Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Error SDL_Init: " << SDL_GetError() << std::endl;
        return 1;
    }

    // 2. Crear ventana
    SDL_Window* window = SDL_CreateWindow(
        "Atari Breakout - Puntaje: 0",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::cerr << "Error ventana: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // 3. Crear renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Error renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 4. Objetos del juego
    // Paleta
    SDL_Rect paddle;
    paddle.w = PADDLE_WIDTH;
    paddle.h = PADDLE_HEIGHT;
    paddle.x = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;
    paddle.y = SCREEN_HEIGHT - PADDLE_HEIGHT - 10;

    // Pelota
    SDL_Rect ball;
    ball.w = BALL_SIZE;
    ball.h = BALL_SIZE;
    ball.x = SCREEN_WIDTH / 2 - BALL_SIZE / 2;
    ball.y = SCREEN_HEIGHT / 2;
    int vx = BALL_SPEED_X;
    int vy = BALL_SPEED_Y;

    // Ladrillos
    std::vector<Brick> bricks;
    initBricks(bricks);

    int score = 0;
    bool running = true;
    bool gameOver = false;
    bool win = false;
    SDL_Event event;

    // Bucle principal
    while (running) {
        // ---- Manejo de eventos ----
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            // Teclado para mover la paleta
            if (event.type == SDL_KEYDOWN && !gameOver && !win) {
                const Uint8* state = SDL_GetKeyboardState(NULL);
                if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]) {
                    paddle.x -= PADDLE_SPEED;
                }
                if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) {
                    paddle.x += PADDLE_SPEED;
                }
            }
        }

        // Movimiento continuo de la paleta (para mayor fluidez)
        if (!gameOver && !win) {
            const Uint8* state = SDL_GetKeyboardState(NULL);
            if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]) {
                paddle.x -= PADDLE_SPEED;
            }
            if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) {
                paddle.x += PADDLE_SPEED;
            }
        }

        // Limitar la paleta dentro de la pantalla
        if (paddle.x < 0) paddle.x = 0;
        if (paddle.x + PADDLE_WIDTH > SCREEN_WIDTH) paddle.x = SCREEN_WIDTH - PADDLE_WIDTH;

        // ---- Lógica del juego (solo si no está terminado) ----
        if (!gameOver && !win) {
            // Mover pelota
            ball.x += vx;
            ball.y += vy;

            // Colisiones con bordes de la pantalla
            if (ball.x <= 0) { ball.x = 0; vx = -vx; }
            if (ball.x + BALL_SIZE >= SCREEN_WIDTH) { ball.x = SCREEN_WIDTH - BALL_SIZE; vx = -vx; }
            if (ball.y <= 0) { ball.y = 0; vy = -vy; }
            if (ball.y + BALL_SIZE >= SCREEN_HEIGHT) {
                // Pelota cayó al fondo: game over
                gameOver = true;
            }

            // Colisión con la paleta
            SDL_Rect overlap;
            if (SDL_IntersectRect(&ball, &paddle, &overlap)) {
                // Ajustar posición para que no se quede dentro
                ball.y = paddle.y - BALL_SIZE;
                vy = -vy;
                // Pequeño efecto lateral según donde golpea
                int hitPos = ball.x + BALL_SIZE/2 - (paddle.x + PADDLE_WIDTH/2);
                vx += hitPos / 10;
                // Limitar velocidad máxima para que no sea loca
                if (vx > 8) vx = 8;
                if (vx < -8) vx = -8;
            }

            // Colisión con ladrillos
            if (handleBrickCollision(ball, vx, vy, bricks, score)) {
                // Actualizar título con el puntaje
                std::string title = "Atari Breakout - Puntaje: " + std::to_string(score);
                SDL_SetWindowTitle(window, title.c_str());
            }

            // Verificar si ya no quedan ladrillos activos (victoria)
            bool anyActive = false;
            for (const auto& brick : bricks) {
                if (brick.active) {
                    anyActive = true;
                    break;
                }
            }
            if (!anyActive) {
                win = true;
            }
        }

        // ---- Renderizado ----
        // Limpiar pantalla
        SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, BACKGROUND_COLOR.a);
        SDL_RenderClear(renderer);

        // Dibujar ladrillos activos
        for (const auto& brick : bricks) {
            if (brick.active) {
                SDL_SetRenderDrawColor(renderer, brick.color.r, brick.color.g, brick.color.b, brick.color.a);
                SDL_RenderFillRect(renderer, &brick.rect);
                // Borde negro para cada ladrillo (opcional)
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &brick.rect);
            }
        }

        // Dibujar paleta
        SDL_SetRenderDrawColor(renderer, PADDLE_COLOR.r, PADDLE_COLOR.g, PADDLE_COLOR.b, PADDLE_COLOR.a);
        SDL_RenderFillRect(renderer, &paddle);

        // Dibujar pelota
        SDL_SetRenderDrawColor(renderer, BALL_COLOR.r, BALL_COLOR.g, BALL_COLOR.b, BALL_COLOR.a);
        SDL_RenderFillRect(renderer, &ball);

        // Mensajes de fin del juego
        if (gameOver) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            // Texto simple dibujando rectángulos (por simplicidad, no usamos TTF)
            // Mostrar mensaje en consola y en título
            std::string msg = "GAME OVER - Puntaje final: " + std::to_string(score);
            SDL_SetWindowTitle(window, msg.c_str());
            std::cout << msg << std::endl;
            // Esperar un poco y salir (o podrías esperar tecla)
            SDL_Delay(2000);
            running = false;
        }
        else if (win) {
            std::string msg = "¡VICTORIA! Puntaje: " + std::to_string(score);
            SDL_SetWindowTitle(window, msg.c_str());
            std::cout << msg << std::endl;
            SDL_Delay(2000);
            running = false;
        }

        // Presentar el renderizado
        SDL_RenderPresent(renderer);

        // Control de FPS (~60)
        SDL_Delay(16);
    }

    // Limpiar recursos
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}