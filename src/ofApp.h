#pragma once

#include "ofMain.h"
#include "ofxPiMapper.h"
#include "VideoSource.h"
#include "VideoServer.h"
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
    VideoServer videoServer;
    
    bool editMode = false; // Modo edición
    bool loop = false;
    
    std::vector<std::string> sequenceFolders;
    
    Secuencia * secuencia;
    vector<Secuencia *> secuencias;
    
    void loadData();

    //----------------- SETTINGS -------------------
    void loadSettings();
    void saveSettings();
    ofxXmlSettings          XML;
    string                  xmlMessage;

    //----------------- GUI -------------------
    void drawGui();
    ofxImGui::Gui gui;
    
    // Variables para guardar los nombres de los archivos
    std::vector<std::string> imageFiles;
    std::vector<std::string> videoFiles;
    std::vector<std::string> audioFiles;
    
    ofDirectory dir;
    
    bool showHelpPopup = false;
    
    //----------------- OSC -------------------
    ofxOscReceiver receiver;
    int oscPort;
    void processOscMessage(const ofxOscMessage& message);
    
private:
    
    //----------------- GUI -------------------
    struct HelpPopup {
        string title;
        string content;
        bool shouldOpen = false;
    };
    HelpPopup helpPopup;
};

