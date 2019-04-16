//******************Platformer*******************
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
//#include "worldMap.hpp"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define FRICTION_X 0.2f
#define FRICTION_Y 0.1f
#define GRAVITY 2.0f
using namespace std;
//globals
ShaderProgram program;
glm::mat4 modelMatrix = glm::mat4(1.0f);
GLuint spriteTexture;
GLuint fontTexture;
SDL_Event event;
enum GameMode {STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER};
GameMode mode = STATE_MAIN_MENU;
vector<float> vertexData;
vector<float> texCoordData;
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
float accumulator = 0.0f;
int sprite_count_x = 16;
int sprite_count_y = 8;
float tileSize = .1;
float scale = .1;
float elapsed = 0.0f;
float lastFrameTicks = 0.0f;
bool done = false;
bool initial = true;

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

void DrawText(ShaderProgram &program, int fontTexture, string text, float x, float y, float size, float spacing) {
    float texture_size = 1.0 / 16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    glm::mat4 textModelMatrix = glm::mat4(1.0f);
    textModelMatrix = glm::translate(textModelMatrix, glm::vec3(x, y, 1.0f));
    for (size_t i = 0; i < text.size(); i++) {
        float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
        float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size + spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        });
    }
    glUseProgram(program.programID);
    program.SetModelMatrix(textModelMatrix);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glEnableVertexAttribArray(program.texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glDrawArrays(GL_TRIANGLES, 0, int(text.size()) * 6);
    
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}

class SheetSprite {
public:
    SheetSprite(){};
    SheetSprite(unsigned int input_textureID, float ind,float
                input_size){
        this -> textureID = input_textureID;
        this -> index = ind;
//        this -> width = input_width;
//        this -> height = input_height;
        this -> spriteWidth = 1.0/(float)sprite_count_x;
        this -> spriteHeight = 1.0/(float)sprite_count_y;
        this -> size = input_size;
    }
    void Draw(ShaderProgram &program);
    float size;
    unsigned int textureID;
    float width;
    float height;
    float index;
    float spriteWidth;
    float spriteHeight;
    glm::vec2 trueSize;
};

void SheetSprite::Draw(ShaderProgram &program) {
    float u = (float)(((int)index) % sprite_count_x) / (float) sprite_count_y;
    float v = (float)(((int)index) / sprite_count_x) / (float) sprite_count_y;
    
    float texCoords[] = {
        u, v+spriteHeight,
        u+spriteWidth, v,
        u, v,
        u+spriteWidth, v,
        u, v+spriteHeight,
        u+spriteWidth, v+spriteHeight
    };
    float vertices[] = {-0.5f*size, -0.5f*size, 0.5f*size, 0.5f*size, -0.5f*size, 0.5f*size, 0.5f*size, 0.5f*size,  -0.5f*size,
        -0.5f*size, 0.5f*size, -0.5f*size};
    // draw this data
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}

SheetSprite playertexture;
SheetSprite keytexture;

float lerp(float v0, float v1, float t) {
    return (1.0f - t) * v0 + t * v1;
}

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
    *gridX = (int)(worldX / tileSize);
    *gridY = (int)(worldY / -tileSize);
}

class Entity{
public:
    Entity(){}
    Entity(const string& Type,SheetSprite& input_sprite, float pos_x, float pos_y,float v_x, float v_y,float scale){
        this->position = glm::vec2(pos_x,pos_y);
        this->velocity = glm::vec2(v_x,v_y);
        this->type = Type;
//        this->i = ind;
        this->sprite = input_sprite;
        this->scale = scale;
        
    }
    void draw(ShaderProgram &p){
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, position.y, 1.0f));
        program.SetModelMatrix(modelMatrix);//move
        sprite.Draw(p);
    }
    void update(float elapsed) {
//        velocity.x = lerp(velocity.x, 0.0f, elapsed * FRICTION_X);
//        velocity.y = lerp(velocity.y, 0.0f, elapsed * FRICTION_Y);
        position.x += velocity.x * elapsed;
        position.y += velocity.y * elapsed - GRAVITY*elapsed;
    }
    bool collision_check(Entity &e){
        if (position.x + 0.5f * size.x < e.position.x - 0.5f * e.size.x ||
            position.x - 0.5f * size.x > e.position.x + 0.5f * e.size.x ||
            position.y + 0.5f * size.y < e.position.y - 0.5f * e.size.y ||
            position.y - 0.5f * size.y > e.position.y + 0.5f * e.size.y) {
            return false;
        }
        else {
            return true;
        }
        
    }
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec2 size;
    int i;
    SheetSprite sprite;
    string type;
    float scale;
