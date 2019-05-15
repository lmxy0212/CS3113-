//******************Final Project*******************
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
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

#define FRICTION_X 0.2f
#define FRICTION_Y 0.5f
//#define GRAVITY 1.0f
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
using namespace std;
//globals
Mix_Music *bgm;
Mix_Chunk *footsteps;
Mix_Chunk *collect;
ShaderProgram program;
glm::mat4 modelMatrix = glm::mat4(1.0f);
GLuint spriteTexture;
GLuint fontTexture;
GLuint bakugoSprite;
GLuint todorokiSprite;
GLuint starSprite;
GLuint GGtexture;
GLuint GoodjobTexture;
GLint mainTexture;
GLint keySprite;
GLint exitTexture;
SDL_Event event;
enum GameMode {STATE_MAIN_MENU,STATE_INSTRUCTION, STATE_EXIT,STATE_GAME_LEVEL_ONE, STATE_GAME_LEVEL_TWO, STATE_GAME_LEVEL_THREE, STATE_GAME_OVER};
GameMode mode = STATE_MAIN_MENU;

float playerAnimate = 0.1f;
float enemyAnimate = 0.1f;
float mainAnimate = 0.1f;
float GRAVITY = 1.0f;
int playerID = 1;
int enemyID = 1;
float accumulator = 0.0f;
//int sprite_count_x = 16;
//int sprite_count_y = 8;
float tileSize = 0.15f;
float scale = 1.5f;
float lastFrameTicks = 0.0f;
bool done = false;
bool initial = true;
bool playerCollideBottom(string type);
bool playerCollideTop(string type);
bool playerCollideLeft(string type);
bool playerCollideRight(string type);
vector<int> solidInd = {12,33,7,17,34,19,35,18};

SDL_Window* displayWindow;
bool win = false;
bool lose = false;
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
    vector<float> vertexData;
    vector<float> texCoordData;
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
                input_size,int count_x,int count_y){
        this -> textureID = input_textureID;
        this -> index = ind;
        this -> spriteWidth = 1.0/(float)count_x;
        this -> spriteHeight = 1.0/(float)count_y;
        this -> size = input_size;
        this -> sprite_count_x = count_x;
        this -> sprite_count_y = count_y;
    }
    void Draw(ShaderProgram &program);
    float size;
    unsigned int textureID;
    float width;
    float height;
    float index;
    float spriteWidth;
    float spriteHeight;
    int sprite_count_x;
    int sprite_count_y;
    float u;
    float v;
};

void SheetSprite::Draw(ShaderProgram &program) {
    u = (float)(((int)index) % sprite_count_x) / (float) sprite_count_x;
    v = (float)(((int)index) / sprite_count_x) / (float) sprite_count_y;
//    cout<< u << " " << v << endl;
    float texCoords[] = {
        u, v+spriteHeight,
        u+spriteWidth, v,
        u, v,
        u+spriteWidth, v,
        u, v+spriteHeight,
        u+spriteWidth, v+spriteHeight
    };
//    float aspect = width / height;
//    float vertices[] = {
//        -0.5f * size * aspect, -0.5f * size,
//        0.5f * size * aspect, 0.5f * size,
//        -0.5f * size * aspect, 0.5f * size,
//        0.5f * size * aspect, 0.5f * size,
//        -0.5f * size * aspect, -0.5f * size ,
//        0.5f * size * aspect, -0.5f * size};
    float vertices[] = {-0.5f*size, -0.5f*size, 0.5f*size, 0.5f*size, -0.5f*size, 0.5f*size, 0.5f*size, 0.5f*size,  -0.5f*size,
        -0.5f*size, 0.5f*size, -0.5f*size};
    // draw this data
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}

SheetSprite playertexture;
SheetSprite keytexture;
SheetSprite startexture;
SheetSprite enemytexture;
SheetSprite mainSprite;

float lerp(float v0, float v1, float t) {
    return (1.0f - t) * v0 + t * v1;
}
float easeInOut(float from, float to, float time) {
    float tVal;
    if(time > 0.5) {
        float oneMinusT = 1.0f-((0.5f-time)*-2.0f);
        tVal =  1.0f - ((oneMinusT * oneMinusT * oneMinusT * oneMinusT *
                         oneMinusT) * 0.5f);
    } else {
        time *= 2.0;
        tVal = (time*time*time*time*time)/2.0;
    }
    return (1.0f-tVal)*from + tVal*to;
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
        this->sprite = input_sprite;
        this->jump = false;
        this->size = glm::vec2(tileSize * scale,tileSize * scale);
        this->acceleration = glm::vec2(0.0f,0.0f);
        this->collidedBot = false;
    }
    void draw(ShaderProgram &p){
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, position.y, 1.0f));
        program.SetModelMatrix(modelMatrix);//move
        sprite.Draw(p);
        if(type == "player"){
            DrawText(program, fontTexture, "p1: "+to_string(score), position.x-size.x,position.y+size.y,0.07f, 0.01f);
        }else if(type == "enemy"){
            DrawText(program, fontTexture, "p2: "+to_string(score), position.x-size.x,position.y+size.y,0.07f, 0.01f);
        }
    }
    void update(float elapsed) {
        velocity.x = lerp(velocity.x, 0.0f, elapsed * FRICTION_X);
        velocity.y = lerp(velocity.y, 0.0f, elapsed * FRICTION_Y);
        velocity.x += acceleration.x * elapsed;
        velocity.y += (acceleration.y - GRAVITY) * elapsed;
        position.x += elapsed * velocity.x;
        playerCollideLeft(type);
        playerCollideRight(type);
        position.y += elapsed * velocity.y;
        playerCollideBottom(type);
        playerCollideTop(type);
        
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
    bool colide_right(Entity e){
        if(position.x + 0.5f * size.x > e.position.x - 0.5f * e.size.x){return true;}
        else{return false;}
    }
    bool colide_left(Entity e){
        if(position.x - 0.5f * size.x < e.position.x + 0.5f * e.size.x){return true;}
        else{return false;}
    }
    bool colide_bot(Entity e){
        if(position.y - 0.5f * size.y < e.position.y + 0.5f * e.size.y){return true;}
        else{return false;}
    }
    bool colide_top(Entity e){
        if(position.y + 0.5f * size.y < e.position.y - 0.5f * e.size.y){return true;}
        else{return false;}
    }
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec2 acceleration;
    glm::vec2 size;
    int i;
    SheetSprite sprite;
    string type;
    float scale;
    int score = 0;
    bool jump;
    int doubleJump = 2;
    bool collidedBot;
};

