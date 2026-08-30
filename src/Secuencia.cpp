#include "Secuencia.h"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

struct SequenceDecodedFrame {
    // El hilo de carga entrega píxeles puros. La textura se crea después,
    // exclusivamente en el hilo principal que posee el contexto OpenGL.
    uint64_t generation = 0;
    size_t frameIndex = 0;
    bool loaded = false;
    ofPixels pixels;
};

struct SequenceFrameMailbox {
    // Al cambiar de Play o detener una secuencia se marca el buzón anterior.
    // Los trabajos que todavía estaban en cola se descartan sin decodificar PNG.
    std::atomic<bool> cancelled{false};
    ofThreadChannel<SequenceDecodedFrame> decodedFrames;
};

namespace {

struct SequenceFrameRequest {
    uint64_t generation = 0;
    size_t frameIndex = 0;
    std::string path;
    int targetWidth = 0;
    int targetHeight = 0;
    // Menor prioridad numérica significa que el cuadro se necesita antes.
    size_t priority = 0;
    uint64_t order = 0;
    std::shared_ptr<SequenceFrameMailbox> mailbox;
};

class SequenceFrameLoader {
public:
    SequenceFrameLoader() {
        // El cargador es compartido por todas las secuencias. Usa entre dos y
        // cuatro hilos para atender varias animaciones sin saturar el equipo.
        const unsigned int hardwareThreads = std::thread::hardware_concurrency();
        const size_t workerCount = std::clamp(
            hardwareThreads == 0 ? MIN_WORKERS : static_cast<size_t>(hardwareThreads / 2),
            MIN_WORKERS,
            MAX_WORKERS);
        workers.reserve(workerCount);
        for (size_t i = 0; i < workerCount; ++i) {
            workers.push_back(std::make_unique<Worker>(*this));
        }
    }

    ~SequenceFrameLoader() {
        {
            std::lock_guard<std::mutex> lock(requestMutex);
            closing = true;
        }
        requestReady.notify_all();
        workers.clear();
    }

    void request(SequenceFrameRequest request) {
        std::lock_guard<std::mutex> lock(requestMutex);
        if (closing) {
            return;
        }
        request.order = nextRequestOrder++;
        requests.push(std::move(request));
        requestReady.notify_one();
    }

private:
    struct RequestOrder {
        bool operator()(const SequenceFrameRequest& left, const SequenceFrameRequest& right) const {
            if (left.priority != right.priority) {
                return left.priority > right.priority;
            }
            return left.order > right.order;
        }
    };

    class Worker final : public ofThread {
    public:
        explicit Worker(SequenceFrameLoader& loader)
            : loader(loader) {
            startThread();
        }

        ~Worker() {
            waitForThread(true);
        }

    protected:
        void threadedFunction() override {
            SequenceFrameRequest request;
            while (loader.takeNextRequest(request)) {
                // Un Play nuevo invalida la cola anterior. Saltar el trabajo acá
                // evita que una animación vieja demore el siguiente disparo.
                if (!request.mailbox || request.mailbox->cancelled.load()) {
                    continue;
                }

                SequenceDecodedFrame decodedFrame;
                decodedFrame.generation = request.generation;
                decodedFrame.frameIndex = request.frameIndex;
                decodedFrame.loaded = ofLoadImage(decodedFrame.pixels, request.path);

                // El FBO de BMapper trabaja a 800×600. Reducir los píxeles antes
                // de guardarlos evita cachear y subir a GPU información que no se verá.
                if (decodedFrame.loaded && request.targetWidth > 0 && request.targetHeight > 0 &&
                    (decodedFrame.pixels.getWidth() != request.targetWidth ||
                     decodedFrame.pixels.getHeight() != request.targetHeight)) {
                    decodedFrame.pixels.resize(request.targetWidth, request.targetHeight);
                }

                if (!request.mailbox->cancelled.load()) {
                    request.mailbox->decodedFrames.send(std::move(decodedFrame));
                }
            }
        }

    private:
        SequenceFrameLoader& loader;
    };

