#pragma once

#include "ofMain.h"
#include "ofxPiMapper.h"
#include "VideoSource.h"
#include "SyphonSource.h"
#include "ofxImGui.h"
#include "ofxXmlSettings.h"
#include "ofxOsc.h"
#include "Secuencia.h"
#include <thread>


class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseDragged(int x, int y, int button);
    
    //-----------------OFXPIMAPPER ------------------
    ofxPiMapper piMapper;
    
    // By using a custom source that is derived from FboSource
    // you will be able to see the source listed in sources editor
    SyphonSource SyphonClient;
    
    bool editMode = false; // Modo edición
    bool loop = false;
    
    std::vector<std::string> sequenceFolders;
    
    Secuencia * secuencia;
    vector<Secuencia *> secuencias;
    
    void loadData();

    
    //----------------- GUI -------------------
    void drawGui();
    ofxImGui::Gui gui;
    
    // Variables para guardar los nombres de los archivos
    std::vector<std::string> imageFiles;
    std::vector<std::string> videoFiles;
    std::vector<std::string> audioFiles;
    
    ofDirectory dir;
    
    //----------------- OSC -------------------
    ofxOscReceiver receiver;
    int oscPort = 3333;
    void processOscMessage(const ofxOscMessage& message);
};