struct GameState {
    Entity key;
    Entity player;
//    vector<Entity> bullets;
    Entity enemy;
//    int player_score = 0;
//    int enemy_score = 0;
//    vector<Entity> enemies;
    vector<Entity> stars;
};
GameState state;
//GameState state1;
GameState state2;
GameState state3;

void set_state(GameState newstate){
    state.key = newstate.key;
    state.player = newstate.player;
    state.enemy = newstate.enemy;
    state.stars = newstate.stars;
}

void animatePlayer(){
    if(playerID == 1){
        playerID = 0;
        state.player.sprite.index = playerID;
    }
    else{
        playerID = 1;
        state.player.sprite.index = playerID;
    }
}
void animatenemy(){
    if(enemyID == 1){
        enemyID = 0;
        state.enemy.sprite.index = enemyID;
    }
    else{
        enemyID = 1;
        state.enemy.sprite.index = enemyID;
    }
}

struct worldMapEntity {
    string type;
    float x;
    float y;
};

void placeEntity(string& type, float position_x, float position_y, SheetSprite& mySprite) {
//    cout<<"placeEntity():";
//    cout << type << position_x << position_y << "\n";
    if (type == "player") {
        cout <<"player!!"<<endl;
        state.player = Entity(type,mySprite,position_x,position_y,0.0f, 0.0f, scale);
    }else if (type == "key") {
        state.key = Entity(type,mySprite,position_x, position_y, 0.0f, 0.0f, 1.0f);
    }else if(type == "star"){
        cout << "star";
        state.stars.push_back(Entity(type,mySprite,position_x,position_y,0.0f, 0.0f, 1.0f));
    }else if(type == "enemy"){
        cout << "enemy";
        state.enemy = Entity(type,mySprite,position_x,position_y,0.0f, 0.0f, scale);
//        state.enemies.push_back(Entity(type,mySprite,position_x,position_y,0.0f, 0.0f, 1.0f));
    }
}


class worldMap {
public:
    worldMap();
    void Load(const string fileName);
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
//worldMap map2;
//worldMap map3;

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
            
//            cout << type;
//              placeEntity(string& type, float position_x, float position_y,SheetSprite& mySprite)
            if(type == "player"){
                cout << type << placeX << placeY << "\n";
                placeEntity(type, placeX, placeY,playertexture);
            }else if(type == "key"){
                cout << type << placeX << placeY << "\n";
                placeEntity(type, placeX, placeY,keytexture);
            }else if(type == "star"){
                cout << type << placeX << placeY << "\n";
                placeEntity(type, placeX, placeY,startexture);
            }else if(type == "enemy"){
                cout << type << placeX << placeY << "\n";
                placeEntity(type, placeX, placeY,enemytexture);
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
            for(int i = 0; i < 13; i ++){
                ReadEntityData(infile);
            }
            cout << initial;
            initial = false;
        }
    }
}

bool playerCollideBottom(string type){
    int gridX, gridY;
    if(type == "player"){
         worldToTileCoordinates(state.player.position.x,state.player.position.y-0.5f* state.player.size.y,&gridX,&gridY);
        if(gridX < map.mapWidth && abs(gridY) < map.mapHeight){
            if(map.mapData[gridY][gridX] != 0 && map.mapData[gridY][gridX]<90){
                state.player.position.y += fabs((-tileSize * gridY) -(state.player.position.y - state.player.size.y*0.5f))+0.001f;
                state.player.collidedBot = true;
                state.player.doubleJump = 2; //if collide bottom -> reset double jump
                if(!state.player.jump){
                    state.player.velocity.y = 0.0f;
                }
                else{
                    state.player.jump = false;
                    //                state.player.doubleJump = 0;
                }
                return true;
                
            }
            
        }
//        if(state.player.collision_check(state.enemy) && state.player.colide_bot(state.enemy)){
//            state.player.position.y += fabs( (state.enemy.position.y + state.enemy.size.y*0.5f)-(state.player.position.y - state.player.size.y*0.5f))+0.001f;
//        }
    }else if (type == "enemy"){
         worldToTileCoordinates(state.enemy.position.x,state.enemy.position.y-0.5f* state.enemy.size.y,&gridX,&gridY);
        if(gridX < map.mapWidth && abs(gridY) < map.mapHeight){
            if(map.mapData[gridY][gridX] != 0 && map.mapData[gridY][gridX]<90){
                state.enemy.position.y += fabs((-tileSize * gridY) -(state.enemy.position.y - state.enemy.size.y*0.5f))+0.001f;
                state.enemy.collidedBot = true;
                state.enemy.doubleJump = 2; //if collide bottom -> reset double jump
                if(!state.enemy.jump){
                    state.enemy.velocity.y = 0.0f;
                }
                else{
                    state.enemy.jump = false;
                    //                state.player.doubleJump = 0;
                }
                return true;
            }
        }
//        if(state.enemy.collision_check(state.player) && state.enemy.colide_bot(state.player)){
//            state.enemy.position.y += fabs( (state.player.position.y + state.player.size.y*0.5f)-(state.enemy.position.y - state.enemy.size.y*0.5f))+0.001f;
//        }
    }
    return false;
}


