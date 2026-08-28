#pragma once

#include "ofMain.h"
#include "FboSource.h"
#include "ofSoundPlayer.h"

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

    std::string getName();
    void setName(std::string newName);

private:
    //std::vector<ofTexture> images;
    std::vector<ofImage> images;
    ofSoundPlayer soundPlayer;
    int currentFrame;
    float frameDuration; // Duration of each frame in milliseconds
    bool isPlaying;
    bool isPaused;
    bool isLooping;
    int speed; // Playback speed in fps
    uint64_t lastUpdateTime; // Last update time in milliseconds
    std::string audioTrack;
    void clear() override;
};
