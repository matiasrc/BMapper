#include "Secuencia.h"

#include <algorithm>

Secuencia::Secuencia()
    : currentFrame(0), isPlaying(false), isPaused(false), isLooping(false), speed(24), lastUpdateTime(0) {
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
    if (!isPlaying || images.empty()) {
        return;
    }

    const uint64_t currentTime = ofGetElapsedTimeMillis();
    const uint64_t elapsedTime = currentTime - lastUpdateTime;
    if (elapsedTime < frameDuration) {
        return;
    }

    const size_t framesToAdvance = static_cast<size_t>(elapsedTime / frameDuration);
    if (!isLooping && currentFrame + framesToAdvance >= images.size()) {
        stop();
        return;
    }

    currentFrame = (currentFrame + framesToAdvance) % images.size();
    lastUpdateTime += static_cast<uint64_t>(framesToAdvance * frameDuration);
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

    std::vector<ofImage> loadedImages;
    loadedImages.reserve(dir.size());
    
    for (const auto& file : dir) {
        ofImage img;
        if (img.load(file.getAbsolutePath())) {
            loadedImages.push_back(std::move(img));
        } else {
            ofLogError("ImageSequenceSource") << "Failed to load image: " << file.getAbsolutePath();
            return false;
        }
    }

    if (loadedImages.empty()) {
        ofLogError("ImageSequenceSource") << "No images loaded from directory: " << folderPath;
        return false;
    }

    clear();
    images = std::move(loadedImages);
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
    if (audioPath == audioTrack) {
        return true;
    }

    const bool wasPlaying = isPlaying;
    const bool wasPaused = isPaused;
    soundPlayer.stop();
    soundPlayer.unload();
    audioTrack.clear();

    if (audioPath.empty()) {
        return true;
    }

    if (!soundPlayer.load("sources/sonidos/" + audioPath)) {
        ofLogError("ImageSequenceSource") << "Failed to load audio track: " << audioPath;
        return false;
    }

    audioTrack = audioPath;
    soundPlayer.setLoop(isLooping);
    if (wasPlaying) {
        soundPlayer.play();
    } else if (wasPaused) {
        soundPlayer.play();
        soundPlayer.setPaused(true);
    }
    return true;
}

std::string Secuencia::getAudioTrack(){
    return audioTrack;
}

void Secuencia::play() {
    if (!images.empty()) {
        ofLogNotice() << "------->PLAY secuencia: " + name;
        currentFrame = 0;
        isPlaying = true;
        isPaused = false;
        if (soundPlayer.isLoaded()) {
            soundPlayer.stop();
            soundPlayer.setPosition(0.0f);
            soundPlayer.play();
        }
        lastUpdateTime = ofGetElapsedTimeMillis();
    }
}

void Secuencia::stop() {
    ofLogNotice() << "------->STOP secuencia: " + name;
    isPlaying = false;
    isPaused = false;
    if (soundPlayer.isLoaded()) {
        soundPlayer.stop();
        soundPlayer.setPosition(0.0f);
    }
    currentFrame = 0;
}

void Secuencia::pause() {
    ofLogNotice() << "------->PAUSE secuencia: " + name;
    if (!isPlaying) {
        return;
    }
    isPlaying = false;
    isPaused = true;
    if (soundPlayer.isLoaded()) {
        soundPlayer.setPaused(true);
    }
}

void Secuencia::resume() {
    ofLogNotice() << "------->RESUME secuencia: " + name;
    if (!images.empty()) {
        const bool resumePausedPlayback = isPaused;
        isPlaying = true;
        isPaused = false;
        if (soundPlayer.isLoaded()) {
            if (resumePausedPlayback) {
                soundPlayer.setPaused(false);
            } else {
                soundPlayer.stop();
                soundPlayer.setPosition(0.0f);
                soundPlayer.play();
            }
        }
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
    speed = std::max(1, fps);
    frameDuration = 1000.0f / speed;
}

int Secuencia::getSpeed(){
    return speed;
}

void Secuencia::clear() {
    
    images.clear();
    currentFrame = 0;
    soundPlayer.stop();
    soundPlayer.unload();
    audioTrack.clear();
    isPlaying = false;
    isPaused = false;
}

std::string Secuencia::getName() {
    return name;
}

void Secuencia::setName(std::string newName) {
    name = newName;
}
