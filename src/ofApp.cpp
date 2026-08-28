#include "ofApp.h"

void ofApp::setup() {
    ofBackground(0, 255);
    ofSetWindowTitle("BMapper");
    
    //----------------- GUI -------------------
    loadSettings();
    //required call
    gui.setup();
    editModeFont.load("verdana.ttf", 12);
    
    ImGui::GetIO().MouseDrawCursor = false;
    
    loadData();
    
    piMapper.setup();
    
    //----------------- OSC -------------------
    receiver.setup(oscPort);
}

void ofApp::update(){
    piMapper.update();
    int processedMessages = 0;
    while (receiver.hasWaitingMessages() && processedMessages < MAX_OSC_MESSAGES_PER_FRAME) {
        ofxOscMessage m;
        receiver.getNextMessage(m);
        processOscMessage(m);
        ++processedMessages;
    }
}

void ofApp::draw(){
    if (editMode) {
        ofPushStyle();
        ofBackground(100);
        ofSetColor(80);
        
        const string text = "MODO EDICIÓN";
        const int textWidth = 200;
        const int textHeight = 50;
        
        int screenWidth = ofGetWidth();
        int screenHeight = ofGetHeight();
        
        // Dibujar el texto en un patrón de mosaico
        for (int y = 0; y < screenHeight; y += textHeight) {
            for (int x = 0; x < screenWidth; x += textWidth) {
                editModeFont.drawString(text, x, y);
            }
        }
        
        ofPopStyle();
    } else {
        ofBackground(0);
        ofSetColor(255);
    }
    
    piMapper.draw();
    drawGui();
}

void ofApp::keyPressed(int key) {
    
    if((key == 'e' || key == 'E' || key==5) && (ofGetKeyPressed(OF_KEY_COMMAND) || ofGetKeyPressed(OF_KEY_CONTROL))) {
        editMode = !editMode;
        if(editMode){
            piMapper.setMode(ofx::piMapper::MAPPING_MODE);
        }else{
            piMapper.setMode(ofx::piMapper::PRESENTATION_MODE);
        }
    }
    
    if((key == 's' || key == 'S' || key == 19) && (ofGetKeyPressed(OF_KEY_COMMAND) || ofGetKeyPressed(OF_KEY_CONTROL))) {
        piMapper.saveProject();
        saveSettings();
        ofLogNotice() << "--------> PROYECTO GUARDADO";
    }
    
    if ((key == 'z' || key == 'Z' || key == 26) && (ofGetKeyPressed(OF_KEY_COMMAND) || ofGetKeyPressed(OF_KEY_CONTROL))) {
        piMapper.undo();
    }
    
    if ((key == 'f' || key == 'F' || key == 6) && (ofGetKeyPressed(OF_KEY_COMMAND) || ofGetKeyPressed(OF_KEY_CONTROL))) {
        ofToggleFullscreen();
    }
    if ((key == 'p' || key == 'P' || key == 16) && (ofGetKeyPressed(OF_KEY_CONTROL))) {
        piMapper.setMode(ofx::piMapper::PRESENTATION_MODE);
        ofSetFullscreen(true);
        editMode = false;
    }

    if (key == OF_KEY_F1) {
        showHelpPopup = true;
    }

    if (editMode && ImGui::GetIO().WantCaptureKeyboard) {
        return;
    }
    
    switch(key){
        case 'l':
            piMapper.toggleLayerPanel();
            break;
        case '+':
            piMapper.scaleUp();
            break;
        case '-':
            piMapper.scaleDown();
            break;
        case OF_KEY_UP:
            if(ofGetKeyPressed(OF_KEY_SHIFT)){
                piMapper._application.moveSelection(ofx::piMapper::Vec3(0.0f, -10.0f, 0.0f));
            }else{
                piMapper._application.moveSelection(ofx::piMapper::Vec3(0.0f, -1.0f, 0.0f));
            }
            break;
               
        case OF_KEY_DOWN:
            if(ofGetKeyPressed(OF_KEY_SHIFT)){
                piMapper._application.moveSelection(ofx::piMapper::Vec3(0.0f, 10.0f, 0.0f));
            }else{
                piMapper._application.moveSelection(ofx::piMapper::Vec3(0.0f, 1.0f, 0.0f));
            }
            break;
               
        case OF_KEY_LEFT:
            if(ofGetKeyPressed(OF_KEY_SHIFT)){
                piMapper._application.moveSelection(ofx::piMapper::Vec3(-10.0f, 0.0f, 0.0f));
            }else{
                piMapper._application.moveSelection(ofx::piMapper::Vec3(-1.0f, 0.0f, 0.0f));
            }
            break;

        case OF_KEY_RIGHT:
            if(ofGetKeyPressed(OF_KEY_SHIFT)){
                piMapper._application.moveSelection(ofx::piMapper::Vec3(10.0f, 0.0f, 0.0f));
            }else{
                piMapper._application.moveSelection(ofx::piMapper::Vec3(1.0f, 0.0f, 0.0f));
            }
            break;
        case ',':
            piMapper.selectPrevSurface();
            break;
        case '.':
            piMapper.selectNextSurface();
            break;
        case '<':
            piMapper.selectPrevVertex();
            break;
        case '>':
            piMapper.selectNextVertex();
            break;
    }
    
    if(editMode && !ImGui::IsAnyItemActive()){
        
        switch(key){
            case 't':
                piMapper.createSurface(ofx::piMapper::TRIANGLE_SURFACE);
                break;
            case 'q':
                piMapper.createSurface(ofx::piMapper::QUAD_SURFACE);
                break;
            case 'g':
                piMapper.createSurface(ofx::piMapper::GRID_WARP_SURFACE);
                break;
            case 'h':
                piMapper.createSurface(ofx::piMapper::HEXAGON_SURFACE);
                break;
            case 'c':
                piMapper.createSurface(ofx::piMapper::CIRCLE_SURFACE);
                break;
            case 'd':
                piMapper.duplicateSurface();
                break;
            case OF_KEY_BACKSPACE:
                int selectedSurface = piMapper.getSelectedSurface();
                piMapper.eraseSurface(selectedSurface);
                break;
        }
    } else if (!editMode) {
        
    }
    
    // Ejecutar 'play' para VideoSurface y FboSource con tecla asignada
    for (int i = 0; i < piMapper.getNumSurfaces(); ++i) {
    ofx::piMapper::BaseSurface* surface = piMapper.getSurfaceAt(i);
        if (surface->getAssignedKey() == key) {
            ofx::piMapper::BaseSource* selectedSource = surface->getSource();
            if (selectedSource->getType() == ofx::piMapper::SourceType::SOURCE_TYPE_VIDEO ||
                selectedSource->getType() == ofx::piMapper::SourceType::SOURCE_TYPE_FBO) {
                piMapper.playForSurface(i);
            }
        }
    }
}

