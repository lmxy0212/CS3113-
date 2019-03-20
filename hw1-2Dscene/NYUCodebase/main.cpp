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

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 590, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif
    
    //setup
    glViewport(0, 0, 960, 590);
    ShaderProgram program;
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    GLuint bakugoTexture = LoadTexture(RESOURCE_FOLDER"bakugou.png");
    GLuint todorokiTexture = LoadTexture(RESOURCE_FOLDER"todoroki.png");
    GLuint bombTexture = LoadTexture(RESOURCE_FOLDER"bomb.png");
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::ortho(-1.62f, 1.62f, -1.0f, 1.0f, -1.0f, 1.0f);
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);

    glUseProgram(program.programID);
    
    ShaderProgram program1;
    program1.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    program1.SetProjectionMatrix(projectionMatrix);
    program1.SetViewMatrix(viewMatrix);
    glUseProgram(program1.programID);
    program1.SetColor(0.0f,0.0f, 0.0f, 0.5f);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1.0f, 0.5f, 0.0f,1.0f);
    
    float lastFrameTicks = 0.0f;
    float angle = 0.0f;
    float translateDis = -0.4f;
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        angle += elapsed;
        translateDis += elapsed*0.5f;
        
        //drawing
        glClear(GL_COLOR_BUFFER_BIT);
       
        //draw triangle
        modelMatrix = glm::mat4(1.0f);
        program1.SetModelMatrix(modelMatrix);
        
        float vertices[] = {0.5f, -0.5f, 0.0f, 0.5f, -0.5f, -0.5f};
        glEnableVertexAttribArray(program1.positionAttribute);
        glVertexAttribPointer(program1.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisableVertexAttribArray(program1.positionAttribute);
        
        //        texture todoroki

        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(+1.15f, -0.3f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.7f, 0.7f, 0.0f));
        program.SetModelMatrix(modelMatrix);
        glBindTexture(GL_TEXTURE_2D,todorokiTexture);

        float vertices3[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,vertices3);
        glEnableVertexAttribArray(program.positionAttribute);

        float texCoords3[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute,2,GL_FLOAT,false,0,texCoords3);
        glEnableVertexAttribArray(program.texCoordAttribute);

        glDrawArrays(GL_TRIANGLES, 0,6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);

        //texture bomb
        modelMatrix = glm::mat4(1.0f);
        if(translateDis >= 2)
            translateDis = -1.0f;
        modelMatrix = glm::translate(modelMatrix, glm::vec3(translateDis,0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, -angle, glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.7f, 0.7f, 0.0f));
        program.SetModelMatrix(modelMatrix);
        glBindTexture(GL_TEXTURE_2D, bombTexture);

        float vertices2[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,vertices2);
        glEnableVertexAttribArray(program.positionAttribute);

        float texCoords2[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute,2,GL_FLOAT,false,0,texCoords2);
        glEnableVertexAttribArray(program.texCoordAttribute);

        glDrawArrays(GL_TRIANGLES, 0,6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);

        //texture bakugo

        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.15f, -0.3f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.7f, 0.7f, 0.0f));
        program.SetModelMatrix(modelMatrix);

        glBindTexture(GL_TEXTURE_2D, bakugoTexture);

        float vertices1[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,vertices1);
        glEnableVertexAttribArray(program.positionAttribute);

        float texCoords1[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute,2,GL_FLOAT,false,0,texCoords1);
        glEnableVertexAttribArray(program.texCoordAttribute);

        glDrawArrays(GL_TRIANGLES, 0,6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);

        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
