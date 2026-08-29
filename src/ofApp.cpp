#include "ofApp.h"

#include <algorithm>

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
    piMapper.setAutoSaveEnabled(false);
    lastProjectAutosaveTime = ofGetElapsedTimeMillis();
    auditProjectAssets();
    if (assetWarnings.empty()) {
        setStatusMessage("Proyecto verificado");
    } else {
        setStatusMessage("Atención: " + ofToString(assetWarnings.size()) + " recurso(s) no disponible(s)");
    }
    
    //----------------- OSC -------------------
    receiver.setup(oscPort);
}

void ofApp::update(){
    piMapper.update();

    const uint64_t now = ofGetElapsedTimeMillis();
    if (now - lastAssetAuditTime >= 1000) {
        auditProjectAssets();
    }

    if (editMode && now - lastProjectAutosaveTime >= PROJECT_AUTOSAVE_INTERVAL_MS) {
        saveProjectFiles(true);
    }

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
    const bool commandShortcut = ofGetKeyPressed(OF_KEY_COMMAND) || ofGetKeyPressed(OF_KEY_CONTROL);
    const bool controlShortcut = ofGetKeyPressed(OF_KEY_CONTROL) && !ofGetKeyPressed(OF_KEY_COMMAND);

    const bool editShortcut = key == 'e' || key == 'E' || (controlShortcut && key == 5);
    const bool saveShortcut = key == 's' || key == 'S' || (controlShortcut && key == 19);
    const bool undoShortcut = key == 'z' || key == 'Z' || (controlShortcut && key == 26);
    const bool fullscreenShortcut = key == 'f' || key == 'F' || (controlShortcut && key == 6);
    const bool presentationShortcut = key == 'p' || key == 'P' || (controlShortcut && key == 16);

    if(editShortcut && commandShortcut) {
        editMode = !editMode;
        if(editMode){
            piMapper.setMode(ofx::piMapper::MAPPING_MODE);
            lastProjectAutosaveTime = ofGetElapsedTimeMillis();
            setStatusMessage("Modo edición activo");
        }else{
            piMapper.setMode(ofx::piMapper::PRESENTATION_MODE);
            setStatusMessage("Modo presentación activo: edición bloqueada");
        }
    }
    
    if(saveShortcut && commandShortcut) {
        saveProjectFiles();
    }
    
    if (editMode && undoShortcut && commandShortcut) {
        piMapper.undo();
    }
    
    if (fullscreenShortcut && commandShortcut) {
        ofToggleFullscreen();
    }
    if (presentationShortcut && commandShortcut) {
        piMapper.setMode(ofx::piMapper::PRESENTATION_MODE);
        ofSetFullscreen(true);
        editMode = false;
        setStatusMessage("Modo presentación activo: edición bloqueada");
    }

    if (editMode && key == OF_KEY_F1) {
        showHelpPopup = true;
    }

    if (editMode && ImGui::GetIO().WantCaptureKeyboard) {
        return;
    }
    
    if (editMode) {
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
                requestDeleteSelectedSurface();
                break;
        }
    } else if (!editMode) {
        
    }
    
    if (!commandShortcut) {
        // Ejecutar 'play' para VideoSurface y FboSource con tecla asignada.
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
}

void ofApp::keyReleased(int key) {
    piMapper.keyReleased(key);
}

void ofApp::requestDeleteSelectedSurface() {
    const int selectedSurface = piMapper.getSelectedSurface();
    if (selectedSurface < 0 || selectedSurface >= piMapper.getNumSurfaces()) {
        setStatusMessage("No hay una superficie seleccionada para borrar");
        return;
    }

    pendingSurfaceDeletion = selectedSurface;
    showDeleteSurfaceConfirmation = true;
}

void ofApp::mouseDragged(int x, int y, int button) {
    if (editMode && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsAnyItemHovered()) {
        piMapper.mouseDragged(x, y, button);
    }
}

void ofApp::mousePressed(int x, int y, int button) {
    if (editMode && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsAnyItemHovered()) {
        piMapper.mousePressed(x, y, button);
    }
}