void ofApp::keyReleased(int key) {
    piMapper.keyReleased(key);
}

void ofApp::mouseDragged(int x, int y, int button) {
    if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsAnyItemHovered()) {
        piMapper.mouseDragged(x, y, button);
    }
}

void ofApp::mousePressed(int x, int y, int button) {
    if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsAnyItemHovered()) {
        piMapper.mousePressed(x, y, button);
    }
}

void ofApp::mouseReleased(int x, int y, int button) {
    if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsAnyItemHovered()) {
        piMapper.mouseReleased(x, y, button);
    }
}


void ofApp::processOscMessage(const ofxOscMessage& message) {
    const std::string address = message.getAddress();
    if (address.empty() || message.getNumArgs() == 0 || message.getArgType(0) != OFXOSC_TYPE_STRING) {
        ofLogWarning("OSC") << "Mensaje OSC inválido recibido en " << address;
        return;
    }

    const std::string command = ofToLower(message.getArgAsString(0));
    for (int i = 0; i < piMapper.getNumSurfaces(); ++i) {
        ofx::piMapper::BaseSurface* surface = piMapper.getSurfaceAt(i);
        if (surface == nullptr || surface->getOscAddress() != address || surface->getSource() == nullptr) {
            continue;
        }

        const auto sourceType = surface->getSource()->getType();
        if (sourceType != ofx::piMapper::SourceType::SOURCE_TYPE_VIDEO &&
            sourceType != ofx::piMapper::SourceType::SOURCE_TYPE_FBO) {
            ofLogWarning("OSC") << "La superficie " << i << " no admite controles de reproducción";
            continue;
        }

        if (command == "play") {
            piMapper.playForSurface(i);
        } else if (command == "pause") {
            piMapper.pauseForSurface(i);
        } else if (command == "stop") {
            piMapper.stopForSurface(i);
        } else if (command == "resume") {
            piMapper.resumeForSurface(i);
        } else {
            ofLogWarning("OSC") << "Comando OSC no reconocido: " << command;
        }
    }
}

