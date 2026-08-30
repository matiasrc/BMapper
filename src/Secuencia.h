#pragma once

#include "ofMain.h"
#include "FboSource.h"
#include "ofSoundPlayer.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>

struct SequenceFrameMailbox;

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
    size_t getQueuedFrameCount() const;
    bool isPreparingPlayback() const;

    std::string getName();
    void setName(std::string newName);

private:
    // Ventana de cuadros decodificados que se mantiene alrededor del cuadro actual.
    // Los valores están pensados para sostener aproximadamente 0,75 s a 24 fps.
    static constexpr size_t CACHE_AHEAD_FRAMES = 18;
    static constexpr size_t CACHE_BEHIND_FRAMES = 2;
    // Antes de reproducir se espera este mínimo para que audio e imagen comiencen estables.
    static constexpr size_t START_BUFFER_FRAMES = 6;
    // Limita el trabajo de incorporar resultados en cada update del hilo principal.
    static constexpr size_t MAX_DECODED_FRAMES_PER_UPDATE = 8;

    std::vector<std::string> framePaths;
    // Cuadros ya decodificados en memoria RAM. No tienen textura propia de OpenGL.
    std::unordered_map<size_t, ofPixels> frameCache;
    std::unordered_set<size_t> failedFrames;
    // Cuadros pedidos al cargador y cuadros que siguen siendo útiles para esta reproducción.
    std::unordered_set<size_t> queuedFrames;
    std::unordered_set<size_t> wantedFrames;
    std::shared_ptr<SequenceFrameMailbox> frameMailbox;
    // Única textura GPU reutilizada para mostrar el cuadro actual de la secuencia.
    ofTexture frameTexture;
    ofSoundPlayer soundPlayer;
    int currentFrame;
    float frameDuration; // Duration of each frame in milliseconds
    bool isPlaying;
    bool isPaused;
    // Estado transitorio: el usuario pidió Play, pero todavía se está llenando el búfer.
    bool isPreparing;
    bool isLooping;
    int speed; // Playback speed in fps
    uint64_t lastUpdateTime; // Last update time in milliseconds
    // Identifica la generación de carga vigente; evita aceptar cuadros de un Play anterior.
    uint64_t loadGeneration;
    std::string audioTrack;

    void processDecodedFrames();
    void resetFrameMailbox();
    void requestFrameWindow(size_t centerFrame);
    void queueFrame(size_t frameIndex, size_t priority = 0);
    void uploadFrame(size_t frameIndex);
    bool hasStartBuffer() const;
    size_t getFrameIndexWithOffset(size_t frameIndex, int offset) const;
    void clear() override;
};
