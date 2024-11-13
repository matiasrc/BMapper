#pragma once

#include "ofMain.h"
#include "FboSource.h"
#include "ofSoundPlayer.h"

class Secuencia : public ofx::piMapper::FboSource {
public:
    Secuencia();
    ~Secuencia();

    void setup(std::string newName);
    void update();
    void draw();

    bool loadSequence(const std::string& folderPath);
    bool setAudioTrack(const std::string& audioPath);
    std::string getAudioTrack();
    void play();
    void stop();
    void pause();
    void resume();

    void setLoop(bool loop);
    bool getLoop();

    void setSpeed(int fps);
    int getSpeed();

    std::string getName();
    void setName(std::string newName);

private:
    //std::vector<ofTexture> images;
    std::vector<ofImage> images;
    ofSoundPlayer soundPlayer;
    int currentFrame;
    float frameDuration; // Duration of each frame in milliseconds
    bool isPlaying;
    bool isLooping;
    int speed; // Playback speed in fps
    uint64_t lastUpdateTime; // Last update time in milliseconds
    std::string audioTrack;
    void clear();
};