void ofApp::loadData() {
    // Configuración inicial
    ofDirectory imageDirectory(DEFAULT_IMAGES_DIR);
    imageDirectory.allowExt("jpg");
    imageDirectory.allowExt("png");
    imageDirectory.allowExt("bmp");
    imageDirectory.allowExt("gif");
    imageDirectory.listDir();
    imageDirectory.sort();
    for(const auto& file : imageDirectory.getFiles()){
        ofLogNotice() << "Imagen encontrada: " << file.getFileName(); // Mensaje de depuración
        imageFiles.push_back(file.getFileName());
    }
    
    ofDirectory videoDirectory(DEFAULT_VIDEOS_DIR);
    videoDirectory.allowExt("mp4");
    videoDirectory.allowExt("mov");
    videoDirectory.allowExt("avi");
    videoDirectory.listDir();
    videoDirectory.sort();
    for(const auto& file : videoDirectory.getFiles()){
        ofLogNotice() << "Video encontrado: " << file.getFileName(); // Mensaje de depuración
        videoFiles.push_back(file.getFileName());
    }
    
    ofDirectory audioDirectory(DEFAULT_SOUNDS_DIR);
    audioDirectory.allowExt("wav");
    audioDirectory.allowExt("aiff");
    audioDirectory.allowExt("aif");
    audioDirectory.allowExt("mp3");
    audioDirectory.listDir();
    audioDirectory.sort();
    for (const auto& file : audioDirectory.getFiles()) {
        ofLogNotice() << "Audio encontrado: " << file.getFileName();
        audioFiles.push_back(file.getFileName());
    }
    
    //-----------------OFXPIMAPPER ------------------
    // Enable or disable audio for video sources globally
    // Set this to false to save resources on the Raspberry Pi
    ofx::piMapper::VideoSource::enableAudio = true;
    ofx::piMapper::VideoSource::useHDMIForAudio = false;
    
    // Register our sources.
    // This should be done before mapper.setup().
    piMapper.registerFboSource(videoServer);
        
    // Aquí crearemos una nueva instancia de Secuencia y la asignaremos

    ofDirectory sequenceDirectory(DEFAULT_SEQUENCES_DIR);
    sequenceDirectory.listDir();
    sequenceDirectory.sort();
    for (const auto& folder : sequenceDirectory.getFiles()) {
        if (folder.isDirectory()) {
            std::string folderName = folder.getFileName();
            ofLogNotice() << "----------->Secuencia encontrada: " << folderName; // Mensaje de depuración
            sequenceFolders.push_back(folder.getFileName());
        
            // Aquí crearemos una nueva instancia de Secuencia y la asignaremos
            auto* sequenceSource = new Secuencia();
            sequenceSource->setup(folderName);
            std::string nombre = sequenceSource->getName();
            //ofLogNotice()<< "----------->nombre de fuente: " + nombre;
            if(!sequenceSource->loadSequence(std::string(DEFAULT_SEQUENCES_DIR) + nombre)) {
                ofLogWarning() << "No se pudieron cargar imágenes en la secuencia " << nombre;
            }
            
            piMapper.registerFboSource(sequenceSource);
            secuencias.push_back(sequenceSource);
        }
    }
}
//--------------------------------------------------------------
void ofApp::loadSettings(){
    //-----------
    //the string is printed at the top of the app
    //to give the user some feedback
    xmlMessage = "loading mySettings.xml";

    //we load our settings file
    //if it doesn't exist we can still make one
    //by hitting the 's' key
    if( XML.load("mySettings.xml") ){
        xmlMessage = "mySettings.xml loaded!";
    }else{
        xmlMessage = "unable to load mySettings.xml check data/ folder";
    }
        
    //---------------- OSC --------------------
    oscPort = XML.getValue("OSC:PUERTO_OSC", 3333);

    ofLog(OF_LOG_NOTICE,xmlMessage);
}
//--------------------------------------------------------------
void ofApp::saveSettings(){

    XML.clear();

   
    //---------------- OSC --------------------
    XML.setValue("OSC:PUERTO_OSC", oscPort);
    
    XML.save("mySettings.xml");
    xmlMessage ="settings saved to xml!";
    ofLog(OF_LOG_NOTICE,xmlMessage);
}