bool playerCollideTop(string type){
    int gridX, gridY;
    if(type == "player"){
        worldToTileCoordinates(state.player.position.x,state.player.position.y+0.5f*state.player.size.y,&gridX,&gridY);
        
        
        if(gridX < map.mapWidth && abs(gridY) < map.mapHeight){
            if(map.mapData[gridY][gridX] != 0 && map.mapData[gridY][gridX]<90){
                state.player.position.y -= fabs(((-tileSize * gridY) -tileSize) - (state.player.position.y + state.player.size.y*0.5f))+0.001f;
                cout << " Top == ture" <<endl;
                state.player.velocity.y = 0.0f;
                return true;
                
            }
        }
//        if(state.player.collision_check(state.enemy)){
//            state.player.position.y += fabs( (state.enemy.position.y - state.enemy.size.y*0.5f)-(state.player.position.y + state.player.size.y*0.5f))+0.001f;
//        }
    }else if(type == "enemy"){
        worldToTileCoordinates(state.enemy.position.x,state.enemy.position.y+0.5f*state.enemy.size.y,&gridX,&gridY);
        
        
        if(gridX < map.mapWidth && abs(gridY) < map.mapHeight){
            if(map.mapData[gridY][gridX] != 0 && map.mapData[gridY][gridX]<90){
                state.enemy.position.y -= fabs(((-tileSize * gridY) -tileSize) - (state.enemy.position.y + state.enemy.size.y*0.5f))+0.001f;
                cout << " Top == ture" <<endl;
                state.enemy.velocity.y = 0.0f;
                return true;
            }
        }
//        if(state.enemy.collision_check(state.player)){
//            state.enemy.position.y += fabs( (state.player.position.y - state.player.size.y*0.5f)-(state.enemy.position.y + state.enemy.size.y*0.5f))+0.001f;
//        }
    }
    
    
    return false;
}
bool playerCollideLeft(string type){
    int gridX, gridY;
    if(type == "player"){
        worldToTileCoordinates(state.player.position.x-0.5f*state.player.size.x,state.player.position.y,&gridX,&gridY);
//        if(state.player.collision_check(state.enemy) && state.player.colide_left(state.enemy)){
//            cout << "player Left == ture" <<endl;
//            state.player.position.x += fabs( (state.enemy.position.x + state.enemy.size.x*0.5f)-(state.player.position.x - state.player.size.x*0.5f))+0.00001f;
//        }
        if(gridX < map.mapWidth && abs(gridY) < map.mapHeight){
            if(map.mapData[gridY][gridX] != 0 && map.mapData[gridY][gridX]<90){
                state.player.position.x += fabs(((tileSize * gridX) + tileSize) - (state.player.position.x - state.player.size.x*0.5f))+0.00001f;
                cout << "Left == ture" <<endl;
                state.player.velocity.x = 0.0f;
                return true;
                
            }
        }
        
    }else if(type == "enemy"){
        worldToTileCoordinates(state.enemy.position.x-0.5f*state.enemy.size.x,state.enemy.position.y,&gridX,&gridY);
//        if(state.enemy.collision_check(state.player)&&state.enemy.colide_left(state.player)){
//            cout << "Enemy Left == ture" <<endl;
//            state.enemy.position.x += fabs( (state.player.position.x + state.player.size.x*0.5f)-(state.enemy.position.x - state.enemy.size.x*0.5f))+0.00001f;
//        }

        if(gridX < map.mapWidth && abs(gridY) < map.mapHeight){
            if(map.mapData[gridY][gridX] != 0 && map.mapData[gridY][gridX]<90){
                state.enemy.position.x += fabs(((tileSize * gridX) + tileSize) - (state.enemy.position.x - state.enemy.size.x*0.5f))+0.00001f;
                cout << "Left == ture" <<endl;
                state.enemy.velocity.x = 0.0f;
                return true;
                
            }
        }
    }
    
    return false;
}
bool playerCollideRight(string type){
    int gridX, gridY;
    if(type == "player"){
        worldToTileCoordinates(state.player.position.x+0.5f*state.player.size.x,state.player.position.y,&gridX,&gridY);
//        if(state.player.collision_check(state.enemy) && state.player.colide_right(state.enemy)){
//            cout << "Player Right == ture" <<endl;
//            state.player.position.x -= fabs( (state.enemy.position.x - state.enemy.size.x*0.5f)-(state.player.position.x + state.player.size.x*0.5f))+0.00001f;
//        }
        if(gridX < map.mapWidth && abs(gridY) < map.mapHeight){
            if(map.mapData[gridY][gridX] != 0 && map.mapData[gridY][gridX]<90){
                state.player.position.x -= fabs((tileSize * gridX) - (state.player.position.x + state.player.size.x / 2.0f))+0.000001f;
                cout << "Right == ture" <<endl;
                state.player.velocity.x = 0.0f;
                return true;
            }
        }
       
    }else if(type == "enemy"){
        worldToTileCoordinates(state.enemy.position.x+0.5f*state.enemy.size.x,state.enemy.position.y,&gridX,&gridY);
//        if(state.enemy.collision_check(state.player)&& state.enemy.colide_right(state.player)){
//            state.enemy.position.x -= fabs( (state.player.position.x - state.player.size.x*0.5f)-(state.enemy.position.x + state.enemy.size.x*0.5f))+0.00001f;
//        }
        if(gridX < map.mapWidth && abs(gridY) < map.mapHeight){
            if(map.mapData[gridY][gridX] != 0 && map.mapData[gridY][gridX]<90){
                state.enemy.position.x -= fabs((tileSize * gridX) - (state.enemy.position.x + state.enemy.size.x / 2.0f))+0.000001f;
                cout << "Right == ture" <<endl;
                state.enemy.velocity.x = 0.0f;
                return true;
            }
        }
    }
    return false;
}