void ofApp::mouseReleased(int x, int y, int button) {
    if (editMode && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsAnyItemHovered()) {
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
    ofx::piMapper::VideoSource::enableAudio = true;
    ofx::piMapper::VideoSource::useHDMIForAudio = false;

    // Register sources before mapper.setup().
    piMapper.registerFboSource(videoServer);
    refreshMediaLists();
}

void ofApp::refreshMediaLists() {
    imageFiles.clear();
    ofDirectory imageDirectory(DEFAULT_IMAGES_DIR);
    imageDirectory.allowExt("jpg");
    imageDirectory.allowExt("png");
    imageDirectory.allowExt("bmp");
    imageDirectory.allowExt("gif");
    imageDirectory.listDir();
    imageDirectory.sort();
    for(const auto& file : imageDirectory.getFiles()){
        imageFiles.push_back(file.getFileName());
    }
    
    videoFiles.clear();
    ofDirectory videoDirectory(DEFAULT_VIDEOS_DIR);
    videoDirectory.allowExt("mp4");
    videoDirectory.allowExt("mov");
    videoDirectory.allowExt("avi");
    videoDirectory.listDir();
    videoDirectory.sort();
    for(const auto& file : videoDirectory.getFiles()){
        videoFiles.push_back(file.getFileName());
    }
    
    audioFiles.clear();
    ofDirectory audioDirectory(DEFAULT_SOUNDS_DIR);
    audioDirectory.allowExt("wav");
    audioDirectory.allowExt("aiff");
    audioDirectory.allowExt("aif");
    audioDirectory.allowExt("mp3");
    audioDirectory.listDir();
    audioDirectory.sort();
    for (const auto& file : audioDirectory.getFiles()) {
        audioFiles.push_back(file.getFileName());
    }

    ofDirectory sequenceDirectory(DEFAULT_SEQUENCES_DIR);
    sequenceDirectory.listDir();
    sequenceDirectory.sort();
    std::vector<std::string> availableSequenceFolders;
    for (const auto& folder : sequenceDirectory.getFiles()) {
        if (folder.isDirectory()) {
            std::string folderName = folder.getFileName();
            const bool isAlreadyRegistered = std::any_of(
                secuencias.begin(), secuencias.end(),
                [&folderName](Secuencia* sequence) {
                    return sequence != nullptr && sequence->getName() == folderName;
                });
            if (isAlreadyRegistered) {
                availableSequenceFolders.push_back(folderName);
                continue;
            }

            auto* sequenceSource = new Secuencia();
            sequenceSource->setup(folderName);
            if(!sequenceSource->loadSequence(std::string(DEFAULT_SEQUENCES_DIR) + folderName)) {
                ofLogWarning() << "No se pudieron cargar imágenes en la secuencia " << folderName;
                delete sequenceSource;
                continue;
            }
            piMapper.registerFboSource(sequenceSource);
            secuencias.push_back(sequenceSource);
            availableSequenceFolders.push_back(folderName);
        }
    }
    sequenceFolders = std::move(availableSequenceFolders);

    auditProjectAssets();
    setStatusMessage(
        "Recursos actualizados: " + ofToString(imageFiles.size()) + " imágenes, " +
        ofToString(videoFiles.size()) + " videos, " +
        ofToString(audioFiles.size()) + " audios y " +
        ofToString(sequenceFolders.size()) + " secuencias");
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
    setStatusMessage("Proyecto guardado");
    ofLog(OF_LOG_NOTICE,xmlMessage);
}

void ofApp::saveProjectFiles(bool automatic) {
    auditProjectAssets();
    if (!assetWarnings.empty()) {
        setStatusMessage(automatic
            ? "Autoguardado bloqueado: restaurá los recursos faltantes primero"
            : "Guardado bloqueado: restaurá los recursos faltantes primero");
        ofLogWarning("BMapper") << "Project save skipped because resources are missing";
        lastProjectAutosaveTime = ofGetElapsedTimeMillis();
        return;
    }

    if (!createProjectBackup()) {
        setStatusMessage(automatic
            ? "Autoguardado cancelado: no se pudo crear un respaldo seguro"
            : "Guardado cancelado: no se pudo crear un respaldo seguro");
        ofLogError("BMapper") << "Project save skipped because backup creation failed";
        lastProjectAutosaveTime = ofGetElapsedTimeMillis();
        return;
    }

    piMapper.saveProject();
    saveSettings();
    lastProjectAutosaveTime = ofGetElapsedTimeMillis();
    if (automatic) {
        setStatusMessage("Autoguardado realizado");
    }
    ofLogNotice() << "--------> PROYECTO GUARDADO";
}

bool ofApp::createProjectBackup() {
    const std::vector<std::string> projectFiles = {
        "ofxpimapper.xml",
        "mySettings.xml"
    };

    bool anyProjectFileExists = false;
    bool allProjectFilesExist = true;
    for (const auto& fileName : projectFiles) {
        const bool exists = ofFile::doesFileExist(fileName);
        anyProjectFileExists = anyProjectFileExists || exists;
        allProjectFilesExist = allProjectFilesExist && exists;
    }

    // The first save of a new project has nothing to preserve yet.
    if (!anyProjectFileExists) {
        return true;
    }

    // Never overwrite a partial project without a complete recovery point.
    if (!allProjectFilesExist) {
        ofLogError("BMapper") << "Backup skipped because one project settings file is missing";
        return false;
    }

    const std::string backupPath = ".backups/" + ofGetTimestampString();
    ofDirectory backupDirectory(backupPath);
    if (!backupDirectory.create(true)) {
        ofLogError("BMapper") << "Could not create backup directory: " << backupPath;
        return false;
    }

    for (const auto& fileName : projectFiles) {
        if (!ofFile::copyFromTo(fileName, backupPath + "/" + fileName)) {
            ofLogError("BMapper") << "Could not copy project file to backup: " << fileName;
            backupDirectory.remove(true);
            return false;
        }
    }

    pruneProjectBackups();
    ofLogNotice("BMapper") << "Project backup created: " << backupPath;
    return true;
}

void ofApp::pruneProjectBackups() {
    ofDirectory backupRoot(".backups");
    backupRoot.listDir();
    backupRoot.sort();

    std::vector<std::string> backupPaths;
    for (std::size_t i = 0; i < backupRoot.size(); ++i) {
        const ofFile backup = backupRoot.getFile(i);
        if (backup.isDirectory()) {
            backupPaths.push_back(backup.getAbsolutePath());
        }
    }

    const std::size_t backupsToRemove = backupPaths.size() > MAX_PROJECT_BACKUPS
        ? backupPaths.size() - MAX_PROJECT_BACKUPS
        : 0;

    for (std::size_t i = 0; i < backupsToRemove; ++i) {
        ofDirectory oldBackup(backupPaths[i]);
        if (!oldBackup.remove(true)) {
            ofLogWarning("BMapper") << "Could not remove old backup: " << backupPaths[i];
        }
    }
}

void ofApp::auditProjectAssets() {
    assetWarnings.clear();
    lastAssetAuditTime = ofGetElapsedTimeMillis();

    ofxXmlSettings projectXml;
    if (!projectXml.load("ofxpimapper.xml")) {
        assetWarnings.push_back("No se pudo leer ofxpimapper.xml");
        return;
    }

    const int presetCount = projectXml.getNumTags("surfaces");
    for (int presetIndex = 0; presetIndex < presetCount; ++presetIndex) {
        projectXml.pushTag("surfaces", presetIndex);
        const int surfaceCount = projectXml.getNumTags("surface");
        for (int surfaceIndex = 0; surfaceIndex < surfaceCount; ++surfaceIndex) {
            projectXml.pushTag("surface", surfaceIndex);
            if (projectXml.tagExists("source", 0)) {
                projectXml.pushTag("source");
                const std::string type = projectXml.getValue("source-type", "");
                const std::string name = projectXml.getValue("source-name", "");
                const std::string sound = projectXml.getValue("source-sound", "");
                const std::string surfaceLabel = "superficie " + ofToString(surfaceIndex + 1);

                if (type == "image" && !name.empty() && name != "none" &&
                    !ofFile::doesFileExist(std::string(DEFAULT_IMAGES_DIR) + name)) {
                    assetWarnings.push_back("Imagen no disponible: " + name + " (" + surfaceLabel + ")");
                } else if (type == "video" && !name.empty() && name != "none" &&
                           !ofFile::doesFileExist(std::string(DEFAULT_VIDEOS_DIR) + name)) {
                    assetWarnings.push_back("Video no disponible: " + name + " (" + surfaceLabel + ")");
                } else if (type == "fbo" && !name.empty() && name != "none" && name != "Video server") {
                    ofDirectory sequenceDirectory(std::string(DEFAULT_SEQUENCES_DIR) + name);
                    if (!sequenceDirectory.exists()) {
                        assetWarnings.push_back("Secuencia no disponible: " + name + " (" + surfaceLabel + ")");
                    } else {
                        sequenceDirectory.allowExt("png");
                        sequenceDirectory.listDir();
                        if (sequenceDirectory.size() == 0) {
                            assetWarnings.push_back("Secuencia sin imágenes PNG: " + name + " (" + surfaceLabel + ")");
                        }
                    }
                } else if (type != "" && type != "none" && type != "image" && type != "video" && type != "fbo") {
                    assetWarnings.push_back("Tipo de fuente no válido: " + type + " (" + surfaceLabel + ")");
                }

                if (!sound.empty() && !ofFile::doesFileExist(std::string(DEFAULT_SOUNDS_DIR) + sound)) {
                    assetWarnings.push_back("Audio no disponible: " + sound + " (" + surfaceLabel + ")");
                }
                projectXml.popTag();
            }
            projectXml.popTag();
        }
        projectXml.popTag();
    }
}

void ofApp::setStatusMessage(const std::string& message) {
    statusMessage = message;
    statusMessageExpiresAt = ofGetElapsedTimeMillis() + 4000;
}
