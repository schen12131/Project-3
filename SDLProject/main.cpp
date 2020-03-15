#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define FIXED_TIMESTEP 0.0166666f

#include "Entity.h"
#define PLATFORM_COUNT 35

struct GameState {
    Entity *player;
    Entity *platforms;
    Entity *successful;
    Entity *failed;
    Entity *target;
};

GameState state;

SDL_Window* displayWindow;
bool gameIsRunning = true;
bool finished = false;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;


GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);
    
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    return textureID;
}


void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Textured!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 480);
    
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    
    glUseProgram(program.programID);
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    
    // Initialize Game Objects
    
    state.successful = new Entity[15];
    state.failed = new Entity[14];

    float counter = -3.5f;
    
    GLuint fontTextureID = LoadTexture("font1.png");
    
    for (int i = 0; i < 15; i++){
        state.successful[i].textureID = fontTextureID;
        state.successful[i].position = glm::vec3(counter, 0, 0);
        state.successful[i].animCols = 16;
        state.successful[i].animRows = 16;
        state.successful[i].height = 0.3f;
        state.successful[i].width = 0.3f;
        state.successful[i].success = new int[15] {77, 73, 83, 83, 73, 79, 78, 9, 83, 85, 67, 67, 69, 83, 83 };
        state.successful[i].index = i;
        state.successful[i].isActive = false;
        
        if (i != 14){
            state.failed[i].textureID = fontTextureID;
            state.failed[i].position = glm::vec3(counter, 0, 0);
            state.failed[i].animCols = 16;
            state.failed[i].animRows = 16;
            state.failed[i].height = 0.3f;
            state.failed[i].width = 0.3f;
            state.failed[i].fail = new int[15] {77, 73, 83, 83, 73, 79, 78, 9, 70, 65, 73, 76, 69, 68 };
            state.failed[i].index = i;
            state.failed[i].isActive = false;
            
        }
        
        counter += 0.5f;
        
    }
    
    // Initialize Player
    state.player = new Entity();
    state.player->position = glm::vec3(0, 3, 0);
    state.player->movement = glm::vec3(0);
    state.player->acceleration = glm::vec3(0, -0.25f, 0);
    state.player->speed = 1.5f;
    state.player->textureID = LoadTexture("rocket.png");
    
    
    state.player->height = 0.8f;
    state.player->width = 0.8f;
    
    
    
    state.platforms = new Entity[PLATFORM_COUNT];
    GLuint platformTextureID = LoadTexture("platformPack_tile016.png");
    
    state.target = new Entity();
    GLuint targetTextureID = LoadTexture("platformPack_tile042.png");
    
    state.platforms[0].textureID = platformTextureID;
    state.platforms[0].position = glm::vec3(-1, -3.5f, 0);
    
    state.platforms[1].textureID = platformTextureID;
    state.platforms[1].position = glm::vec3(0, -3.5f, 0);
    
    state.platforms[2].textureID = platformTextureID;
    state.platforms[2].position = glm::vec3(1, -3.5f, 0);
    
    state.platforms[3].textureID = platformTextureID;
    state.platforms[3].position = glm::vec3(-3, -3.5f, 0);
    
    state.platforms[4].textureID = platformTextureID;
    state.platforms[4].position = glm::vec3(2, -3.5f, 0);
    
    state.target->textureID = targetTextureID;
    state.target->position = glm::vec3(-2, -3.5f, 0);
    
    state.platforms[5].textureID = platformTextureID;
    state.platforms[5].position = glm::vec3(-4, -3.5f, 0);
    
    state.platforms[6].textureID = platformTextureID;
    state.platforms[6].position = glm::vec3(-5, -3.5f, 0);
    
    state.platforms[7].textureID = platformTextureID;
    state.platforms[7].position = glm::vec3(3, -3.5f, 0);
    
    state.platforms[8].textureID = platformTextureID;
    state.platforms[8].position = glm::vec3(4, -3.5f, 0);
    
    state.platforms[9].textureID = platformTextureID;
    state.platforms[9].position = glm::vec3(5, -3.5f, 0);
    
    state.platforms[10].textureID = platformTextureID;
    state.platforms[10].position = glm::vec3(5, -2.5f, 0);
    
    state.platforms[11].textureID = platformTextureID;
    state.platforms[11].position = glm::vec3(5, -1.5f, 0);
    
    state.platforms[12].textureID = platformTextureID;
    state.platforms[12].position = glm::vec3(5, -0.5f, 0);
    
    state.platforms[13].textureID = platformTextureID;
    state.platforms[13].position = glm::vec3(5, 0.5f, 0);
    
    state.platforms[14].textureID = platformTextureID;
    state.platforms[14].position = glm::vec3(5, 1.5f, 0);
    
    state.platforms[15].textureID = platformTextureID;
    state.platforms[15].position = glm::vec3(5, 2.5f, 0);
    
    state.platforms[16].textureID = platformTextureID;
    state.platforms[16].position = glm::vec3(5, 2.5f, 0);
    
    state.platforms[17].textureID = platformTextureID;
    state.platforms[17].position = glm::vec3(5, 3.5f, 0);
    
    state.platforms[18].textureID = platformTextureID;
    state.platforms[18].position = glm::vec3(-5, -2.5f, 0);
    
    state.platforms[19].textureID = platformTextureID;
    state.platforms[19].position = glm::vec3(-5, -1.5f, 0);
    
    state.platforms[20].textureID = platformTextureID;
    state.platforms[20].position = glm::vec3(-5, -0.5f, 0);

    state.platforms[21].textureID = platformTextureID;
    state.platforms[21].position = glm::vec3(-5, 0.5f, 0);
    
    state.platforms[22].textureID = platformTextureID;
    state.platforms[22].position = glm::vec3(-5, 1.5f, 0);
    
    state.platforms[23].textureID = platformTextureID;
    state.platforms[23].position = glm::vec3(-5, 2.5f, 0);
    
    state.platforms[24].textureID = platformTextureID;
    state.platforms[24].position = glm::vec3(-5, 2.5f, 0);
    
    state.platforms[25].textureID = platformTextureID;
    state.platforms[25].position = glm::vec3(-5, 3.5f, 0);
    
    state.platforms[26].textureID = platformTextureID;
    state.platforms[26].position = glm::vec3(-4, 2.5f, 0);
    
    state.platforms[27].textureID = platformTextureID;
    state.platforms[27].position = glm::vec3(-3, 2.5f, 0);
    
    state.platforms[28].textureID = platformTextureID;
    state.platforms[28].position = glm::vec3(-2, 2.5f, 0);
    
    state.platforms[29].textureID = platformTextureID;
    state.platforms[29].position = glm::vec3(-1, 2.5f, 0);
    
    state.platforms[30].textureID = platformTextureID;
    state.platforms[30].position = glm::vec3(4, -1.0f, 0);
    
    state.platforms[31].textureID = platformTextureID;
    state.platforms[31].position = glm::vec3(3, -1.0f, 0);
    
    state.platforms[32].textureID = platformTextureID;
    state.platforms[32].position = glm::vec3(2, -1.0f, 0);
    
    state.platforms[33].textureID = platformTextureID;
    state.platforms[33].position = glm::vec3(1, -1.0f, 0);
    
    state.platforms[34].textureID = platformTextureID;
    state.platforms[34].position = glm::vec3(0, -1.0f, 0);
    
    
    for (int i = 0; i < PLATFORM_COUNT; i++){
        state.platforms[i].Update(0, NULL, 0);
    }
    
    for (int j = 0; j < 15; j++){
        state.successful[j].Update(0, NULL, 0);
    }
    
    for (int k = 0; k < 14; k++){
        state.failed[k].Update(0, NULL, 0);
    }
    state.target->Update(0, NULL, 0);
}

