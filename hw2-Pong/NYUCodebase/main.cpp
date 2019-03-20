#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <math.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
using namespace std;
ShaderProgram program;

SDL_Window* displayWindow;

GLuint LoadTexture(const char* filePath) {
    int w, h, comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    
    if (image == NULL) {
        cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    stbi_image_free(image);
    return retTexture;
}

class P1 {
public:
    void draw(ShaderProgram &p) {
        
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, 0.0f));
//        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.3f, 0.3f, 0.3f));
        p.SetModelMatrix(modelMatrix);
        p.SetColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        float vertices[] = { -0.125, -0.25, 0.125, -0.25, 0.125, 0.25, -0.125, -0.25, 0.125, 0.25, -0.125, 0.25 };
        glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(p.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(p.positionAttribute);
    }
    float x = -1.45f;
    float y = 0.0f;
    float velocity_y = 3.0f;
    float half_width = 0.125f;
    float half_height = 0.25f;
};

class P2 {
public:
    void draw(ShaderProgram &p) {
        
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, 0.0f));
        p.SetModelMatrix(modelMatrix);
        p.SetColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        float vertices[] = { -0.125, -0.25, 0.125, -0.25, 0.125, 0.25, -0.125, -0.25, 0.125, 0.25, -0.125, 0.25 };
        glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(p.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(p.positionAttribute);
    }
    float x = 1.45f;
    float y = 0.0f;
    float velocity_y = 3.0f;
    float half_width = 0.125f;
    float half_height = 0.25f;
};

class Ball {
public:
    void draw(ShaderProgram &p) {
        
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, 0.0f));
        p.SetModelMatrix(modelMatrix);
        p.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        
        float vertices[] = {0.05f, 0.05f, -0.05f, 0.05f, -0.05f, -0.05f, -0.05f, -0.05f, 0.05f, -0.05f, 0.05f, 0.05f};
        glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(p.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(p.positionAttribute);
    }
    float x = 0.0f;
    float y = 0.0f;
    float velocity_x = 0.8f;
    float velocity_y = 0.4f;
    float direction_x = 1.0f;
    float direction_y = 1.0f;
    float half_width = 0.05f;
    float half_height = 0.05f;
    
};



void setUp(){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Welcome to Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 590, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 960, 590);
    program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::ortho(-1.62f, 1.62f, -1.0f, 1.0f, -1.0f, 1.0f);
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);

    glUseProgram(program.programID);
    program.SetColor(0.0f,0.0f, 0.0f, 1.0f);
    
    glEnable(GL_BLEND);
    glClearColor(1.0f, 0.5f, 0.0f,1.0f);
}

void processEvents(SDL_Event& event, bool& done, P1& left, P2& right, float& elapsed) {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
    }
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_W]) {
        left.y += left.velocity_y * elapsed;
    }
    else if (keys[SDL_SCANCODE_S]) {
        left.y -= left.velocity_y * elapsed;
    }
    else if (keys[SDL_SCANCODE_UP]) {
        right.y += right.velocity_y * elapsed;
    }
    else if (keys[SDL_SCANCODE_DOWN]) {
        right.y -= right.velocity_y * elapsed;
    }
}

void update(P1& left, P2& right, Ball& pong, bool& done, float& elapsed) {
    //update the ball
    pong.x += elapsed * pong.velocity_x * pong.direction_x;
    pong.y += elapsed * pong.velocity_y * pong.direction_y;
    
    if(left.y + left.half_height >= 1.0f){ //pattle hit the wall
        left.y = 1.0f - left.half_height;
    }else if(left.y - left.half_height <= -1.0f){
        left.y = -1.0f + left.half_height;
    }
    if(right.y + right.half_height >= 1.0f){
        right.y = 1.0f - right.half_height;
    }else if(right.y - right.half_height <= -1.0f){
        right.y = -1.0f + right.half_height;
    }
    //check who wins
    if (pong.x - pong.half_width >= 1.62f) {
        cout << "Player One Wins!\n";
        done = true;
    }else if (pong.x + pong.half_width <= -1.62f) {
        cout << "Player Two Wins!\n";
        done = true;
    }else if (abs(pong.y) + pong.half_height >= 1.0f) {
        //ball hit the wall
        pong.direction_y = -pong.direction_y;
        pong.x += elapsed * pong.velocity_x * pong.direction_x;
        pong.y += elapsed * pong.velocity_y * pong.direction_y;
    }else if (abs(left.y- pong.y) - (pong.half_height + left.half_height) < 0 && abs(left.x- pong.x) - (pong.half_width + left.half_width) < 0) {
        //ball hit the pattle
        pong.direction_x = -pong.direction_x;
        pong.x += elapsed * pong.velocity_x * pong.direction_x;
        pong.y += elapsed * pong.velocity_y * pong.direction_y;
    }else if (abs(right.y- pong.y) - (pong.half_height + right.half_height) < 0 && abs(right.x- pong.x) - (pong.half_width + right.half_width) < 0) {
        //ball hit the pattle
        pong.direction_x = -pong.direction_x;
        pong.x += elapsed * pong.velocity_x * pong.direction_x;
        pong.y += elapsed * pong.velocity_y * pong.direction_y;
    }
}


void render(P1& left, P2& right, Ball& pong) {
    glClear(GL_COLOR_BUFFER_BIT);
    left.draw(program);
    right.draw(program);
    pong.draw(program);
}


int main(int argc, char *argv[])
{
    setUp();
    P1 left;
    P2 right;
    Ball pong;
    float lastFrameTicks = 0.0f;
    SDL_Event event;
    bool done = false;
    while (!done) {
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        processEvents(event, done, left, right, elapsed);
        update(left, right, pong, done, elapsed);
        render(left, right, pong);
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
