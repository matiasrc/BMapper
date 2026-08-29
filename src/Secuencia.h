#pragma once

#include "ofMain.h"
#include "FboSource.h"
#include "ofSoundPlayer.h"

#include <unordered_map>
#include <unordered_set>

class Secuencia : public ofx::piMapper::FboSource {
public:
    using ofx::piMapper::FboSource::setup;

    Secuencia();
    ~Secuencia() override;

    void setup(std::string newName);
    void update() override;
    void draw() override;

    bool loadSequence(const std::string& folderPath);
    bool setAudioTrack(const std::string& audioPath) override;
    std::string getAudioTrack() override;
    void play() override;
    void stop() override;
    void pause() override;
    void resume() override;

    void setLoop(bool loop) override;
    bool getLoop() override;

    void setSpeed(int fps) override;
    int getSpeed() override;

    size_t getFrameCount() const;
    size_t getCachedFrameCount() const;

    std::string getName();
    void setName(std::string newName);

private:
    static constexpr size_t INITIAL_CACHE_FRAMES = 8;
    static constexpr size_t CACHE_AHEAD_FRAMES = 12;
    static constexpr size_t CACHE_BEHIND_FRAMES = 3;
    static constexpr size_t CACHE_LOADS_PER_UPDATE = 1;

    std::vector<std::string> framePaths;
    std::unordered_map<size_t, ofImage> frameCache;
    std::unordered_set<size_t> failedFrames;
    ofSoundPlayer soundPlayer;
    int currentFrame;
    float frameDuration; // Duration of each frame in milliseconds
    bool isPlaying;
    bool isPaused;
    bool isLooping;
    int speed; // Playback speed in fps
    uint64_t lastUpdateTime; // Last update time in milliseconds
    std::string audioTrack;

    bool loadFrame(size_t frameIndex);
    void refreshFrameCache(size_t centerFrame, size_t loadBudget);
    size_t getFrameIndexWithOffset(size_t frameIndex, int offset) const;
    void clear() override;
};