    bool takeNextRequest(SequenceFrameRequest& request) {
        std::unique_lock<std::mutex> lock(requestMutex);
        requestReady.wait(lock, [this] { return closing || !requests.empty(); });
        if (closing) {
            return false;
        }
        request = requests.top();
        requests.pop();
        return true;
    }

    static constexpr size_t MIN_WORKERS = 2;
    static constexpr size_t MAX_WORKERS = 4;
    std::priority_queue<SequenceFrameRequest, std::vector<SequenceFrameRequest>, RequestOrder> requests;
    std::mutex requestMutex;
    std::condition_variable requestReady;
    uint64_t nextRequestOrder = 0;
    bool closing = false;
    std::vector<std::unique_ptr<Worker>> workers;
};

SequenceFrameLoader& getSequenceFrameLoader() {
    // Instancia única, creada recién cuando una secuencia necesita precarga.
    static SequenceFrameLoader loader;
    return loader;
}

} // namespace

Secuencia::Secuencia()
    : currentFrame(0), isPlaying(false), isPaused(false), isPreparing(false), isLooping(false),
      speed(24), lastUpdateTime(0), loadGeneration(0) {
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
    processDecodedFrames();

    if (framePaths.empty()) {
        return;
    }

    const uint64_t currentTime = ofGetElapsedTimeMillis();

    if (isPreparing) {
        // Play no inicia el audio hasta contar con una pequeña reserva de cuadros.
        // Así se evita arrancar fluido y frenarse enseguida por una lectura de disco.
        if (!hasStartBuffer()) {
            return;
        }

        isPreparing = false;
        isPlaying = true;
        lastUpdateTime = currentTime;
        if (soundPlayer.isLoaded()) {
            soundPlayer.play();
        }
        ofLogNotice("ImageSequenceSource") << "Sequence buffered and playing: " << name;
        return;
    }

    if (!isPlaying) {
        return;
    }

    const uint64_t elapsedTime = currentTime - lastUpdateTime;
    if (elapsedTime < frameDuration) {
        requestFrameWindow(currentFrame);
        return;
    }

    const size_t nextFrame = getFrameIndexWithOffset(currentFrame, 1);
    if (nextFrame >= framePaths.size()) {
        stop();
        return;
    }

    // No se avanza sobre un cuadro que todavía se está decodificando. La imagen
    // puede esperar brevemente, pero el render permanece responsivo y la cola
    // no se llena de pedidos obsoletos.
    if (frameCache.count(nextFrame) == 0) {
        queueFrame(nextFrame);
        return;
    }

    currentFrame = static_cast<int>(nextFrame);
    lastUpdateTime = currentTime;
    requestFrameWindow(currentFrame);
    uploadFrame(currentFrame);
}

void Secuencia::draw() {
    // El PNG ya trae su canal alfa. Limpiar el FBO con alfa cero evita que el
    // fondo negro opaco reemplace las zonas transparentes de cada fotograma.
    ofClear(0, 0, 0, 0);
    ofEnableAlphaBlending();
    ofSetColor(255, 255, 255, 255);
    if (frameTexture.isAllocated()) {
        frameTexture.draw(0, 0, getWidth(), getHeight());
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

    ofPixels firstFrame;
    if (!ofLoadImage(firstFrame, loadedPaths.front())) {
        ofLogError("ImageSequenceSource") << "Failed to load first image: " << loadedPaths.front();
        return false;
    }
    // El primer cuadro actúa como imagen de espera y se ajusta al mismo tamaño
    // interno del FBO. Los PNG originales del proyecto no se alteran.
    if (getWidth() > 0 && getHeight() > 0 &&
        (firstFrame.getWidth() != static_cast<int>(getWidth()) ||
         firstFrame.getHeight() != static_cast<int>(getHeight()))) {
        firstFrame.resize(static_cast<int>(getWidth()), static_cast<int>(getHeight()));
    }

    clear();
    framePaths = std::move(loadedPaths);
    frameCache.emplace(0, std::move(firstFrame));
    currentFrame = 0;
    frameMailbox = std::make_shared<SequenceFrameMailbox>();
    wantedFrames.insert(0);
    uploadFrame(0);
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
        // Cada Play parte de una cola nueva, incluso si el anterior se interrumpió.
        resetFrameMailbox();
        currentFrame = 0;
        isPlaying = false;
        isPaused = false;
        isPreparing = true;
        if (soundPlayer.isLoaded()) {
            soundPlayer.stop();
            soundPlayer.setPosition(0.0f);
        }
        lastUpdateTime = ofGetElapsedTimeMillis();
        requestFrameWindow(currentFrame);
    }
}

