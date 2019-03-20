#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <math.h>
#include <vector>
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

//class vec3{
//public:
//    vec3(){}
//    vec3(float x_input, float y_input, float z_input){
//        x = x_input;
//        y = y_input;
//        z = z_input;
//    }
//    float x;
//    float y;
//    float z;
//};

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

void DrawText(ShaderProgram* program, int fontTexture, const string& text, float size, float spacing, float start_x, float start_y) {
    float textureSize = 1 / 16.0f;
    vector<float> vertexData;
    vector<float> texData;
    for (size_t i = 0; i < text.size(); i++) {
        int spriteIndex = (int)text[i];
        float texture_X = (float)(spriteIndex % 16) / 16.0f;
        float texture_Y = (float)(spriteIndex / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size + spacing) * i + (-0.5f * size)), 0.5f * size,
            ((size + spacing) * i + (-0.5f * size)), -0.5f * size,
            ((size + spacing) * i + (0.5f * size)), 0.5f * size,
            ((size + spacing) * i + (0.5f * size)), -0.5f * size,
            ((size + spacing) * i + (0.5f * size)), 0.5f * size,
            ((size + spacing) * i + (-0.5f * size)), -0.5f * size });
        texData.insert(texData.end(), {
            texture_X, texture_Y,
            texture_X, texture_Y + textureSize,
            texture_X + textureSize, texture_Y,
            texture_X + textureSize, texture_Y + textureSize,
            texture_X + textureSize, texture_Y,
            texture_X, texture_Y + textureSize });
    }
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(start_x, start_y, 0.0f));
    program->SetModelMatrix(modelMatrix);
    program->SetColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, int(text.size()) * 6);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

class SheetSprite {
public:
    SheetSprite();
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size);
    
    void Draw(ShaderProgram &program);
    
    float size;
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
};

void SheetSprite::Draw(ShaderProgram &program) {
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLfloat texCoords[] = {
        u, v+height,
        u+width, v,
        u, v,
        u+width, v,
        u, v+height,
        u+width, v+height
    };
    float aspect = width / height;
    float vertices[] = {
        -0.5f * size * aspect, -0.5f * size,
        0.5f * size * aspect, 0.5f * size,
        -0.5f * size * aspect, 0.5f * size,
        0.5f * size * aspect, 0.5f * size,
        -0.5f * size * aspect, -0.5f * size ,
        0.5f * size * aspect, -0.5f * size
    };
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}

class Entity{
public:
    
    Entity(){}
    Entity(SheetSprite& mySprite,const string& Type, bool alive, float x, float y, float z, float displ_x, float displ_y, float displ_z, float v_x, float v_y, float v_z, float size_x, float size_y, float size_z) {
        position = glm::vec3(x,y,z);
        velocity = glm::vec3(v_x,v_y,v_z);
        size = glm::vec3(size_x,size_y,size_z);
        type = Type;
        
    }
    void Draw(ShaderProgram &p){
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, position.y, 0.0f));
        p.SetModelMatrix(modelMatrix);
        p.SetColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        float vertices[] = { -0.125, -0.25, 0.125, -0.25, 0.125, 0.25, -0.125, -0.25, 0.125, 0.25, -0.125, 0.25 };
        glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(p.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(p.positionAttribute);
    };
    void update(float elapsed) {
        if (type == "bullet") {
            timeAlive += elapsed;
            displacement.y += elapsed * velocity.y;
        }
        else if (type == "enemy"){
            if (displacement.x >= 1.5f) {
                velocity.x = -2.0f;
                displacement.x += elapsed * velocity.x;
            }
            else if (displacement.x <= -1.5f) {
                velocity.x = 2.0f;
                displacement.x += elapsed * velocity.x;
            }
            else {
                displacement.x += elapsed * velocity.x;
            }
        }
        else {
            displacement.x += elapsed * velocity.x;
        }
    }
    
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 size;
    string type;
//    float rotation;3090
    
    
    SheetSprite sprite;
    
    float health;
    
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

void processEvents(SDL_Event& event, bool& done) {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
    }
    
}

void update() {
    
}


void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
}


int main(int argc, char *argv[])
{
    setUp();
    
    float lastFrameTicks = 0.0f;
    SDL_Event event;
    bool done = false;
    while (!done) {
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        processEvents(event, done);
        update();
        render();
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
