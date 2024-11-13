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
    } else {
        ofBackground(0);
    }
    
    piMapper.draw();
    drawGui();
    ofSetWindowTitle("FPS: " + ofToString(ofGetFrameRate()));
}

void ofApp::keyPressed(int key) {
    if((key == 'e' || key == 'E') && (ofGetKeyPressed(OF_KEY_COMMAND) || ofGetKeyPressed(OF_KEY_CONTROL))) {
        editMode = !editMode;
        if(editMode){
            piMapper.setMode(ofx::piMapper::MAPPING_MODE);
        }else{
            piMapper.setMode(ofx::piMapper::PRESENTATION_MODE);
        }
    }
    
    if((key == 's' || key == 'S') && (ofGetKeyPressed(OF_KEY_COMMAND) || ofGetKeyPressed(OF_KEY_CONTROL))) {
        piMapper.saveProject();
        ofLogNotice() << "--------> PROYECTO GUARDADO";
    }
    
    if ((key == 'z' || key == 'Z') && (ofGetKeyPressed(OF_KEY_COMMAND) || ofGetKeyPressed(OF_KEY_CONTROL))) {
        piMapper.undo();
    }
    
    if ((key == 'f' || key == 'F') && (ofGetKeyPressed(OF_KEY_COMMAND) || ofGetKeyPressed(OF_KEY_CONTROL))) {
        ofToggleFullscreen();
    }
    if ((key == 'p' || key == 'P') && (ofGetKeyPressed(OF_KEY_COMMAND) || ofGetKeyPressed(OF_KEY_CONTROL))) {
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


/*
void ofApp::assignFileToSurface(const ofFile& file) {
     if (!piMapper.getSelectedSurface()) {
         ofLogError() << "No hay superficie seleccionada. No se puede asignar el archivo.";
         return; // No hacemos nada si no hay una superficie seleccionada
     }
 
    int selectedSurface = piMapper.getSelectedSurface();
    ofx::piMapper::BaseSource* selectedSource = piMapper.getSurfaceAt(selectedSurface)->getSource();
 
    std::string newFilePath;
    std::string fileExtension = file.getExtension();
    
    // Definir la carpeta de destino según el tipo de archivo
    if (isImageFile(fileExtension)) {
        newFilePath = std::string(DEFAULT_IMAGES_DIR) + "/" + file.getFileName();
        ofLogNotice() << "---------> Nueva IMAGEN: " << newFilePath;
        ofLogNotice() << "Copiando imagen a: " << newFilePath;
        file.copyTo(newFilePath, true);  // Copiar el archivo
        imageFiles.push_back(file.getFileName()); // Actualizar vector de imágenes
        ofLogNotice() << "--------->IMAGEN Cargada en el vector: " << file.getFileName();
 
        //piMapper.setup();
 
        piMapper.setImageSource(file.getFileName()); // Asignar a la superficie
        ofLogNotice() << "--------->IMAGEN en piMapper: " << selectedSource->getName();
 
    } else if (isAudioFile(fileExtension)) {
 
        if (selectedSource->getType() == ofx::piMapper::SourceType::SOURCE_TYPE_FBO) {
            if (selectedSource->getName() != "Syphon Source") {
                Secuencia * sec = dynamic_cast<Secuencia *>(selectedSource);
                newFilePath = std::string(DEFAULT_SOUNDS_DIR) + "/" + file.getFileName();
                ofLogNotice() << "Copiando sonido a: " << newFilePath;
                file.copyTo(newFilePath, true);  // Copiar el archivo
                audioFiles.push_back(file.getFileName()); // Actualizar vector de audio
                sec->setAudioTrack(file.getFileName()); // Asignar a la superficie (ejemplo si tienes esta función)
            }
        }
    } else if (isVideoFile(fileExtension)) {
        newFilePath = std::string(DEFAULT_VIDEOS_DIR) + "/" + file.getFileName();
        ofLogNotice() << "Copiando video a: " << newFilePath;
        file.copyTo(newFilePath, true);  // Copiar el archivo
        videoFiles.push_back(file.getFileName()); // Actualizar vector de videos
        piMapper.setVideoSource(newFilePath, true); // Asignar a la superficie con loop
        
    } else if (isSequenceFolder(file)) {
        newFilePath = std::string(DEFAULT_SEQUENCES_DIR) + "/" + file.getFileName();
        ofLogNotice() << "Copiando secuencia a: " << newFilePath;
        file.copyTo(newFilePath, true);  // Copiar la carpeta
        sequenceFolders.push_back(file.getFileName()); // Actualizar vector de secuencias
 
        // Aquí crearemos una nueva instancia de Secuencia y la asignaremos
        auto* sequenceSource = new Secuencia();
        sequenceSource->setup(file.getFileName());
        
        piMapper.registerFboSource(sequenceSource);
        secuencias.push_back(sequenceSource);
 
        //piMapper.setup();
 
        piMapper.setFboSource(file.getFileName()); // Asignar la secuencia a la superficie

    } else {
        ofLogError() << "Tipo de archivo no soportado.";
        }
    }
 
void ofApp::dragEvent(ofDragInfo dragInfo) {
    if (!editMode) {
        ofLogWarning() << "El modo edición no está activado. No se permite arrastrar y soltar.";
        return;
}
    
if (piMapper.getSelectedSurface() == -1) {
    ofLogError() << "No hay superficie seleccionada. No se puede asignar el archivo arrastrado.";
    return;
}
 
for (auto& filePath : dragInfo.files) {
    ofFile file(filePath);
    std::string fileExtension = file.getExtension();
 
    if (file.isFile()) {
        // Copiar el archivo según su tipo (imagen, video, audio)
        assignFileToSurface(file);
    }
    else if (file.isDirectory()) {
        // Si es una carpeta, verificar si es una carpeta de secuencias de PNG
        if (isSequenceFolder(file)) {
            assignFileToSurface(file);
        } else {
            ofLogError() << "Carpeta no válida. Solo se permiten carpetas con secuencias de PNG.";
        }
    } else {
        ofLogError() << "Tipo de archivo o carpeta no reconocido.";
        }
    }
}

// Verificar si el archivo es una imagen
bool ofApp::isImageFile(const std::string& fileExtension) {
    return fileExtension == "jpg" || fileExtension == "png" ||
    fileExtension == "bmp" || fileExtension == "gif";
}
 
// Verificar si el archivo es de audio
bool ofApp::isAudioFile(const std::string& fileExtension) {
    return fileExtension == "wav" || fileExtension == "mp3" ||
    fileExtension == "aiff" || fileExtension == "aif";
 }
 
// Verificar si el archivo es de video
bool ofApp::isVideoFile(const std::string& fileExtension) {
    return fileExtension == "mp4" || fileExtension == "mov" ||
    fileExtension == "avi";
}
 
 // Verificar si la carpeta contiene una secuencia de imágenes PNG
bool ofApp::isSequenceFolder(const ofFile& folder) {
    ofDirectory dir(folder.getAbsolutePath());
    dir.allowExt("png"); // Solo permitimos archivos PNG
    dir.listDir();
 
if (dir.size() > 0) {
    for (auto& file : dir.getFiles()) {
        if (file.isFile() && file.getExtension() == "png") {
            return true; // Si encontramos al menos un archivo PNG, consideramos que es una secuencia
            }
        }
    }

 return false; // Si no hay ningún archivo PNG, no es una secuencia válida
}
 
 */