void Secuencia::stop() {
    ofLogNotice() << "------->STOP secuencia: " + name;
    isPlaying = false;
    isPaused = false;
    isPreparing = false;
    if (soundPlayer.isLoaded()) {
        soundPlayer.stop();
        soundPlayer.setPosition(0.0f);
    }
    currentFrame = 0;
    // Detener también invalida los cuadros pendientes: no deben competir con
    // una reproducción que el usuario pueda iniciar más tarde.
    resetFrameMailbox();
    wantedFrames.clear();
    wantedFrames.insert(currentFrame);
    for (auto it = frameCache.begin(); it != frameCache.end();) {
        if (it->first != static_cast<size_t>(currentFrame)) {
            it = frameCache.erase(it);
        } else {
            ++it;
        }
    }
    queueFrame(currentFrame);
    uploadFrame(currentFrame);
}

void Secuencia::pause() {
    ofLogNotice() << "------->PAUSE secuencia: " + name;
    if (!isPlaying) {
        isPreparing = false;
        return;
    }
    isPlaying = false;
    isPaused = true;
    isPreparing = false;
    if (soundPlayer.isLoaded()) {
        soundPlayer.setPaused(true);
    }
}

void Secuencia::resume() {
    ofLogNotice() << "------->RESUME secuencia: " + name;
    if (framePaths.empty()) {
        return;
    }

    if (!isPaused) {
        play();
        return;
    }

    isPlaying = true;
    isPaused = false;
    isPreparing = false;
    if (soundPlayer.isLoaded()) {
        soundPlayer.setPaused(false);
    }
    lastUpdateTime = ofGetElapsedTimeMillis();
    requestFrameWindow(currentFrame);
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
    if (frameMailbox) {
        frameMailbox->cancelled.store(true);
    }
    ++loadGeneration;
    frameCache.clear();
    failedFrames.clear();
    queuedFrames.clear();
    wantedFrames.clear();
    frameMailbox.reset();
    frameTexture.clear();
    framePaths.clear();
    currentFrame = 0;
    soundPlayer.stop();
    soundPlayer.unload();
    audioTrack.clear();
    isPlaying = false;
    isPaused = false;
    isPreparing = false;
}

void Secuencia::processDecodedFrames() {
    if (!frameMailbox) {
        return;
    }

    SequenceDecodedFrame decodedFrame;
    size_t processedFrames = 0;
    while (processedFrames < MAX_DECODED_FRAMES_PER_UPDATE &&
           frameMailbox->decodedFrames.tryReceive(decodedFrame)) {
        ++processedFrames;
        // Un resultado de otra generación pertenece a un Play ya cancelado.
        if (decodedFrame.generation != loadGeneration) {
            continue;
        }

        queuedFrames.erase(decodedFrame.frameIndex);
        if (!decodedFrame.loaded) {
            failedFrames.insert(decodedFrame.frameIndex);
            ofLogError("ImageSequenceSource") << "Failed to load image: " << framePaths[decodedFrame.frameIndex];
            continue;
        }

        // Si el cuadro quedó fuera de la ventana actual, liberarlo enseguida.
        if (wantedFrames.count(decodedFrame.frameIndex) == 0) {
            continue;
        }

        frameCache.emplace(decodedFrame.frameIndex, std::move(decodedFrame.pixels));
        if (decodedFrame.frameIndex == static_cast<size_t>(currentFrame)) {
            uploadFrame(decodedFrame.frameIndex);
        }
    }
}

void Secuencia::resetFrameMailbox() {
    if (frameMailbox) {
        frameMailbox->cancelled.store(true);
    }
    // Cambiar la generación es una barrera lógica entre dos reproducciones.
    ++loadGeneration;
    queuedFrames.clear();
    frameMailbox = std::make_shared<SequenceFrameMailbox>();
}