float mapValue(float value, float srcMin, float srcMax, float dstMin, float dstMax) {
    float retVal = dstMin + ((value - srcMin)/(srcMax-srcMin) * (dstMax-dstMin));
    if(retVal < dstMin) {
        retVal = dstMin;
    }
    if(retVal > dstMax) {
        retVal = dstMax;
    }
    return retVal;
}

float animationTime = 0.0f;

void drawMap(){
    int sprite_count_x = 16;
    int sprite_count_y = 8;
    vector<float> vertexData;
    vector<float> texCoordData;
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
    displayWindow = SDL_CreateWindow("Welcome to my game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 590, SDL_WINDOW_OPENGL);
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
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 );
    bgm = Mix_LoadMUS(RESOURCE_FOLDER"my_hero.mp3");
    fontTexture = LoadTexture(RESOURCE_FOLDER"pixel_font.png");
    spriteTexture = LoadTexture(RESOURCE_FOLDER"sprites.png");
    todorokiSprite = LoadTexture(RESOURCE_FOLDER"todorokiSprite.png");
    starSprite = LoadTexture(RESOURCE_FOLDER"durian.png");
    bakugoSprite = LoadTexture(RESOURCE_FOLDER"bakugoSprite.png");
    keySprite = LoadTexture(RESOURCE_FOLDER"key.png");
    GGtexture = LoadTexture(RESOURCE_FOLDER"GG.png");
    GoodjobTexture = LoadTexture(RESOURCE_FOLDER"GoodJob.png");
    mainTexture = LoadTexture(RESOURCE_FOLDER"mainManue.png");
    exitTexture = LoadTexture(RESOURCE_FOLDER"exit.png");
    mainSprite = SheetSprite(mainTexture,1,0.75,1,2);
    footsteps = Mix_LoadWAV(RESOURCE_FOLDER"footstep.wav");
    collect = Mix_LoadWAV(RESOURCE_FOLDER"collect.wav");
    mainTexture = LoadTexture(RESOURCE_FOLDER"instructions.png");
    playertexture = SheetSprite(bakugoSprite,playerID,tileSize*scale,2,1);
    enemytexture = SheetSprite(todorokiSprite,enemyID,tileSize*scale,2,1);
//    playertexture = SheetSprite(spriteTexture,80,tileSize*scale,16,8);
    keytexture = SheetSprite(keySprite,0,tileSize,1,1);
    startexture = SheetSprite(starSprite,0,tileSize,1,1);
//    bombtexture = SheetSprite(spriteTexture,70,tileSize);
//    set_state(state1);
    map.Load(RESOURCE_FOLDER"map1.txt");
    cout<<map.mapWidth << " " << map.mapHeight<<endl;
    glUseProgram(program.programID);
    program.SetColor(1.0f, 0.5f, 0.0f,1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
}

void processEvents() {
    int gridX, gridY;
    switch (mode) {
        case STATE_MAIN_MENU:
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                    done = true;
                }else if(event.type == SDL_KEYDOWN){
                    if(event.key.keysym.scancode == SDL_SCANCODE_RETURN){
                        mode = STATE_INSTRUCTION;
                    }
                    if(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
                        mode = STATE_EXIT;
                    }
                }
            }
            break;
        case STATE_EXIT:{
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                    done = true;
                }else if(event.type == SDL_KEYDOWN){
                    if(event.key.keysym.scancode == SDL_SCANCODE_RETURN){
                        done = true;
                    }
                }
            }
            break;
        }case STATE_INSTRUCTION:
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                    done = true;
                }else if(event.type == SDL_KEYDOWN){
                    if(event.key.keysym.scancode == SDL_SCANCODE_RETURN){
                        mode = STATE_GAME_LEVEL_ONE;
                        
                    }
                    if(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
                        mode = STATE_EXIT;
                    }
                }
            }
            break;
        case STATE_GAME_LEVEL_ONE:{
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                    done = true;
                }else if(event.type == SDL_KEYDOWN){
//                    cout << "hummm I wonder why ";
//                    cout << playerCollideBottom() << " ";
//                    cout << state.player.collidedBot << endl;
                    if(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
                        mode = STATE_EXIT;
                    }
                    if(event.key.keysym.scancode == SDL_SCANCODE_RSHIFT) {
                        if(state.player.doubleJump>0){
                            state.player.velocity.y = 1.3f;
                            state.player.doubleJump -=1;
                            state.player.jump = true;
                            state.player.collidedBot = false;
                            cout << "jump!!!!!!!" << endl;
                        } //already jump twice
                        
                    }
                    if(event.key.keysym.scancode == SDL_SCANCODE_LSHIFT) {
                        if(state.enemy.doubleJump>0){
                            state.enemy.velocity.y = 1.3f;
                            state.enemy.doubleJump -=1;
                            state.enemy.jump = true;
                            state.enemy.collidedBot = false;
                            cout << "jump!!!!!!!" << endl;
                        } //already jump twice
                    }
                }
            }
            worldToTileCoordinates(state.player.position.x,state.player.position.y-0.5f* state.player.size.y,&gridX,&gridY);
            if(map.mapData[gridY][gridX] == 100){
                lose = true;
            }
            worldToTileCoordinates(state.enemy.position.x,state.enemy.position.y-0.5f* state.enemy.size.y,&gridX,&gridY);
            if(map.mapData[gridY][gridX] == 100){
                lose = true;
            }
            if(win == true){
                win = false;
                int temp1 = state.player.score;
                int temp2 = state.enemy.score;
                initial = true;
                set_state(state2);
                map.Load(RESOURCE_FOLDER"map2.txt");
                cout<<"GO TO L2"<<endl;
                state.player.score = temp1;
                state.enemy.score = temp2;
                mode = STATE_GAME_LEVEL_TWO;
                GRAVITY = 0.0f;
            }
            if(lose == true){
                mode = STATE_GAME_OVER;
            }
             break;
        }case STATE_GAME_LEVEL_TWO:{
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                        done = true;
                    }else if(event.type == SDL_KEYDOWN){
                        if(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
                            mode = STATE_EXIT;
                        }
                    }
                }
                worldToTileCoordinates(state.player.position.x,state.player.position.y-0.5f* state.player.size.y,&gridX,&gridY);
                if(map.mapData[gridY][gridX] == 100){
                    lose = true;
                }
                worldToTileCoordinates(state.enemy.position.x,state.enemy.position.y-0.5f* state.enemy.size.y,&gridX,&gridY);
                if(map.mapData[gridY][gridX] == 100){
                    lose = true;
                }
                if(win == true){
                    win = false;
                    int temp1 = state.player.score;
                    int temp2 = state.enemy.score;
                    initial = true;
                    set_state(state3);
                    map.Load(RESOURCE_FOLDER"map3.txt");
                    cout<<"GO TO L3"<<endl;
                    state.player.score = temp1;
                    state.enemy.score = temp2;
                    //                    cout<<map.mapWidth << " " << map.mapHeight<<endl;
                    GRAVITY = 1.0f;
                    mode = STATE_GAME_LEVEL_THREE;
                }
                if(lose == true){
                    mode = STATE_GAME_OVER;
                }
                
                
                break;
        }case STATE_GAME_LEVEL_THREE:{
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                        done = true;
                    }else if(event.type == SDL_KEYDOWN){
                        //                    cout << "hummm I wonder why ";
                        //                    cout << playerCollideBottom() << " ";
                        //                    cout << state.player.collidedBot << endl;
                        if(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
                            mode = STATE_EXIT;
                        }
                        if(event.key.keysym.scancode == SDL_SCANCODE_RSHIFT) {
                            if(state.player.doubleJump>0){
                                state.player.velocity.y = 1.3f;
                                state.player.doubleJump -=1;
                                state.player.jump = true;
                                state.player.collidedBot = false;
                                cout << "jump!!!!!!!" << endl;
                            } //already jump twice
                            
                        }
                        if(event.key.keysym.scancode == SDL_SCANCODE_LSHIFT) {
                            if(state.enemy.doubleJump>0){
                                state.enemy.velocity.y = 1.3f;
                                state.enemy.doubleJump -=1;
                                state.enemy.jump = true;
                                state.enemy.collidedBot = false;
                                cout << "jump!!!!!!!" << endl;
                            } //already jump twice
                        }
                    }
                }
                worldToTileCoordinates(state.player.position.x,state.player.position.y-0.5f* state.player.size.y,&gridX,&gridY);
                if(map.mapData[gridY][gridX] == 100){
                    lose = true;
                }
                worldToTileCoordinates(state.enemy.position.x,state.enemy.position.y-0.5f* state.enemy.size.y,&gridX,&gridY);
                if(map.mapData[gridY][gridX] == 100){
                    lose = true;
                }
                if(win == true){
                    mode = STATE_GAME_OVER;
                }
                if(lose == true){
                    mode = STATE_GAME_OVER;
                }
                break;
            }
            case STATE_GAME_OVER:{
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                        done = true;
                    }else if(event.type == SDL_KEYDOWN){
                        if(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
                            cout << "exit!"<<endl;
                            mode = STATE_EXIT;
                        }
                        if(event.key.keysym.scancode == SDL_SCANCODE_SPACE){
                            win = false;
                            lose = false;
//                            int temp1 = state.player.score;
//                            int temp2 = state.enemy.score;
                            initial = true;
                            set_state(state);
                            map.Load(RESOURCE_FOLDER"map1.txt");
                            cout<<"GO TO L1"<<endl;
//                            state.player.score = temp1;
//                            state.enemy.score = temp2;
                            mode = STATE_GAME_LEVEL_ONE;
//                            GRAVITY = 1.0f;
                        }
                    }
                }
                break;
            }
        }
}