//    SheetSprite sprite;
    float health;
};

struct GameState {
    Entity key;
    Entity player;
};
GameState state;
struct worldMapEntity {
    string type;
    float x;
    float y;
};

void placeEntity(string& type, float position_x, float position_y, SheetSprite& mySprite) {
//    cout<<"placeEntity():";
//    cout << type << position_x << position_y << "\n";
    if (type == "player") {
        cout <<"player!!";
        state.player = Entity(type,mySprite,position_x,position_y,0.0f, 0.0f, 0.0f);
    }
    else if (type == "key") {
        state.key = Entity(type,mySprite,position_x, position_y, 0.0f, 0.0f, 0.0f);
    }
}


class worldMap {
public:
    worldMap();
    void Load(const string fileName);
//    void placeEntity(string type, int placeX, int placeY);

    int mapWidth;
    int mapHeight;
    unsigned int **mapData;
    vector<worldMapEntity> entities;
    
private:
    bool ReadHeader(ifstream &stream);
    bool ReadLayerData(ifstream &stream);
    bool ReadEntityData(ifstream &stream);

    
};

worldMap:: worldMap() {
    mapData = nullptr;
    mapWidth = -1;
    mapHeight = -1;
}
worldMap map;
bool worldMap::ReadHeader(ifstream &stream) {
    string line;
    mapWidth = -1;
    mapHeight = -1;
    while( getline(stream, line)) {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "width") {
            mapWidth =  atoi(value.c_str());
        } else if(key == "height"){
            mapHeight =  atoi(value.c_str());
        }
    }
    if(mapWidth == -1 || mapHeight == -1) {
        return false;
    } else {
        mapData = new unsigned int*[mapHeight];
        for(int i = 0; i < mapHeight; ++i) {
            mapData[i] = new unsigned int[mapWidth];
        }
        return true;
    }
}

bool worldMap::ReadLayerData(ifstream &stream) {
    string line;
    while(getline(stream, line)) {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "data") {
            for(int y=0; y < mapHeight; y++) {
                getline(stream, line);
                istringstream lineStream(line);
                string tile;
                for(int x=0; x < mapWidth; x++) {
                    getline(lineStream, tile, ',');
                    unsigned int val = atoi(tile.c_str());
                    if(val > 0) {
                        mapData[y][x] = val-1;
                    } else {
                        mapData[y][x] = 0;
                    }
                }
            }
        }
    }
    return true;
}

bool worldMap::ReadEntityData(ifstream &stream) {
    string line;
    string type;
    while(getline(stream, line)) {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "type") {
            type = value;
        } else if(key == "location") {
            istringstream lineStream(value);
            string xPosition, yPosition;
            getline(lineStream, xPosition, ',');
            getline(lineStream, yPosition, ',');
            float placeX = (atoi(xPosition.c_str())*tileSize);
            float placeY = (atoi(yPosition.c_str())*-tileSize);
            
            cout << type;
//              placeEntity(string& type, float position_x, float position_y,SheetSprite& mySprite)
            if(type == "player"){
                cout << type << placeX << placeY << "\n";
                placeEntity(type, placeX, placeY,playertexture);
            }else if(type == "key"){
                cout << type << placeX << placeY << "\n";
                placeEntity(type, placeX, placeY,keytexture);
            }
            

        }
    }
    return true;
}

void worldMap::Load(const string fileName) {
    ifstream infile(fileName);
    if(infile.fail()) {
        assert(false); // unable to open file
    }
    string line;
    while (getline(infile, line)) {
        if(line == "[header]") {
            if(!ReadHeader(infile)) {
                assert(false); // invalid file data
            }
        } else if(line == "[layer]") {
            ReadLayerData(infile);
        } else if(line == "[Entities]" and initial == true) {
            ReadEntityData(infile);
            ReadEntityData(infile);
            cout << initial;
            initial = false;
            //only read form the txt file in the begining of the game
        }
    }
}



