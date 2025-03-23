#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <chrono>
#include <thread>
#include <SDL2/SDL.h>



// Game constants
const int MAP_WIDTH = 16;
const int MAP_HEIGHT = 16;
const int SCREEN_WIDTH = 800;  // Changed to better resolution for graphics
const int SCREEN_HEIGHT = 600;
const float FOV = 3.14159f / 3.0f;  // Wider FOV (60 degrees) to reduce motion sickness
const float DEPTH = 32.0f;  // Increased depth for better visibility
const float MOVE_SPEED = 5.0f;
const float ROTATION_SPEED = 3.0f;

// Add these new constants
const float MOUSE_SENSITIVITY = 0.002f;  // Reduced for smoother camera
const float ACCELERATION = 15.0f;  // Increased for more responsive movement
const float FRICTION = 8.0f;  // Increased for better stopping
const float MAX_SPEED = 5.0f;  // Reduced for more controlled movement

// Add these new constants
const float HEAD_BOB_SPEED = 10.0f;
const float HEAD_BOB_AMOUNT = 0.5f;
const float VERTICAL_FOV = 0.75f;  // Controls wall height

// Game map (# = wall, . = empty space)
std::string map = 
    "################"
    "#..............#"
    "#........#.....#"
    "#........#.....#"
    "#..............#"
    "#.......####...#"
    "#..............#"
    "#..............#"
    "#..#...........#"
    "#..#...........#"
    "#..#...........#"
    "#..#...........#"
    "#..............#"
    "#.........#....#"
    "#.........#....#"
    "################";

// Add these new structs before main()
struct Vector2 {
    float x, y;
    Vector2(float x = 0, float y = 0) : x(x), y(y) {}
    
    Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
    Vector2 operator*(float scalar) const { return Vector2(x * scalar, y * scalar); }
    float length() const { return sqrt(x*x + y*y); }
    
    Vector2 normalized() const {
        float len = length();
        return len > 0 ? Vector2(x/len, y/len) : Vector2();
    }
};