void Secuencia::requestFrameWindow(size_t centerFrame) {
    if (framePaths.empty() || centerFrame >= framePaths.size()) {
        return;
    }

    wantedFrames.clear();
    wantedFrames.insert(centerFrame);

    for (size_t offset = 1; offset <= CACHE_AHEAD_FRAMES; ++offset) {
        const size_t frameIndex = getFrameIndexWithOffset(centerFrame, static_cast<int>(offset));
        if (frameIndex >= framePaths.size()) {
            break;
        }
        wantedFrames.insert(frameIndex);
    }

    for (size_t offset = 1; offset <= CACHE_BEHIND_FRAMES; ++offset) {
        const size_t frameIndex = getFrameIndexWithOffset(centerFrame, -static_cast<int>(offset));
        if (frameIndex >= framePaths.size()) {
            break;
        }
        wantedFrames.insert(frameIndex);
    }

    // Mantener sólo los cuadros próximos limita la RAM aun con secuencias largas.
    for (auto it = frameCache.begin(); it != frameCache.end();) {
        if (wantedFrames.count(it->first) == 0) {
            it->second.clear();
            it = frameCache.erase(it);
        } else {
            ++it;
        }
    }

    // Los cuadros más cercanos entran primero a la cola global. Así, al iniciar
    // varias superficies a la vez, todas reciben sus próximos cuadros de forma equitativa.
    queueFrame(centerFrame, 0);
    for (size_t offset = 1; offset <= CACHE_AHEAD_FRAMES; ++offset) {
        const size_t frameIndex = getFrameIndexWithOffset(centerFrame, static_cast<int>(offset));
        if (frameIndex >= framePaths.size()) {
            break;
        }
        queueFrame(frameIndex, offset);
    }
    for (size_t offset = 1; offset <= CACHE_BEHIND_FRAMES; ++offset) {
        const size_t frameIndex = getFrameIndexWithOffset(centerFrame, -static_cast<int>(offset));
        if (frameIndex >= framePaths.size()) {
            break;
        }
        queueFrame(frameIndex, CACHE_AHEAD_FRAMES + offset);
    }
}

void Secuencia::queueFrame(size_t frameIndex, size_t priority) {
    if (!frameMailbox || frameIndex >= framePaths.size() || frameCache.count(frameIndex) != 0 ||
        queuedFrames.count(frameIndex) != 0 || failedFrames.count(frameIndex) != 0) {
        return;
    }

    // Este método sólo agenda trabajo: no abre PNG ni usa OpenGL.
    SequenceFrameRequest request;
    request.generation = loadGeneration;
    request.frameIndex = frameIndex;
    request.path = framePaths[frameIndex];
    request.targetWidth = static_cast<int>(getWidth());
    request.targetHeight = static_cast<int>(getHeight());
    request.priority = priority;
    request.mailbox = frameMailbox;
    queuedFrames.insert(frameIndex);
    getSequenceFrameLoader().request(std::move(request));
}

void Secuencia::uploadFrame(size_t frameIndex) {
    const auto frame = frameCache.find(frameIndex);
    if (frame != frameCache.end() && frame->second.isAllocated()) {
        // Reutilizar una única textura evita reservar una textura GPU por cuadro.
        frameTexture.loadData(frame->second);
    }
}

bool Secuencia::hasStartBuffer() const {
    if (framePaths.empty()) {
        return false;
    }

    // Para secuencias cortas se pide como máximo la cantidad de cuadros existente.
    const size_t requiredFrames = std::min(START_BUFFER_FRAMES, framePaths.size());
    for (size_t offset = 0; offset < requiredFrames; ++offset) {
        const size_t frameIndex = getFrameIndexWithOffset(currentFrame, static_cast<int>(offset));
        if (frameIndex >= framePaths.size() || frameCache.count(frameIndex) == 0) {
            return false;
        }
    }
    return true;
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

size_t Secuencia::getQueuedFrameCount() const {
    return queuedFrames.size();
}

bool Secuencia::isPreparingPlayback() const {
    return isPreparing;
}

std::string Secuencia::getName() {
    return name;
}

void Secuencia::setName(std::string newName) {
    name = newName;
}
