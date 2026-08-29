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
    if (!isPlaying || framePaths.empty()) {
        return;
    }

    const uint64_t currentTime = ofGetElapsedTimeMillis();
    const uint64_t elapsedTime = currentTime - lastUpdateTime;
    if (elapsedTime < frameDuration) {
        return;
    }

    const size_t framesToAdvance = static_cast<size_t>(elapsedTime / frameDuration);
    if (!isLooping && currentFrame + framesToAdvance >= framePaths.size()) {
        stop();
        return;
    }

    currentFrame = (currentFrame + framesToAdvance) % framePaths.size();
    lastUpdateTime += static_cast<uint64_t>(framesToAdvance * frameDuration);

    const size_t cacheBudget = std::max(
        CACHE_LOADS_PER_UPDATE,
        std::min(framesToAdvance, CACHE_AHEAD_FRAMES));
    refreshFrameCache(currentFrame, cacheBudget);
}

void Secuencia::draw() {
    ofClear(0, 0, 0, 255);
    if (currentFrame < framePaths.size() && loadFrame(currentFrame)) {
        frameCache[currentFrame].draw(0, 0, getWidth(), getHeight());
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

    std::vector<std::string> loadedPaths;
    loadedPaths.reserve(dir.size());
    for (const auto& file : dir) {
        loadedPaths.push_back(file.getAbsolutePath());
    }

    ofImage firstFrame;
    if (!firstFrame.load(loadedPaths.front())) {
        ofLogError("ImageSequenceSource") << "Failed to load first image: " << loadedPaths.front();
        return false;
    }

    clear();
    framePaths = std::move(loadedPaths);
    frameCache.emplace(0, std::move(firstFrame));
    currentFrame = 0;
    refreshFrameCache(currentFrame, INITIAL_CACHE_FRAMES - 1);
    return true;
}

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
    if (!framePaths.empty()) {
        ofLogNotice() << "------->PLAY secuencia: " + name;
        currentFrame = 0;
        refreshFrameCache(currentFrame, CACHE_LOADS_PER_UPDATE);
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
    if (!framePaths.empty()) {
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
    for (auto& cachedFrame : frameCache) {
        cachedFrame.second.clear();
    }
    frameCache.clear();
    failedFrames.clear();
    framePaths.clear();
    currentFrame = 0;
    soundPlayer.stop();
    soundPlayer.unload();
    audioTrack.clear();
    isPlaying = false;
    isPaused = false;
}

bool Secuencia::loadFrame(size_t frameIndex) {
    if (frameIndex >= framePaths.size()) {
        return false;
    }
    if (frameCache.count(frameIndex) != 0) {
        return true;
    }
    if (failedFrames.count(frameIndex) != 0) {
        return false;
    }

    ofImage frame;
    if (!frame.load(framePaths[frameIndex])) {
        failedFrames.insert(frameIndex);
        ofLogError("ImageSequenceSource") << "Failed to load image: " << framePaths[frameIndex];
        return false;
    }

    frameCache.emplace(frameIndex, std::move(frame));
    return true;
}

void Secuencia::refreshFrameCache(size_t centerFrame, size_t loadBudget) {
    if (framePaths.empty() || centerFrame >= framePaths.size()) {
        return;
    }

    std::unordered_set<size_t> wantedFrames;
    wantedFrames.insert(centerFrame);
    size_t loadedFrames = 0;

    if (frameCache.count(centerFrame) == 0 && loadFrame(centerFrame)) {
        ++loadedFrames;
    }

    for (size_t offset = 1; offset <= CACHE_AHEAD_FRAMES; ++offset) {
        const size_t frameIndex = getFrameIndexWithOffset(centerFrame, static_cast<int>(offset));
        if (frameIndex >= framePaths.size()) {
            break;
        }
        wantedFrames.insert(frameIndex);
        if (loadedFrames < loadBudget && frameCache.count(frameIndex) == 0 && loadFrame(frameIndex)) {
            ++loadedFrames;
        }
    }

    for (size_t offset = 1; offset <= CACHE_BEHIND_FRAMES; ++offset) {
        const size_t frameIndex = getFrameIndexWithOffset(centerFrame, -static_cast<int>(offset));
        if (frameIndex >= framePaths.size()) {
            break;
        }
        wantedFrames.insert(frameIndex);
        if (loadedFrames < loadBudget && frameCache.count(frameIndex) == 0 && loadFrame(frameIndex)) {
            ++loadedFrames;
        }
    }

    for (auto it = frameCache.begin(); it != frameCache.end();) {
        if (wantedFrames.count(it->first) == 0) {
            it->second.clear();
            it = frameCache.erase(it);
        } else {
            ++it;
        }
    }
}

size_t Secuencia::getFrameIndexWithOffset(size_t frameIndex, int offset) const {
    if (framePaths.empty()) {
        return 0;
    }

    const long long frameCount = static_cast<long long>(framePaths.size());
    long long candidate = static_cast<long long>(frameIndex) + offset;
    if (isLooping) {
        candidate = (candidate % frameCount + frameCount) % frameCount;
    } else if (candidate < 0 || candidate >= frameCount) {
        return framePaths.size();
    }
    return static_cast<size_t>(candidate);
}

size_t Secuencia::getFrameCount() const {
    return framePaths.size();
}

size_t Secuencia::getCachedFrameCount() const {
    return frameCache.size();
}

std::string Secuencia::getName() {
    return name;
}

void Secuencia::setName(std::string newName) {
    name = newName;
}