void update(float elapsed) {
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    modelMatrix = glm::mat4(1.0f);
//    state.player.velocity.y = 0.0f;
    switch (mode) {
        case STATE_MAIN_MENU:
            if(keys[SDL_SCANCODE_SPACE]){
                Mix_PlayChannel( -1, footsteps, 0);
                mainAnimate -= elapsed;
                if(mainAnimate < 0){
                    if(mainSprite.index == 1){
                        mainSprite.index = 0;
                    }
                    else{
                        mainSprite.index = 1;
                    }
                    enemyAnimate = 0.1f;
                }
            }else{
                mainSprite.index = 1;
            }
            break;
        case STATE_EXIT:
            break;
        case STATE_INSTRUCTION:
            animationTime = animationTime + elapsed;
            break;
        case STATE_GAME_LEVEL_ONE:
        case STATE_GAME_LEVEL_THREE:{
            if(keys[SDL_SCANCODE_LEFT] && !playerCollideLeft("player")){
//                cout << "left" <<endl;
//                Mix_PlayChannel( -1, footsteps, 0);
                state.player.acceleration.x = -1.5f;
                playerAnimate -= elapsed;
                if(playerAnimate < 0 && !(state.player.jump)){
                    animatePlayer();
                    playerAnimate = .1;
                }
            }else if(keys[SDL_SCANCODE_RIGHT] && !playerCollideRight("player")){
//                cout << "right" << endl;
//                Mix_PlayChannel( -1, footsteps, 0);
                state.player.acceleration.x = 1.5f;
                playerAnimate -= elapsed;
                if(playerAnimate < 0 && !(state.player.jump)){
                    animatePlayer();
                    playerAnimate = .1;
                }
            }else{
                state.player.velocity.x = 0.0f;
                playerID = 1;
                state.player.sprite.index = playerID;
            }
            if(keys[SDL_SCANCODE_A] && !playerCollideLeft("enemy")){
//                cout << "left" <<endl;
//                Mix_PlayChannel( -1, footsteps, 0);
                state.enemy.acceleration.x = -1.5f;
                enemyAnimate -= elapsed;
                if(enemyAnimate < 0 && !(state.enemy.jump)){
                    animatenemy();
                    enemyAnimate = .1;
                }
            }else if(keys[SDL_SCANCODE_D] && !playerCollideRight("enemy")){
//                cout << "right" << endl;
//                Mix_PlayChannel( -1, footsteps, 0);
                state.enemy.acceleration.x = 1.5f;
                enemyAnimate -= elapsed;
                if(enemyAnimate < 0 && !(state.enemy.jump)){
                    animatenemy();
                    enemyAnimate = .1;
                }
            }else{
                state.enemy.velocity.x = 0.0f;
                enemyID = 1;
                state.enemy.sprite.index = enemyID;
            }
            if(state.player.collision_check(state.key)||state.enemy.collision_check(state.key)){
                Mix_PlayChannel( -1, collect, 0);
                state.key.position.x = -1000000;
                win = true;
            }
            for(int i = 0; i < 5; i++){
                if(state.player.collision_check(state.stars[i])){
                    Mix_PlayChannel( -1, collect, 0);
                    state.player.score += 1;
                    state.stars[i].position.x = -1000000;
                }
                if(state.enemy.collision_check(state.stars[i])){
                    Mix_PlayChannel( -1, collect, 0);
                    state.enemy.score += 1;
                    state.stars[i].position.x = -1000000;
                }
            }
            if(!win && !lose){
                state.player.update(elapsed);
                state.enemy.update(elapsed);
                state.player.acceleration.x = 0.0f;
                state.enemy.acceleration.x = 0.0f;
            }
            break;
        
        }case STATE_GAME_LEVEL_TWO:{
            if(keys[SDL_SCANCODE_LEFT] && !playerCollideLeft("player")){
//                cout << "left" <<endl;
//                Mix_PlayChannel( -1, footsteps, 0);
                state.player.acceleration.x = -1.5f;
                playerAnimate -= elapsed;
                if(playerAnimate < 0){
                    animatePlayer();
                    playerAnimate = .1;
                }
            }else if(keys[SDL_SCANCODE_RIGHT] && !playerCollideRight("player")){
//                cout << "right" << endl;
//                Mix_PlayChannel( -1, footsteps, 0);
                state.player.acceleration.x = 1.5f;
                playerAnimate -= elapsed;
                if(playerAnimate < 0){
                    animatePlayer();
                    playerAnimate = .1;
                }
            }else if(keys[SDL_SCANCODE_UP] && !playerCollideTop("player")){
//                cout << "up" << endl;
//                Mix_PlayChannel( -1, footsteps, 0);
                state.player.acceleration.y = 1.5f;
                playerAnimate -= elapsed;
                if(playerAnimate < 0){
                    animatePlayer();
                    playerAnimate = .1;
                }
            }else if(keys[SDL_SCANCODE_DOWN] && !playerCollideBottom("player")){
//                cout << "down" << endl;
//                Mix_PlayChannel( -1, footsteps, 0);
                state.player.acceleration.y = -1.5f;
                playerAnimate -= elapsed;
                if(playerAnimate < 0){
                    animatePlayer();
                    playerAnimate = .1;
                }
            }else{
                state.player.velocity.x = 0.0f;
                state.player.velocity.y = 0.0f;
                playerID = 1;
                state.player.sprite.index = playerID;
            }
            if(keys[SDL_SCANCODE_A] && !playerCollideLeft("enemy")){
                //                cout << "left" <<endl;
//                Mix_PlayChannel( -1, footsteps, 0);
                state.enemy.acceleration.x = -1.5f;
                enemyAnimate -= elapsed;
                if(enemyAnimate < 0){
                    animatenemy();
                    enemyAnimate = .1;
                }
            }else if(keys[SDL_SCANCODE_D] && !playerCollideRight("enemy")){
                //                cout << "right" << endl;
//                Mix_PlayChannel( -1, footsteps, 0);
                state.enemy.acceleration.x = 1.5f;
                enemyAnimate -= elapsed;
                if(enemyAnimate < 0){
                    animatenemy();
                    enemyAnimate = .1;
                }
            }else if(keys[SDL_SCANCODE_W] && !playerCollideTop("enemy")){
                //                cout << "up" << endl;
                state.enemy.acceleration.y = 1.5f;
                enemyAnimate -= elapsed;
                if(enemyAnimate < 0){
                    animatenemy();
                    enemyAnimate = .1;
                }
            }else if(keys[SDL_SCANCODE_S] && !playerCollideBottom("enemy")){
                //                cout << "down" << endl;
//                Mix_PlayChannel( -1, footsteps, 0);
                state.enemy.acceleration.y = -1.5f;
                enemyAnimate -= elapsed;
                if(enemyAnimate < 0){
                    animatenemy();
                    enemyAnimate = .1;
                }
            }else{
                state.enemy.velocity.x = 0.0f;
                state.enemy.velocity.y = 0.0f;
                enemyID = 1;
                state.enemy.sprite.index = enemyID;
            }
            if(state.player.collision_check(state.key)||state.enemy.collision_check(state.key)){
                Mix_PlayChannel( -1, collect, 0);
                
                state.key.position.x = -1000000;
                win = true;
            }
            for(int i = 0; i < 5; i++){
                if(state.player.collision_check(state.stars[i])){
                    Mix_PlayChannel( -1, collect, 0);
                    state.player.score += 1;
                    state.stars[i].position.x = -1000000;
                }
                if(state.enemy.collision_check(state.stars[i])){
                    Mix_PlayChannel( -1, collect, 0);
                    state.enemy.score += 1;
                    state.stars[i].position.x = -1000000;
                }
            }
            if(!win && !lose){
                state.player.update(elapsed);
                state.enemy.update(elapsed);
                state.player.acceleration.x = 0.0f;
                state.player.acceleration.y = 0.0f;
                state.enemy.acceleration.x = 0.0f;
                state.enemy.acceleration.y = 0.0f;

            }
            break;
//        case STATE_GAME_LEVEL_THREE:
//            break;
        }case STATE_GAME_OVER:
            break;
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    switch (mode) {
        case STATE_MAIN_MENU:{
            glClearColor(0.4f, 0.3f, 0.5f, 1.0f);
            DrawText(program, fontTexture, "Welcome back, my old friend!", -1.1f,0.7f,0.07f, 0.01f);
            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -0.05f, 0.0f));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(1.8f, 1.6f, 0.0f));
            program.SetModelMatrix(modelMatrix);
            mainSprite.Draw(program);
            DrawText(program, fontTexture, "Created by Manxueying Li", 0.4f,-0.7f, 0.04f, 0.01f);
            DrawText(program, fontTexture, "Press Enter To Move On", 0.2f,-0.9f, 0.05f, 0.01f);
            break;
        }
        case STATE_EXIT:{
            modelMatrix = glm::mat4(1.0f);
            glm::mat4 viewMatrix = glm::mat4(1.0f);
            program.SetViewMatrix(viewMatrix);
            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::scale(modelMatrix, glm::vec3(2.1f, 2.1f, 0.0f));
            program.SetModelMatrix(modelMatrix);
            glBindTexture(GL_TEXTURE_2D,exitTexture);
            float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
            glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,vertices);
            glEnableVertexAttribArray(program.positionAttribute);
            float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
            glVertexAttribPointer(program.texCoordAttribute,2,GL_FLOAT,false,0,texCoords);
            glEnableVertexAttribArray(program.texCoordAttribute);
            glDrawArrays(GL_TRIANGLES, 0,6);
            glDisableVertexAttribArray(program.positionAttribute);
            glDisableVertexAttribArray(program.texCoordAttribute);
            break;
        }case STATE_INSTRUCTION:{
            float animationValue = mapValue(animationTime, 0.0f, 15.0f, 0.0f, 1.0f);
            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::scale(modelMatrix, glm::vec3(1.9f, 1.9f, 0.0f));
            float yPos = lerp(-1.0f, 0.75f, animationValue);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f,yPos, 0.0f));
            program.SetModelMatrix(modelMatrix);
            glBindTexture(GL_TEXTURE_2D,mainTexture);
            float vertices[] = {-0.5, -1.0, 0.5, -1.0, 0.5, 1.0, -0.5, -1.0, 0.5, 1.0, -0.5, 1.0};
            glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,vertices);
            glEnableVertexAttribArray(program.positionAttribute);
            float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
            glVertexAttribPointer(program.texCoordAttribute,2,GL_FLOAT,false,0,texCoords);
            glEnableVertexAttribArray(program.texCoordAttribute);
            glDrawArrays(GL_TRIANGLES, 0,6);
            glDisableVertexAttribArray(program.positionAttribute);
            glDisableVertexAttribArray(program.texCoordAttribute);
            DrawText(program, fontTexture, "Press Enter To Start", 0.4f,-0.9f, 0.05f, 0.01f);
            break;
        }
            
        case STATE_GAME_LEVEL_ONE:{
            modelMatrix = glm::mat4(1.0f);
            program.SetModelMatrix(modelMatrix);
            glClearColor(0.0f, 0.7f, 1.0f, 1.0f);
            drawMap();
            state.player.draw(program);
            state.key.draw(program);
            state.enemy.draw(program);
            for(int i = 0; i < 5; i ++){
                state.stars[i].draw(program);
            }
            //re-center
            glm::mat4 viewMatrix = glm::mat4(1.0f);
            viewMatrix = glm::translate(viewMatrix, glm::vec3(-0.5*fabs(state.player.position.x+state.enemy.position.x), 0.5*fabs(state.player.position.y+state.enemy.position.y), 0.0f));
            program.SetViewMatrix(viewMatrix);
            break;
        }
            
        case STATE_GAME_LEVEL_TWO:{
            modelMatrix = glm::mat4(1.0f);
            program.SetModelMatrix(modelMatrix);
            glClearColor(0.8f, 0.5f, 0.1f, 1.0f);
            drawMap();
            state.player.draw(program);
            state.key.draw(program);
            state.enemy.draw(program);
            for(int i = 0; i < 5; i ++){
                state.stars[i].draw(program);
            }
            //re-center
            glm::mat4 viewMatrix = glm::mat4(1.0f);
            viewMatrix = glm::translate(viewMatrix, glm::vec3(-0.5*fabs(state.player.position.x+state.enemy.position.x), 0.5*fabs(state.player.position.y+state.enemy.position.y), 0.0f));
            program.SetViewMatrix(viewMatrix);
            break;
        }case STATE_GAME_LEVEL_THREE:{
            modelMatrix = glm::mat4(1.0f);
            program.SetModelMatrix(modelMatrix);
            glClearColor(0.6f, 0.6f, 0.1f, 1.0f);
            drawMap();
            state.player.draw(program);
            state.key.draw(program);
            state.enemy.draw(program);
            for(int i = 0; i < 5; i ++){
                state.stars[i].draw(program);
            }
            //re-center
            glm::mat4 viewMatrix = glm::mat4(1.0f);
            viewMatrix = glm::translate(viewMatrix, glm::vec3(-0.5*fabs(state.player.position.x+state.enemy.position.x), 0.5*fabs(state.player.position.y+state.enemy.position.y), 0.0f));
            program.SetViewMatrix(viewMatrix);
            break;
        }case STATE_GAME_OVER:{
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glm::mat4 viewMatrix = glm::mat4(1.0f);
            program.SetViewMatrix(viewMatrix);
            if(win == true){
                DrawText(program, fontTexture, "Finally", -1.42f,0.2f,0.05f, 0.01f);
                DrawText(program, fontTexture, "Congret!",-1.42f,0.0f, 0.05f, 0.03f);
                DrawText(program, fontTexture, "p1 score: "+to_string(state.player.score),-1.42f,-0.2f, 0.05f, 0.03f);
                DrawText(program, fontTexture, "p2 score: "+to_string(state.enemy.score),-1.42f,-0.4f, 0.05f, 0.03f);
                glm::mat4 viewMatrix = glm::mat4(1.0f);
                program.SetViewMatrix(viewMatrix);
                modelMatrix = glm::mat4(1.0f);
                modelMatrix = glm::translate(modelMatrix, glm::vec3(0.3f, 0.14f, 0.0f));
                modelMatrix = glm::scale(modelMatrix, glm::vec3(1.7f, 1.7f, 0.0f));
                program.SetModelMatrix(modelMatrix);
                glBindTexture(GL_TEXTURE_2D,GoodjobTexture);
                float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
                glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,vertices);
                glEnableVertexAttribArray(program.positionAttribute);
                float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
                glVertexAttribPointer(program.texCoordAttribute,2,GL_FLOAT,false,0,texCoords);
                glEnableVertexAttribArray(program.texCoordAttribute);
                glDrawArrays(GL_TRIANGLES, 0,6);
                glDisableVertexAttribArray(program.positionAttribute);
                glDisableVertexAttribArray(program.texCoordAttribute);
                
            }else{
                DrawText(program, fontTexture, "YOU DIED!",-1.42f,-0.0f, 0.05f, 0.03f);
                DrawText(program, fontTexture, "GAME OVER!",-1.42f,-0.4f, 0.05f, 0.03f);
                modelMatrix = glm::mat4(1.0f);
                modelMatrix = glm::translate(modelMatrix, glm::vec3(0.4f, -0.1f, 0.0f));
                modelMatrix = glm::scale(modelMatrix, glm::vec3(1.7f, 1.7f, 0.0f));
                program.SetModelMatrix(modelMatrix);
                glBindTexture(GL_TEXTURE_2D,GGtexture);
                float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
                glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,vertices);
                glEnableVertexAttribArray(program.positionAttribute);
                float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
                glVertexAttribPointer(program.texCoordAttribute,2,GL_FLOAT,false,0,texCoords);
                glEnableVertexAttribArray(program.texCoordAttribute);
                glDrawArrays(GL_TRIANGLES, 0,6);
                glDisableVertexAttribArray(program.positionAttribute);
                glDisableVertexAttribArray(program.texCoordAttribute);
            }
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    setUp();
    Mix_PlayMusic(bgm, -1);
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