int main() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create window and renderer
    SDL_Window* window = SDL_CreateWindow("First Person Shooter",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cout << "Window creation failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cout << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Player position and direction
    float playerX = 8.0f;
    float playerY = 8.0f;
    float playerA = 0.0f;

    // Add these new variables after player position
    Vector2 velocity(0, 0);
    Vector2 input(0, 0);
    
    // Hide the mouse cursor and capture it
    SDL_SetRelativeMouseMode(SDL_TRUE);

    bool running = true;
    auto tp1 = std::chrono::system_clock::now();
    auto tp2 = tp1;

    float headBob = 0.0f;  // Add this with other player variables

    while (running) {
        tp2 = std::chrono::system_clock::now();
        std::chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();

        // Reset input vector each frame
        input = Vector2(0, 0);

        // Handle input
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            // Add mouse movement handling
            else if (event.type == SDL_MOUSEMOTION) {
                playerA += event.motion.xrel * MOUSE_SENSITIVITY;
            }
        }

        // Handle keyboard state for movement
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        if (keystate[SDL_SCANCODE_Q]) running = false;

        // Calculate movement direction based on input
        if (keystate[SDL_SCANCODE_W]) input.y += 1;
        if (keystate[SDL_SCANCODE_S]) input.y -= 1;
        if (keystate[SDL_SCANCODE_A]) input.x -= 1;
        if (keystate[SDL_SCANCODE_D]) input.x += 1;

        // Normalize input vector if it's not zero
        if (input.length() > 0) {
            input = input.normalized();
        }

        // Calculate acceleration
        Vector2 wishDir;
        if (input.length() > 0) {
            // Convert input to world space (fixed orientation)
            wishDir.x = input.x * cos(playerA - M_PI/2) - input.y * sin(playerA);
            wishDir.y = input.x * sin(playerA - M_PI/2) + input.y * cos(playerA);
            
            // Update head bob
            headBob += fElapsedTime * HEAD_BOB_SPEED;
        } else {
            // Smoothly reset head bob
            headBob = fmod(headBob, 2.0f * M_PI);
            if (headBob > 0) {
                headBob = std::max(0.0f, headBob - fElapsedTime * HEAD_BOB_SPEED);
            }
        }

        // Apply acceleration
        velocity = velocity + wishDir * (ACCELERATION * fElapsedTime);
        
        // Clamp to maximum speed
        float speed = velocity.length();
        if (speed > MAX_SPEED) {
            velocity = velocity * (MAX_SPEED / speed);
        }

        // Apply friction
        speed = velocity.length();
        if (speed > 0) {
            float drop = speed * FRICTION * fElapsedTime;
            velocity = velocity * (speed > drop ? (speed - drop) / speed : 0);
        }

        // Update position
        float newX = playerX + velocity.x * fElapsedTime;
        float newY = playerY + velocity.y * fElapsedTime;

        // Collision checking
        if (map.at((int)playerY * MAP_WIDTH + (int)newX) != '#') {
            playerX = newX;
        }
        if (map.at((int)newY * MAP_WIDTH + (int)playerX) != '#') {
            playerY = newY;
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Ray casting
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            float rayAngle = (playerA - FOV / 2.0f) + ((float)x / (float)SCREEN_WIDTH) * FOV;
            
            // Improved ray step size for better performance and quality
            float stepSize = 0.01f;
            float distanceToWall = 0.0f;
            bool hitWall = false;
            bool boundary = false;  // For edge detection

            float rayX = sin(rayAngle);
            float rayY = cos(rayAngle);

            // Normalized ray direction for better accuracy
            float rayLen = sqrt(rayX * rayX + rayY * rayY);
            rayX /= rayLen;
            rayY /= rayLen;

            while (!hitWall && distanceToWall < DEPTH) {
                distanceToWall += stepSize;
                
                int testX = (int)(playerX + rayX * distanceToWall);
                int testY = (int)(playerY + rayY * distanceToWall);
                
                if (testX < 0 || testX >= MAP_WIDTH || testY < 0 || testY >= MAP_HEIGHT) {
                    hitWall = true;
                    distanceToWall = DEPTH;
                }
                else if (map.at(testY * MAP_WIDTH + testX) == '#') {
                    hitWall = true;
                    
                    // Check for wall boundaries for shading
                    float blockMidX = testX + 0.5f;
                    float blockMidY = testY + 0.5f;
                    float testPointX = playerX + rayX * distanceToWall;
                    float testPointY = playerY + rayY * distanceToWall;
                    float testAngle = atan2f(testPointY - blockMidY, testPointX - blockMidX);
                    
                    if (abs(testPointX - (float)testX) < 0.01f || abs(testPointX - (testX + 1)) < 0.01f ||
                        abs(testPointY - (float)testY) < 0.01f || abs(testPointY - (testY + 1)) < 0.01f)
                        boundary = true;
                }
            }

            // Fix fish-eye effect
            float adjustedDistance = distanceToWall * cos(rayAngle - playerA);
            
            // Calculate wall height with perspective correction
            int ceiling = (float)(SCREEN_HEIGHT / 2.0) - SCREEN_HEIGHT / (adjustedDistance * VERTICAL_FOV);
            int floor = SCREEN_HEIGHT - ceiling;
            
            // Add subtle head bob to ceiling and floor
            float bobOffset = sin(headBob) * HEAD_BOB_AMOUNT;
            ceiling += (int)bobOffset;
            floor += (int)bobOffset;

            // Improved wall shading
            Uint8 shade = (Uint8)(255.0f * (1.0f - adjustedDistance / DEPTH));
            if (boundary) shade = shade * 0.7f;  // Darker edges for depth
            
            // Draw wall with improved colors
            SDL_SetRenderDrawColor(renderer, shade, shade * 0.8f, shade * 0.6f, 255);
            SDL_RenderDrawLine(renderer, x, ceiling, x, floor);

            // Improved sky gradient
            for (int y = 0; y < ceiling; y++) {
                float skyGradient = (float)y / (float)SCREEN_HEIGHT;
                SDL_SetRenderDrawColor(renderer, 
                    (Uint8)(100 + skyGradient * 55),
                    (Uint8)(150 + skyGradient * 55),
                    255, 255);
                SDL_RenderDrawPoint(renderer, x, y);
            }

            // Improved floor with distance shading
            for (int y = floor; y < SCREEN_HEIGHT; y++) {
                float distanceFromCenter = (float)(y - SCREEN_HEIGHT/2) / (float)SCREEN_HEIGHT;
                float floorShade = 1.0f - distanceFromCenter;
                SDL_SetRenderDrawColor(renderer, 
                    (Uint8)(50 * floorShade),
                    (Uint8)(50 * floorShade),
                    (Uint8)(50 * floorShade), 255);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }

        // Draw minimap
        const int MINIMAP_SCALE = 4;
        for (int x = 0; x < MAP_WIDTH; x++) {
            for (int y = 0; y < MAP_HEIGHT; y++) {
                if (map[y * MAP_WIDTH + x] == '#') {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    SDL_Rect rect = {x * MINIMAP_SCALE, y * MINIMAP_SCALE,
                                   MINIMAP_SCALE, MINIMAP_SCALE};
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }

        // Draw player on minimap
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect playerRect = {(int)(playerX * MINIMAP_SCALE) - 2,
                             (int)(playerY * MINIMAP_SCALE) - 2, 4, 4};
        SDL_RenderFillRect(renderer, &playerRect);

        // Present render
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}