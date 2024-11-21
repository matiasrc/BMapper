#include "ofApp.h"

void ofApp::setup() {
    ofBackground(0, 255);
    
    //----------------- GUI -------------------
    //required call
    gui.setup();
    
    ImGui::GetIO().MouseDrawCursor = false;
    
    loadData();
    
    piMapper.setup();
    
    //----------------- OSC -------------------
    receiver.setup(oscPort);
}

void ofApp::update(){
    piMapper.update();
    for (auto s : secuencias) {
        s->update();
    }
    while (receiver.hasWaitingMessages()) {
        ofxOscMessage m;
        receiver.getNextMessage(m);
        processOscMessage(m);
    }
}

void ofApp::draw(){
    if (editMode) {
        ofPushStyle();
        ofBackground(100);
        ofSetColor(80);
        
        // Tamaño del texto
        string text = "MODO EDICIÓN";
        int textWidth = 200; // Ajusta esto según el tamaño del texto
        int textHeight = 50; // Ajusta esto según el tamaño del texto
        // Configurar la fuente y el tamaño
        ofTrueTypeFont font;
        font.load("verdana.ttf", 12); // Asegúrate de que esta fuente esté disponible
        
        int screenWidth = ofGetWidth();
        int screenHeight = ofGetHeight();
        
        // Dibujar el texto en un patrón de mosaico
        for (int y = 0; y < screenHeight; y += textHeight) {
            for (int x = 0; x < screenWidth; x += textWidth) {
                font.drawString(text, x, y);
            }
        }
        
        ofPopStyle();
    } else {
        ofBackground(0);
        ofSetColor(255);
    }
    
    piMapper.draw();
    drawGui();
    ofSetWindowTitle("FPS: " + ofToString(ofGetFrameRate()));
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
    //piMapper.keyReleased(key);
}

void ofApp::mouseDragged(int x, int y, int button) {
    if (!ImGui::IsAnyWindowHovered() && !ImGui::IsAnyItemHovered()) {
        piMapper.mouseDragged(x, y, button);
    }
}

void ofApp::mousePressed(int x, int y, int button) {
    if (!ImGui::IsAnyWindowHovered() && !ImGui::IsAnyItemHovered()) {
        piMapper.mousePressed(x, y, button);
    }
}

void ofApp::mouseReleased(int x, int y, int button) {
    if (!ImGui::IsAnyWindowHovered() && !ImGui::IsAnyItemHovered()) {
        piMapper.mouseReleased(x, y, button);
    }
}


void ofApp::processOscMessage(const ofxOscMessage& message) {
  
    std::string address = message.getAddress();

    //ofLogNotice() << "-------->Entró nuevo mensaje OSC: " << message;
    
    // Obtener el administrador de superficies
    ofx::piMapper::SurfaceManager* surfaceManager = piMapper._application.getSurfaceManager();

    // Iterar sobre todas las superficies
    for (int i = 0; i < piMapper.getNumSurfaces(); ++i) {
      
    ofx::piMapper::BaseSurface* surface = piMapper.getSurfaceAt(i);
      
    if (surface->getOscAddress() == address) {
      // La dirección OSC coincide con la superficie
      if (message.getNumArgs() > 0 && message.getArgType(0) == OFXOSC_TYPE_STRING) {
          
        std::string command = message.getArgAsString(0);
          
          if (surface->getSource()->getType() == ofx::piMapper::SourceType::SOURCE_TYPE_VIDEO ||
              surface->getSource()->getType() == ofx::piMapper::SourceType::SOURCE_TYPE_FBO) {
              
              ofLogNotice() << "-------->furface: " + ofToString(i) + " " + surface->getOscAddress() + " | mensaje OSC: " + address;
              // Verificar si el comando es válido y ejecutar la acción correspondiente
              if (command == "play") {
                piMapper.playForSurface(i);
              } else if (command == "pause") {
                  piMapper.pauseForSurface(i);
              } else if (command == "stop") {
                  piMapper.stopForSurface(i);
              }else if (command == "resume") {
                  piMapper.resumeForSurface(i);
              }
              else {
                ofLogError() << "Comando OSC no reconocido: " << command;
              }
          }
      } else {
        ofLogError() << "Mensaje OSC no contiene el argumento de comando esperado.";
      }
    }
  }
}

void ofApp::loadData() {
    // Configuración inicial
    ofDirectory dir;
    
    // Listar archivos de imagen
    dir.allowExt("jpg");
    dir.allowExt("png");
    dir.allowExt("bmp");
    dir.allowExt("gif");
    dir.listDir(DEFAULT_IMAGES_DIR); // Carpeta de imágenes en la carpeta data
    for(auto file : dir.getFiles()){
        ofLogNotice() << "Imagen encontrada: " << file.getFileName(); // Mensaje de depuración
        imageFiles.push_back(file.getFileName());
    }
    
    // Listar archivos de video
    dir.allowExt("mp4");
    dir.allowExt("mov");
    dir.allowExt("avi");
    dir.listDir(DEFAULT_VIDEOS_DIR); // Carpeta de videos en la carpeta data
    for(auto file : dir.getFiles()){
        ofLogNotice() << "Video encontrado: " << file.getFileName(); // Mensaje de depuración
        videoFiles.push_back(file.getFileName());
    }
    
    // Listar archivos de audio
    dir.allowExt("wav");
    dir.allowExt("aiff");
    dir.allowExt("aif");
    dir.allowExt("mp3");
    dir.listDir(DEFAULT_SOUNDS_DIR); // Carpeta de sonidos en la carpeta data
    for (auto file : dir.getFiles()) {
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
    piMapper.registerFboSource(SyphonClient);
        
    // Aquí crearemos una nueva instancia de Secuencia y la asignaremos

    dir.listDir("sources/secuencias"); // Carpeta de secuencias en la carpeta data
    dir.allowExt(""); // Permitir todas las extensiones para carpetas
    for (auto folder : dir.getFiles()) {
        if (folder.isDirectory()) {
            std::string folderName = folder.getFileName();
            ofLogNotice() << "----------->Secuencia encontrada: " << folderName; // Mensaje de depuración
            sequenceFolders.push_back(folder.getFileName());
        
            // Aquí crearemos una nueva instancia de Secuencia y la asignaremos
            auto* sequenceSource = new Secuencia();
            sequenceSource->setup(folderName);
            std::string nombre = sequenceSource->getName();
            //ofLogNotice()<< "----------->nombre de fuente: " + nombre;
            if(!sequenceSource->loadSequence("sources/secuencias/" + nombre))  ofLogNotice()<< "----------->no se pudieron cagar imagenes en la secuencia" + nombre;;
            
            piMapper.registerFboSource(sequenceSource);
            secuencias.push_back(sequenceSource);
        }
    }
}
