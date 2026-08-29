

#include "ofApp.h"


// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.txt)
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void ofApp::drawGui() {
    gui.begin();
    
    // Popup de ayuda (siempre disponible, se muestra según showHelpPopup)
        if (showHelpPopup) {
            ImGui::OpenPopup("Ayuda de BMapper");
            showHelpPopup = false; // Reseteamos el estado
        }
        
        if (ImGui::BeginPopupModal("Ayuda de BMapper", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text(
                        "MODOS\n\n"
                        
                        "Modo Edición (Control / Cmd + e): Cambia de modo para poder crear, editar superficies y definir su contenido.\n\n"
                        "CONTENIDOS\n"
                        "Para agregar contenidos, ubicarlos en la carpeta 'data/fuentes'.\n"
                        "Se pueden agregar imágenes (jpg, png, bmp, gif)\n"
                        "videos (mov, mp4, avi)\n"
                        "secuencias de PNG con transparencia\n"
                        
                        "sonidos (wav, mp3, aiff, aif).\n\n"
                        "ACCESO RÁPIDO DE TECLADO\n"
                        "Modo edición: ctrl / cmd + e\n"
                        "Guardar: ctrl / cmd + s\n"
                        "Deshacer: ctrl / cmd + z\n"
                        "Pantalla completa: ctrl / cmd + f\n"
                        "Modo presentación: ctrl / cmd + p\n"
                        "Modificar superficies:\n"
                        "ocultar o ver capas: l\n"
                        
                        
                        "Agrandar superficie seleccionada: +\n"
                        "Achicar superficie seleccionada: -\n"
                            
                        "Mover punto, o superficies: flecha o flechas + shift\n"
                               
                        "Seleccionar superficie siguiente:  .\n"

                        "Seleccionar superficie anterior:  ,\n"

                        "Seleccionar vértice siguiente:  <\n"

                        "Seleccionar vértice anterior:  >\n"
                                 
                        "Solo en modo EDICIÓN: \n"

                        "Crear superficie triangular: t\n"
                        "Crear superficie rectangular: q\n"
                        "Crear superficie circular: c\n"
                        "Crear superficie hexagonal: h\n"
                        "Crear superficie grilla: g\n"
                        
                        "Duplicar superficie: d\n"

                        "Borrar superficie: backspace\n\n"
                        
                        "CONTROL DE CONTENIDOS\n\n"
                        
                        "Tanto los videos como las secuencias de png se pueden ejecutar con los comandos: play, stop, pause y resume, tanto desde el menú de cada superficie como desde afuera a través de mensajes OSC.\n"
                        "También se pueden reproducir (play) a partir de una tecla del teclado.\n\n"
                        "OSC permite enviar mensajes a una superficie, usando etiquetas definidas para comandos como '/superficie1 play'. \n"
                        "Lo mismo puede hacerse con el resto de los controles: \n"
                        "/superficie1 stop\n"
                        "/superficie1 pause\n"
                        "/superficie1 resume \n"
                        
                        ); // Tu texto completo aquí
            
            if (ImGui::Button("Cerrar")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    
    // Mostrar menú principal solo en modo edición
    if (editMode && ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("|Aplicación|")) {
            if (ImGui::MenuItem("Guardar (cmd/ctrl + s)")) {
                saveProjectFiles();
            }
            if (ImGui::MenuItem("Actualizar recursos")) {
                refreshMediaLists();
            }
            ImGui::NewLine();
            if (ImGui::MenuItem("Undo ( cmd/ctrl + z)")) {
                piMapper.undo();
            }
            ImGui::NewLine();
            if (ImGui::MenuItem("Full Screen (cmd/ctrl + f)")) {
                ofToggleFullscreen();
            }
            ImGui::NewLine();
            if (ImGui::MenuItem("Modo Presentación (cmd/ctrl + p)")) {
                piMapper.setMode(ofx::piMapper::PRESENTATION_MODE);
                ofSetFullscreen(true);
                editMode = false;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("|Superficies|")) {
            
            if (ImGui::MenuItem("Panel de capas (l)")) {
                piMapper.toggleLayerPanel();
            }
            ImGui::SameLine(); HelpMarker("Ver u ocultar panel de capas");
            
            ImGui::NewLine();
            if (ImGui::BeginMenu("Agregar")) {
                if (ImGui::MenuItem("Cuadrada (q)")) {
                    piMapper.createSurface(ofx::piMapper::SurfaceType::QUAD_SURFACE);
                }
                ImGui::SameLine(); HelpMarker("Agregar superficie cuadrada");
                ImGui::NewLine();
                if (ImGui::MenuItem("Triangular (t)")) {
                    piMapper.createSurface(ofx::piMapper::SurfaceType::TRIANGLE_SURFACE);
                }
                ImGui::SameLine(); HelpMarker("Agregar superficie triangular");
                ImGui::NewLine();
                if (ImGui::MenuItem("Circular (c)")) {
                    piMapper.createSurface(ofx::piMapper::SurfaceType::CIRCLE_SURFACE);
                }
                ImGui::SameLine(); HelpMarker("Agregar superficie circular");
                ImGui::NewLine();
                if (ImGui::MenuItem("Grilla (g)")) {
                    piMapper.createSurface(ofx::piMapper::SurfaceType::GRID_WARP_SURFACE);
                }
                ImGui::SameLine(); HelpMarker("Agregar superficie tipo grilla, con varios nodos");
                
                ImGui::EndMenu();
            }
            
            ImGui::NewLine();
            if (ImGui::BeginMenu("Seleccionar")) {
                if (ImGui::MenuItem("Superficie anterior (,)")) {
                    piMapper.selectPrevSurface();
                }
                ImGui::NewLine();
                if (ImGui::MenuItem("Superficie siguiente (.)")) {
                    piMapper.selectNextSurface();
                }
                ImGui::NewLine();
                if (ImGui::MenuItem("Punto anteior (<)")) {
                    piMapper.selectPrevVertex();
                }
                ImGui::NewLine();
                if (ImGui::MenuItem("Punto siguiente (>)")) {
                    piMapper.selectNextVertex();
                }
                ImGui::EndMenu();
            }
        
            ImGui::NewLine();
            if (ImGui::MenuItem("Duplicar (d)")) {
                piMapper.duplicateSurface();
            }
            ImGui::SameLine(); HelpMarker("Duplicar superficie seleccionada");
            ImGui::NewLine();
            if (ImGui::MenuItem("Subir de capa")) {
                piMapper.moveLayerUp();
            }
            ImGui::SameLine(); HelpMarker("Mover capa seleccionada una capa hacia arriba");
            ImGui::NewLine();
            if (ImGui::MenuItem("Bajar de capa")) {
                piMapper.moveLayerDown();
            }
            ImGui::SameLine(); HelpMarker("Mover capa seleccionada una capa hacia abajo");
            ImGui::NewLine();
            if (ImGui::MenuItem("Agrandar (+)")) {
                piMapper.scaleUp();
            }
            ImGui::SameLine(); HelpMarker("Aumentar tamaño de superficie");
            ImGui::NewLine();
            if (ImGui::MenuItem("Reducir (-)")) {
                piMapper.scaleDown();
            }
            ImGui::SameLine(); HelpMarker("Reducir tamaño de superficie");
            ImGui::NewLine();
            if (ImGui::MenuItem("Borrar (delete)")) {
                piMapper.eraseSurface(piMapper.getSelectedSurface());
            }
            ImGui::SameLine(); HelpMarker("Borrar superficie seleccionada");
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("|OSC|")) {
            if(ImGui::InputInt("Puerto OSC", &oscPort)) {
                oscPort = ofClamp(oscPort, 1, 65535);
                receiver.setup(oscPort);
                setStatusMessage("OSC escuchando en el puerto " + ofToString(oscPort));
            }
            ImGui::SameLine(); HelpMarker("Definir puerto OSC de entrada (1 a 65535)");
        
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("|Acerca|"))
        {
            ImGui::Text("BMapper");
            ImGui::Separator();
            ImGui::Text("Software experimental para video mapping");
            ImGui::Text("Esta aplicación está en desarrollo y no tiene soporte");
            ImGui::Text("..............");
            ImGui::Text("Desarrollado por Matías Romero Costas (Biopus)");
            ImGui::Text("www.biopus.ar");

            ImGui::EndMenu();
        }
        
        
        // Nuevo menú de ayuda
        if (ImGui::BeginMenu("|Ayuda|"))
        {
            if (ImGui::MenuItem("Ver Ayuda")) {
                ImGui::OpenPopup("AyudaPopup");
                
                showHelpPopup = true;
                
                ofLogNotice() << "--------> Abriendo Ayuda"; // Confirmar que entra aquí
            }
            ImGui::EndMenu();
        }
            
        /*
        // Contenido del popup de ayuda
        if (ImGui::BeginPopupModal("AyudaPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ofLogNotice() << "--------> Popup Ayuda Abierto"; // Confirmar que entra aquí
            ImGui::Text(
                        "MODOS\n\n"
                        
                        "Modo Edición (Control / Cmd + e): Cambia de modo para poder crear, editar superficies y definir su contenido.\n\n"
                        "CONTENIDOS\n"
                        "Para agregar contenidos, ubicarlos en la carpeta 'data/fuentes'.\n"
                        "Se pueden agregar imágenes (jpg, png, bmp, gif)\n"
                        "videos (mov, mp4, avi)\n"
                        "secuencias de PNG con transparencia\n"
                        
                        "sonidos (wav, mp3, aiff, aif).\n\n"
                        "ACCESO RÁPIDO DE TECLADO\n"
                        "Modo edición: ctrl / cmd + e\n"
                        "Guardar: ctrl / cmd + s\n"
                        "Deshacer: ctrl / cmd + z\n"
                        "Pantalla completa: ctrl / cmd + f\n"
                        "Modo presentación: ctrl / cmd + p\n"
                        "Modificar superficies:\n"
                        "ocultar o ver capas: l\n"
                        
                        
                        "Agrandar superficie seleccionada: +\n"
                        "Achicar superficie seleccionada: -\n"
                            
                        "Mover punto, o superficies: flecha o flechas + shift\n"
                               
                        "Seleccionar superficie siguiente:  .\n"

                        "Seleccionar superficie anterior:  ,\n"

                        "Seleccionar vértice siguiente:  <\n"

                        "Seleccionar vértice anterior:  >\n"
                                 
                        "Solo en modo EDICIÓN: \n"

                        "Crear superficie triangular: t\n"
                        "Crear superficie rectangular: q\n"
                        "Crear superficie circular: c\n"
                        "Crear superficie hexagonal: h\n"
                        "Crear superficie grilla: g\n"
                        
                        "Duplicar superficie: d\n"

                        "Borrar superficie: backspace\n\n"
                        
                        "CONTROL DE CONTENIDOS\n\n"
                        
                        "Tanto los videos como las secuencias de png se pueden ejecutar con los comandos: play, stop, pause y resume, tanto desde el menú de cada superficie como desde afuera a través de mensajes OSC.\n"
                        "También se pueden reproducir (play) a partir de una tecla del teclado.\n\n"
                        "OSC permite enviar mensajes a una superficie, usando etiquetas definidas para comandos como '/superficie1 play'. \n"
                        "Lo mismo puede hacerse con el resto de los controles: \n"
                        "/superficie1 stop\n"
                        "/superficie1 pause\n"
                        "/superficie1 resume \n"
                         
                        );
            if (ImGui::Button("Cerrar")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
         
         */
    
        
        ImGui::EndMainMenuBar();
    }
    
    if (editMode) {
        
        ofx::piMapper::BaseSurface* surface = piMapper._application.getSurfaceManager()->getSelectedSurface();
        
        int selectedSurface = piMapper.getSelectedSurface();
        
        if (selectedSurface >= 0 && selectedSurface < piMapper.getNumSurfaces()) {
            ofx::piMapper::BaseSource* selectedSource = piMapper.getSurfaceAt(selectedSurface)->getSource();
            
            ImGui::SetNextWindowPos(ImVec2(10, 50), ImGuiCond_Once);
            ImGui::Begin(("Superficie " + std::to_string(selectedSurface) + " - " + selectedSource->getName()).c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize);

            ImGui::Text("ID: %d", selectedSurface);
            
            ImGui::NewLine();
            
            if (ImGui::BeginMenu("Seleccionar Contenido")) {
                if (ImGui::BeginMenu("Imagenes")) {
                    for (const auto& file : imageFiles) {
                        if (ImGui::MenuItem(file.c_str())) {
                            //config.source = file;
                            piMapper.setImageSource(file);
                        }
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Videos")) {
                    for (const auto& file : videoFiles) {
                        if (ImGui::MenuItem(file.c_str())) {
                            //onfig.source = file;
                            piMapper.setVideoSource(file, loop); // Configuración inicial de loop
                        }
                    }
                    ImGui::EndMenu();
                }
                                
                if (ImGui::BeginMenu("Secuencias")) {
                        for (const auto& folder : sequenceFolders) {
                
                            if (ImGui::MenuItem(folder.c_str())) {
                                piMapper.setFboSource(folder);
                            }
                        }
                            ImGui::EndMenu();
                        }

                if (ImGui::MenuItem("Syphon")) {
                    piMapper.setFboSource("Video server");
                }
                ImGui::EndMenu();
            }
            
            ImGui::NewLine();
            
            if (selectedSource->getType() == ofx::piMapper::SourceType::SOURCE_TYPE_FBO) {
                if (selectedSource->getName() == "Video server") {
                    #if defined(TARGET_OSX)
                    const auto serverLabels = videoServer.getServerLabels();
                    if (serverLabels.empty()) {
                        ImGui::TextDisabled("No hay servidores Syphon disponibles.");
                    } else {
                        int selectedServer = videoServer.getSelectedServerIndex();
                        if (selectedServer < 0 || selectedServer >= static_cast<int>(serverLabels.size())) {
                            selectedServer = 0;
                        }
                        if (ImGui::BeginCombo("Fuente Syphon", serverLabels[selectedServer].c_str())) {
                            for (int i = 0; i < static_cast<int>(serverLabels.size()); ++i) {
                                if (ImGui::Selectable(serverLabels[i].c_str(), i == selectedServer)) {
                                    videoServer.selectServer(i);
                                }
                            }
                            ImGui::EndCombo();
                        }
                    }
                    #endif
                } else {
                    if (auto* sec = dynamic_cast<Secuencia *>(selectedSource)) {
                        const std::string selectedAudio = sec->getAudioTrack();
                        const char* preview = selectedAudio.empty() ? "Ninguno" : selectedAudio.c_str();

                        if (ImGui::BeginCombo("Asociar audio", preview)) {
                            if (ImGui::Selectable("Ninguno", selectedAudio.empty())) {
                                sec->setAudioTrack("");
                            }

                            for (const auto& audioFile : audioFiles) {
                                if (ImGui::Selectable(audioFile.c_str(), selectedAudio == audioFile)) {
                                    sec->setAudioTrack(audioFile);
                                }
                            }

                            ImGui::EndCombo();
                        }
                    }
                }
            }
            
            // Controles de video para superficies de tipo video
            if ((selectedSource->getType() == ofx::piMapper::SourceType::SOURCE_TYPE_VIDEO ||
                 selectedSource->getType() == ofx::piMapper::SourceType::SOURCE_TYPE_FBO) &&
                 selectedSource->getName() != "Video server") { // Reemplaza con el tipo de video de tu librería
              ImGui::Separator();
              ImGui::Text("Controles de video");
                if (ImGui::Button("Play")) {
                    piMapper.playForSurface(selectedSurface);
                }
                ImGui::SameLine();
                if (ImGui::Button("Pause")) {
                    piMapper.pauseForSurface(selectedSurface);
                }
                ImGui::SameLine();
                if (ImGui::Button("Resume")) {
                    piMapper.resumeForSurface(selectedSurface);
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop")) {
                    piMapper.stopForSurface(selectedSurface);
                }
                
                
                if (selectedSource->getType() == ofx::piMapper::SourceType::SOURCE_TYPE_VIDEO){
                    
                    ImGui::NewLine();
                    // Realizar un cast dinámico a VideoSource
                    ofx::piMapper::VideoSource* videoSource = dynamic_cast<ofx::piMapper::VideoSource*>(selectedSource);
                    if (videoSource) {
                        bool isLooping = videoSource->getLoop();
                        if (ImGui::Checkbox("Loop", &isLooping)) videoSource->setLoop(isLooping);
                    }
                }
                
                if (selectedSource->getType() == ofx::piMapper::SourceType::SOURCE_TYPE_FBO){
                    
                    if(selectedSource->getName() != "Video server"){
                        Secuencia * sec = dynamic_cast<Secuencia *>(selectedSource);
                                                
                        ImGui::NewLine();
                       
                        if (sec) {
                            ImGui::TextDisabled("Secuencia: %d fotogramas · caché %d",
                                static_cast<int>(sec->getFrameCount()),
                                static_cast<int>(sec->getCachedFrameCount()));
                            bool isLooping = sec->getLoop();
                            if (ImGui::Checkbox("Loop", &isLooping)) sec->setLoop(isLooping);
                            
                            int speed = sec->getSpeed();
                            if (ImGui::DragInt("FPS", &speed, 1.0f, 1, 240)) {
                                sec->setSpeed(speed);
                            }
                            ImGui::SameLine(); HelpMarker("Velocidad, en cuadros por segundo para la animación");
                        }
                    }
                }
             
                ImGui::NewLine();
                
                // Mostrar controles de grilla solo si es del tipo GRID_WARP_SURFACE
               
                if (surface->getType() == ofx::piMapper::SurfaceType::GRID_WARP_SURFACE) {
                    
                    ImGui::Separator();
                    ImGui::Text("Tamaño de la grilla"); ImGui::SameLine(); HelpMarker("Agregar o quitar filas y columnas de la grilla. Nodos editables");
                    if (ImGui::Button("+ Columna")) {
                        piMapper.addGridColumn();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("- Columna")) {
                        piMapper.removeGridColumn();
                    }
                    if (ImGui::Button("+ Fila")) {
                        piMapper.addGridRow();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("- Fila")) {
                        piMapper.removeGridRow();
                    }
                }
                
                ImGui::Separator();
                
                ImGui::NewLine();
                
                char keyBuffer[2] = {surface->getAssignedKey(), '\0'};

                if (ImGui::InputText("Tecla", keyBuffer, 2, ImGuiInputTextFlags_CharsNoBlank)) {
                    // Lista de teclas reservadas (asegúrate de que todos son caracteres, no códigos de teclas)
                    std::vector<int> reservedKeys = {'l', '+', '-', ',', '.', '<', '>', 'q', 'g', 'h', 'd', 'c', 't'};
                    
                    // Agrega las teclas especiales a la lista si son necesarias
                    reservedKeys.push_back(OF_KEY_UP);
                    reservedKeys.push_back(OF_KEY_DOWN);
                    reservedKeys.push_back(OF_KEY_LEFT);
                    reservedKeys.push_back(OF_KEY_RIGHT);

                    bool isReserved = false;
                    for (int reservedKey : reservedKeys) {
                        if ((int)keyBuffer[0] == reservedKey) {
                            isReserved = true;
                            break;
                        }
                    }

                    if (isReserved) {
                        ImGui::OpenPopup("Tecla Reservada");
                    } else {
                        // Si no está reservada, asignar la tecla a la superficie
                        surface->setAssignedKey(keyBuffer[0]);
                    }
                }

                // Manejo del popup
                if (ImGui::BeginPopupModal("Tecla Reservada", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
                    ImGui::Text("Esta tecla está reservada para otras funciones.");
                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
                
                ImGui::SameLine();  HelpMarker("Tecla para ejecutar el 'play' del video o animación");
                
                // Crear un búfer temporal para almacenar el texto ingresado
                char oscAddressBuffer[256] = {};
                std::snprintf(oscAddressBuffer, sizeof(oscAddressBuffer), "%s", surface->getOscAddress().c_str());

                const bool oscAddressChanged = ImGui::InputText("Dirección OSC", oscAddressBuffer, sizeof(oscAddressBuffer), ImGuiInputTextFlags_CharsNoBlank);
                ImGui::SameLine(); HelpMarker("Definir una dirección (address) OSC. Debe comenzar con /");

                if (oscAddressChanged) {
                    std::string newOscAddress = oscAddressBuffer;
                    if (newOscAddress.empty() || newOscAddress.front() == '/') {
                        surface->setOscAddress(newOscAddress);
                    }
                }
                if (oscAddressBuffer[0] != '\0' && oscAddressBuffer[0] != '/') {
                    ImGui::TextDisabled("La dirección OSC debe comenzar con /.");
                }
            }
            
            ImGui::End();
        }

        const ImGuiWindowFlags statusWindowFlags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoInputs;
        ImGui::SetNextWindowPos(ImVec2(12.0f, ofGetHeight() - 12.0f), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
        ImGui::SetNextWindowBgAlpha(0.72f);
        if (ImGui::Begin("Estado operativo", nullptr, statusWindowFlags)) {
            ImGui::TextColored(ImVec4(0.25f, 0.9f, 0.65f, 1.0f), "BMapper · EDICIÓN");
            ImGui::Separator();
            ImGui::Text("FPS %.1f", ofGetFrameRate());
            ImGui::SameLine();
            ImGui::Text("OSC %d", oscPort);

            const int selectedSurface = piMapper.getSelectedSurface();
            if (selectedSurface >= 0 && selectedSurface < piMapper.getNumSurfaces()) {
                auto* selectedSource = piMapper.getSurfaceAt(selectedSurface)->getSource();
                if (selectedSource != nullptr) {
                    ImGui::Text("Superficie %d · %s", selectedSurface, selectedSource->getName().c_str());
                }
            } else {
                ImGui::TextDisabled("Sin superficie seleccionada");
            }

            if (assetWarnings.empty()) {
                ImGui::TextColored(ImVec4(0.25f, 0.9f, 0.65f, 1.0f), "Proyecto: recursos verificados");
            } else {
                ImGui::Separator();
                ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.35f, 1.0f), "Recursos no disponibles: %d", static_cast<int>(assetWarnings.size()));
                const int warningsToShow = std::min(4, static_cast<int>(assetWarnings.size()));
                for (int i = 0; i < warningsToShow; ++i) {
                    ImGui::BulletText("%s", assetWarnings[i].c_str());
                }
                if (static_cast<int>(assetWarnings.size()) > warningsToShow) {
                    ImGui::TextDisabled("Y %d aviso(s) más en el registro.", static_cast<int>(assetWarnings.size()) - warningsToShow);
                }
            }

            if (!statusMessage.empty() && ofGetElapsedTimeMillis() <= statusMessageExpiresAt) {
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.95f, 0.82f, 0.35f, 1.0f), "%s", statusMessage.c_str());
            }
            ImGui::End();
        }
    }
    
    gui.end();
}