void drawMap(){
//    cout << map.mapData;
    for(int y=0; y < map.mapHeight; y++) {
        for(int x=0; x < map.mapWidth; x++) {
            
            if(map.mapData[y][x] != 0 && map.mapData[y][x] != 12){
                float u = (float)(((int)map.mapData[y][x]) % sprite_count_x) / (float) sprite_count_x;
                float v = (float)(((int)map.mapData[y][x]) / sprite_count_x) / (float) sprite_count_y;
                float spriteWidth = 1.0f/(float)sprite_count_x;
                float spriteHeight = 1.0f/(float)sprite_count_y;
                vertexData.insert(vertexData.end(), {
                    tileSize * x, -tileSize * y,
                    tileSize * x, (-tileSize * y)-tileSize,
                    (tileSize * x)+tileSize, (-tileSize * y)-tileSize,
                    tileSize * x, -tileSize * y,
                    (tileSize * x)+tileSize, (-tileSize * y)-tileSize,
                    (tileSize * x)+tileSize, -tileSize * y
                });
                texCoordData.insert(texCoordData.end(), {
                    u, v,
                    u, v+(spriteHeight),
                    u+spriteWidth, v+(spriteHeight),
                    u, v,
                    u+spriteWidth, v+(spriteHeight),
                    u+spriteWidth, v
                });
            }
        }
    }
    glUseProgram(program.programID);
    glm::mat4 mapModelMatrix = glm::mat4(1.0);
    mapModelMatrix = glm::scale(mapModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
    program.SetModelMatrix(mapModelMatrix);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, spriteTexture);
    glDrawArrays(GL_TRIANGLES, 0, int(vertexData.size()/2));
    
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}

void setUp(){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Welcome to platformer >w<", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 590, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(0, 0, 960, 590);
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::ortho(-1.62f, 1.62f, -1.0f, 1.0f, -1.0f, 1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetModelMatrix(modelMatrix);
    
    fontTexture = LoadTexture(RESOURCE_FOLDER"pixel_font.png");
    spriteTexture = LoadTexture(RESOURCE_FOLDER"sprites.png");
    playertexture = SheetSprite(spriteTexture,80,tileSize);
    keytexture = SheetSprite(spriteTexture,83,tileSize);
    map.Load(RESOURCE_FOLDER"map3.txt");
    glUseProgram(program.programID);
    program.SetColor(1.0f, 0.5f, 0.0f,1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.2f, 0.5f, 0.7f, 1.0f);
    
}

void processEvents() {
    switch (mode) {
        case STATE_MAIN_MENU:
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                    done = true;
                }else if(event.type == SDL_KEYDOWN){
                    if(event.key.keysym.scancode == SDL_SCANCODE_RETURN){
                        mode = STATE_GAME_LEVEL;
                    }
                }
            }
            break;
        case STATE_GAME_LEVEL:
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                    done = true;
                }
                break;
            case STATE_GAME_OVER:
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                        done = true;
                    }
                }
                break;
            }
    }
}

void update(float elapsed) {
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    modelMatrix = glm::mat4(1.0f);
    
    switch (mode) {
        case STATE_MAIN_MENU:
            break;
        case STATE_GAME_LEVEL:
            if(keys[SDL_SCANCODE_LEFT] ){
                state.player.velocity.x = -1.5f;
            }else if(keys[SDL_SCANCODE_RIGHT]){
                state.player.velocity.x = 1.5f;
            }else{
                state.player.velocity.x = 0.0f;
            }
            state.player.update(elapsed);
            break;
        case STATE_GAME_OVER:
            break;
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    switch (mode) {
        case STATE_MAIN_MENU:{
            DrawText(program, fontTexture, "Welcome to platformer demo QwQ", -1.1f,0.0f,0.07f, 0.01f);
            DrawText(program, fontTexture, "HINT: Press left/Right/Up/Down to move", -1.4f,-0.2f, 0.05f, 0.01f);
            DrawText(program, fontTexture, "Press Enter To Start", -1.1f,-0.4f, 0.05f, 0.01f);
            break;
        }
        case STATE_GAME_LEVEL:{
            drawMap();
            state.player.draw(program);
            state.key.draw(program);
            //re-center
            glm::mat4 viewMatrix = glm::mat4(1.0f);
            viewMatrix = glm::translate(viewMatrix, glm::vec3(-state.player.position.x, -state.player.position.y, 1.0f));
            program.SetViewMatrix(viewMatrix);

//            DrawSpriteSheetSprite(program, 28, sprite_count_x, sprite_count_y,0.3f);
//            if(initial == true){
//                initial = false;
//            }else{
//                state.player.draw(program);
//                state.enemy.draw(program);
//            }
//            map.ReadEntityData();
            break;
        }
        case STATE_GAME_OVER:{
//            if(state.score == 7){
//                DrawText(program, fontTexture, "Awwwww! You did it >w<", -1.12f,0.0f,0.05f, 0.01f);
//            }
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    setUp();
    //   SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size);

    while (!done) {
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        elapsed += accumulator;
        if(elapsed < FIXED_TIMESTEP) {
            accumulator = elapsed;
            continue;
            
        }
        while(elapsed >= FIXED_TIMESTEP) {
            update(FIXED_TIMESTEP);
            elapsed -= FIXED_TIMESTEP;
        }
        accumulator = elapsed;
        
        processEvents();
        render();
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}