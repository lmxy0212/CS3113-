#include "worldMap.hpp"


//worldMap:: worldMap() {
//    mapData = nullptr;
//    mapWidth = -1;
//    mapHeight = -1;
//}
//
//bool worldMap::ReadHeader( ifstream &stream) {
//    string line;
//    mapWidth = -1;
//    mapHeight = -1;
//    while( getline(stream, line)) {
//        if(line == "") { break; }
//         istringstream sStream(line);
//         string key,value;
//         getline(sStream, key, '=');
//         getline(sStream, value);
//        if(key == "width") {
//            mapWidth =  atoi(value.c_str());
//        } else if(key == "height"){
//            mapHeight =  atoi(value.c_str());
//        }
//    }
//    if(mapWidth == -1 || mapHeight == -1) {
//        return false;
//    } else {
//        mapData = new unsigned int*[mapHeight];
//        for(int i = 0; i < mapHeight; ++i) {
//            mapData[i] = new unsigned int[mapWidth];
//        }
//        return true;
//    }
//}
//
//bool  worldMap::ReadLayerData(ifstream &stream) {
//    string line;
//    while(getline(stream, line)) {
//        if(line == "") { break; }
//         istringstream sStream(line);
//         string key,value;
//         getline(sStream, key, '=');
//         getline(sStream, value);
//        if(key == "data") {
//            for(int y=0; y < mapHeight; y++) {
//                getline(stream, line);
//                istringstream lineStream(line);
//                string tile;
//                for(int x=0; x < mapWidth; x++) {
//                     getline(lineStream, tile, ',');
//                    unsigned int val = atoi(tile.c_str());
//                    if(val > 0) {
//                        mapData[y][x] = val-1;
//                    } else {
//                        mapData[y][x] = 0;
//                    }
//                }
//            }
//        }
//    }
//    return true;
//}
//
//
//bool  worldMap::ReadEntityData(ifstream &stream) {
//    string line;
//    string type;
//    while(getline(stream, line)) {
//        if(line == "") { break; }
//        istringstream sStream(line);
//        string key,value;
//        getline(sStream, key, '=');
//        getline(sStream, value);
//        if(key == "type") {
//            type = value;
//        } else if(key == "location") {
//            istringstream lineStream(value);
//            string xPosition, yPosition;
//            getline(lineStream, xPosition, ',');
//            getline(lineStream, yPosition, ',');
//            float placeX = atoi(xPosition.c_str())*tileSize;
//            float placeY = atoi(yPosition.c_str())*-tileSize;
//            placeEntity(type, placeX, placeY);
//        }
//    }
//    return true;
//}
//
//void  worldMap::Load(const  string fileName) {
//    ifstream infile(fileName);
//    if(infile.fail()) {
//        assert(false); // unable to open file
//    }
//    string line;
//    while (getline(infile, line)) {
//        if(line == "[header]") {
//            if(!ReadHeader(infile)) {
//                assert(false); // invalid file data
//            }
//        } else if(line == "[layer]") {
//            ReadLayerData(infile);
//        } else if(line == "[ObjectsLayer]") {
//            ReadEntityData(infile);
//        }
//    }
//}
//
//void worldMap::placeEntity(string type, int placeX, int placeY){
//
//}
