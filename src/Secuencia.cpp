#include "Secuencia.h"

Secuencia::Secuencia()
    : currentFrame(0), isPlaying(false), isLooping(false), speed(24), lastUpdateTime(0) {
    frameDuration = 1000.0f / speed;
}

Secuencia::~Secuencia() {
    clear();
}

void Secuencia::setup(std::string newName) {
    name = newName;
    // Allocate default size, this can be resized later based on the images
    allocate(800, 600);
}

void Secuencia::update() {
    if (isPlaying) {
        uint64_t currentTime = ofGetElapsedTimeMillis();
        if (currentTime - lastUpdateTime >= frameDuration) {
            currentFrame = (currentFrame + 1) % images.size();
            lastUpdateTime = currentTime;

            if (currentFrame == 0 && !isLooping) {
                stop();
            }
        }
    }
}

void Secuencia::draw() {
    ofBackground(0,0);
    if (!images.empty() && currentFrame < images.size()) {
        images[currentFrame].draw(0, 0, getWidth(), getHeight());
    }
}

bool Secuencia::loadSequence(const std::string& folderPath) {
    ofDirectory dir(folderPath);
    if (!dir.exists()) {
        ofLogError("ImageSequenceSource") << "Directory does not exist: " << folderPath;
        return false;
    }

    dir.allowExt("png");
    dir.listDir();
    dir.sort();
    if (dir.size() == 0) {
        ofLogError("ImageSequenceSource") << "No images found in directory: " << folderPath;
        return false;
    }

    clear();  // Clear any previously loaded images

    images.reserve(dir.size()); // Reservar espacio para las imágenes
    
    for (const auto& file : dir) {
        ofImage img;
        if (img.load(file.getAbsolutePath())) {
            images.push_back(img); // Almacenar ofImage en lugar de ofTexture
        } else {
            ofLogError("ImageSequenceSource") << "Failed to load image: " << file.getAbsolutePath();
            return false;
        }
    }

    if (images.empty()) {
        ofLogError("ImageSequenceSource") << "No images loaded from directory: " << folderPath;
        return false;
    }

    currentFrame = 0;
    return true;
}

/*
bool Secuencia::loadSequence(const std::string& folderPath) {
    ofDirectory dir(folderPath);
    if (!dir.exists()) {
        ofLogError("ImageSequenceSource") << "Directory does not exist: " << folderPath;
        return false;
    }

    dir.allowExt("png");
    dir.listDir();
    dir.sort();
    if (dir.size() == 0) {
        ofLogError("ImageSequenceSource") << "No images found in directory: " << folderPath;
        return false;
    }

    clear();  // Clear any previously loaded images

   ofImage cargador;
    
    for (const auto& file : dir) {
        cargador.load(file.getAbsolutePath());
        ofTexture texture;
        texture.allocate(cargador.getWidth(), cargador.getHeight(), GL_RGBA);
        texture.loadData(cargador.getPixels(), GL_RGBA);
        
        ofLoadImage(texture, file.getAbsolutePath());
        images.push_back(texture);
       // ofLogNotice() << "-------->Cargando imagen en secuencia  ";
    }

    if (images.empty()) {
        ofLogError("ImageSequenceSource") << "No images loaded from directory: " << folderPath;
        return false;
    }

    currentFrame = 0;
    //allocate(images[0].getWidth(), images[0].getHeight());  // Resize FBO to match image size
    return true;
}
 
 */

bool Secuencia::setAudioTrack(const std::string& audioPath) {
    audioTrack = audioPath;
    if (!soundPlayer.load("sources/sonidos/" + audioTrack)) {
        ofLogError("ImageSequenceSource") << "Failed to load audio track: " << audioPath;
        return false;
    }
    soundPlayer.setLoop(isLooping);
    return true;
}

std::string Secuencia::getAudioTrack(){
    return audioTrack;
}

void Secuencia::play() {
    
    if (!images.empty()) {
        ofLogNotice() << "------->PLAY secuencia: " + name;
        isPlaying = true;
        soundPlayer.play();
        lastUpdateTime = ofGetElapsedTimeMillis();
    }
}

void Secuencia::stop() {
    ofLogNotice() << "------->STOP secuencia: " + name;
    isPlaying = false;
    soundPlayer.stop();
    currentFrame = 0;
}

void Secuencia::pause() {
    ofLogNotice() << "------->PAUSE secuencia: " + name;
    isPlaying = false;
    soundPlayer.setPaused(true);
}

void Secuencia::resume() {
    ofLogNotice() << "------->RESUME secuencia: " + name;
    if (!images.empty()) {
        isPlaying = true;
        soundPlayer.setPaused(false);
        lastUpdateTime = ofGetElapsedTimeMillis();
    }
}

void Secuencia::setLoop(bool loop) {
    isLooping = loop;
    soundPlayer.setLoop(loop);
}

bool Secuencia::getLoop() {
    return isLooping;
}

void Secuencia::setSpeed(int fps) {
    speed = fps;
    frameDuration = 1000.0f / speed;
}

int Secuencia::getSpeed(){
    return speed;
}

void Secuencia::clear() {
    
    images.clear();
    currentFrame = 0;
    soundPlayer.stop();
    isPlaying = false;
}

std::string Secuencia::getName() {
    return name;
}

void Secuencia::setName(std::string newName) {
    name = newName;
}