void ProcessInput() {
    
    state.player->movement = glm::vec3(0);
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        // Move the player left
                        break;
                        
                    case SDLK_RIGHT:
                        // Move the player right
                        break;
                        
                    case SDLK_SPACE:
                        // Some sort of action
                        break;
                }
                break; // SDL_KEYDOWN
        }
    }
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    if (keys[SDL_SCANCODE_LEFT]) {
        state.player->movement.x = -1.0f;
        state.player->acceleration = glm::vec3(-0.25f, -0.25f, 0);
    }
    else if (keys[SDL_SCANCODE_RIGHT]) {
        state.player->movement.x = 1.0f;
        state.player->acceleration = glm::vec3(0.25f, -0.25f, 0);
    }
    
    if (glm::length(state.player->movement) > 1.0f) {
        state.player->movement = glm::normalize(state.player->movement);
        state.player->acceleration = glm::normalize(state.player->acceleration);
    }
    
}

float lastTicks = 0;
float accumulator = 0.0f;
void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;
    deltaTime += accumulator;
    
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return; }
    while (deltaTime >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        if (finished == false){
            state.player->Update(FIXED_TIMESTEP, state.target, 1);
            if (state.player->collidedBottom ){
                std::cout <<"success"<< std::endl;
                for (int j = 0; j < 15; j++){
                    state.successful[j].isActive = true;
                    state.successful[j].Update(0, NULL, 0);
                }
                finished = true;
                state.player->isActive = false;
                return;
            }
            state.player->Update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT);
            if (state.player->collidedBottom||state.player->collidedRight || state.player->collidedLeft){
                std::cout <<"failed"<< std::endl;
                for (int j = 0; j < 14; j++){
                    state.failed[j].isActive = true;
                    state.failed[j].Update(0, NULL, 0);
                }
                finished = true;
                state.player->isActive = false;
                return;
            }
        }
        deltaTime -= FIXED_TIMESTEP;
        
    }
    accumulator = deltaTime;
}

void Render() {
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    for (int i = 0; i < PLATFORM_COUNT; i++){
        state.platforms[i].Render(&program);
    }

    for(int j = 0; j < 15; j++){
        state.successful[j].Render(&program);
    }
    
    for(int k = 0; k < 14; k++){
        state.failed[k].Render(&program);
    }
    
    state.player->Render(&program);
    state.target->Render(&program);


    SDL_GL_SwapWindow(displayWindow);
}


void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();
    
    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }
    
    Shutdown();
    return 0;
}
