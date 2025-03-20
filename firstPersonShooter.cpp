#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <chrono>
#include <thread>

// Game constants
const int MAP_WIDTH = 16;
const int MAP_HEIGHT = 16;
const int SCREEN_WIDTH = 200;
const int SCREEN_HEIGHT = 60;
const float FOV = 3.14159f / 4.0f;
const float DEPTH = 16.0f;
const float MOVE_SPEED = 5.0f;
const float ROTATION_SPEED = 3.0f;

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

// Non-blocking keyboard input for macOS
char getch_nonblock() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0) perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 0;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0) perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0) perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0) perror("tcsetattr ~ICANON");
    return buf;
}

int main() {
    // Player position and direction
    float playerX = 8.0f;
    float playerY = 8.0f;
    float playerA = 0.0f;  // Player angle
    
    // Screen buffer
    std::vector<char> screen(SCREEN_WIDTH * SCREEN_HEIGHT, ' ');
    
    // Game loop
    auto tp1 = std::chrono::system_clock::now();
    auto tp2 = tp1;
    
    while(1) {
        tp2 = std::chrono::system_clock::now();
        std::chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();
        
        // Handle input
        char key = getch_nonblock();
        
        // Exit game
        if (key == 'q')
            break;
        
        // Move forward
        if (key == 'w') {
            playerX += sinf(playerA) * MOVE_SPEED * fElapsedTime;
            playerY += cosf(playerA) * MOVE_SPEED * fElapsedTime;
            if (map.at((int)playerY * MAP_WIDTH + (int)playerX) == '#') {
                playerX -= sinf(playerA) * MOVE_SPEED * fElapsedTime;
                playerY -= cosf(playerA) * MOVE_SPEED * fElapsedTime;
            }
        }
        
        // Move backward
        if (key == 's') {
            playerX -= sinf(playerA) * MOVE_SPEED * fElapsedTime;
            playerY -= cosf(playerA) * MOVE_SPEED * fElapsedTime;
            if (map.at((int)playerY * MAP_WIDTH + (int)playerX) == '#') {
                playerX += sinf(playerA) * MOVE_SPEED * fElapsedTime;
                playerY += cosf(playerA) * MOVE_SPEED * fElapsedTime;
            }
        }
        
        // Strafe left
        if (key == 'a') {
            playerX -= cosf(playerA) * MOVE_SPEED * fElapsedTime;
            playerY += sinf(playerA) * MOVE_SPEED * fElapsedTime;
            if (map.at((int)playerY * MAP_WIDTH + (int)playerX) == '#') {
                playerX += cosf(playerA) * MOVE_SPEED * fElapsedTime;
                playerY -= sinf(playerA) * MOVE_SPEED * fElapsedTime;
            }
        }
        
        // Strafe right
        if (key == 'd') {
            playerX += cosf(playerA) * MOVE_SPEED * fElapsedTime;
            playerY -= sinf(playerA) * MOVE_SPEED * fElapsedTime;
            if (map.at((int)playerY * MAP_WIDTH + (int)playerX) == '#') {
                playerX -= cosf(playerA) * MOVE_SPEED * fElapsedTime;
                playerY += sinf(playerA) * MOVE_SPEED * fElapsedTime;
            }
        }
        
        // Turn left
        if (key == 'j')
            playerA -= ROTATION_SPEED * fElapsedTime;
        
        // Turn right
        if (key == 'l')
            playerA += ROTATION_SPEED * fElapsedTime;
        
        // Ray casting
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            // Calculate ray angle for this column
            float rayAngle = (playerA - FOV / 2.0f) + ((float)x / (float)SCREEN_WIDTH) * FOV;
            
            float distanceToWall = 0.0f;
            bool hitWall = false;
            
            float rayX = sinf(rayAngle);
            float rayY = cosf(rayAngle);
            
            while (!hitWall && distanceToWall < DEPTH) {
                distanceToWall += 0.1f;
                
                int testX = (int)(playerX + rayX * distanceToWall);
                int testY = (int)(playerY + rayY * distanceToWall);
                
                // Test if ray is out of bounds
                if (testX < 0 || testX >= MAP_WIDTH || testY < 0 || testY >= MAP_HEIGHT) {
                    hitWall = true;
                    distanceToWall = DEPTH;
                }
                else if (map.at(testY * MAP_WIDTH + testX) == '#') {
                    hitWall = true;
                }
            }
            
            // Calculate distance to ceiling and floor
            int ceiling = (float)(SCREEN_HEIGHT / 2.0) - SCREEN_HEIGHT / ((float)distanceToWall);
            int floor = SCREEN_HEIGHT - ceiling;
            
            // Shader walls based on distance
            char wallShade = ' ';
            if (distanceToWall <= DEPTH / 4.0f)      wallShade = '#';
            else if (distanceToWall < DEPTH / 3.0f)  wallShade = 'X';
            else if (distanceToWall < DEPTH / 2.0f)  wallShade = ':';
            else if (distanceToWall < DEPTH)         wallShade = '.';
            else                                     wallShade = ' ';
            
            // Draw column
            for (int y = 0; y < SCREEN_HEIGHT; y++) {
                if (y < ceiling)
                    screen[y * SCREEN_WIDTH + x] = ' ';  // Sky
                else if (y > ceiling && y <= floor)
                    screen[y * SCREEN_WIDTH + x] = wallShade;  // Wall
                else {
                    // Floor shading
                    float b = 1.0f - (((float)y - SCREEN_HEIGHT / 2.0f) / ((float)SCREEN_HEIGHT / 2.0f));
                    char floorShade = ' ';
                    if (b < 0.25)      floorShade = '#';
                    else if (b < 0.5)  floorShade = 'x';
                    else if (b < 0.75) floorShade = '-';
                    else if (b < 0.9)  floorShade = '.';
                    else               floorShade = ' ';
                    screen[y * SCREEN_WIDTH + x] = floorShade;  // Floor
                }
            }
        }
        
        // Display map (minimap)
        for (int x = 0; x < MAP_WIDTH; x++)
            for (int y = 0; y < MAP_HEIGHT; y++)
                screen[(y + 1) * SCREEN_WIDTH + x] = map.c_str()[y * MAP_WIDTH + x];
        
        // Display player on minimap
        screen[(int)(playerY + 1) * SCREEN_WIDTH + (int)playerX] = 'P';
        
        // Display stats
        std::string stats = "X: " + std::to_string(playerX) + " Y: " + std::to_string(playerY) + " A: " + std::to_string(playerA);
        for (size_t i = 0; i < stats.size(); i++)
            screen[i] = stats[i];
        
        // Display controls
        std::string controls = " W:Forward S:Back A:Left D:Right J:Turn Left L:Turn Right Q:Quit";
        for (size_t i = 0; i < controls.size(); i++)
            screen[SCREEN_WIDTH * (SCREEN_HEIGHT - 1) + i] = controls[i];
        
        // Display frame
        std::cout << "\033[H";  // Move cursor to home position
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                std::cout << screen[y * SCREEN_WIDTH + x];
            }
            std::cout << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    return 0;
}