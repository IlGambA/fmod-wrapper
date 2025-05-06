#include "AudioManager.h"
#include <vector> 
#include <string>
#include <algorithm>
#include <cctype>    


AudioManager::AudioManager() : m_system(nullptr), m_nextChannelId(0) {}

AudioManager::~AudioManager() {
    Shutdown();
}

// --- Init / Shutdown ---

bool AudioManager::Initialize(int maxChannels, FMOD_INITFLAGS flags) {
    FMOD_RESULT result;
    result = FMOD::System_Create(&m_system);
    ERRCHECK(result);

    // Controlla se il sistema è stato creato prima di inizializzarlo
    if (m_system) {
        result = m_system->init(maxChannels, flags, nullptr);
        ERRCHECK(result);
        std::cout << "AudioManager Initialized successfully (" << maxChannels << " channels)." << std::endl;
        return result == FMOD_OK;
    }
    return false;
}

void AudioManager::Shutdown() {
    if (!m_system) return;

    FMOD_RESULT result;

    // Rilascia tutti i suoni caricati
    for (auto const& [key, val] : m_sounds) {
        result = val->release();
        if (result != FMOD_OK) std::cerr << "FMOD Error releasing sound: " << key << " (" << FMOD_ErrorString(result) << ")" << std::endl;
    }
    m_sounds.clear();

    // Rilascia il sistema
    result = m_system->close();
    if (result != FMOD_OK) std::cerr << "FMOD Error closing system: (" << FMOD_ErrorString(result) << ")" << std::endl;

    result = m_system->release();
    if (result != FMOD_OK) std::cerr << "FMOD Error releasing system: (" << FMOD_ErrorString(result) << ")" << std::endl;

    m_system = nullptr;
    m_channels.clear(); // Svuota la mappa dei canali attivi
    std::cout << "AudioManager Shutdown." << std::endl;
}

// --- Load / Unload ---

bool AudioManager::LoadSound(const std::string& filePath, bool isStreaming, bool isLooping) {
    if (m_sounds.count(filePath)) {
        return true;
    }
    if (!m_system) {
        std::cerr << "Error: AudioManager not initialized." << std::endl;
        return false;
    }

    FMOD_MODE mode = FMOD_DEFAULT | FMOD_2D;
    mode |= isStreaming ? FMOD_CREATESTREAM : FMOD_CREATESAMPLE;
    mode |= isLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;

    FMOD::Sound* sound = nullptr;
    FMOD_RESULT result = m_system->createSound(filePath.c_str(), mode, nullptr, &sound);
    ERRCHECK(result);

    if (result == FMOD_OK) {
        m_sounds[filePath] = sound;
        std::cout << "Loaded sound: " << filePath << (isStreaming ? " (Stream)" : " (Sample)") << (isLooping ? " (Loop)" : " (OneShot)") << std::endl;
        return true;
    }
    return false;
}

bool AudioManager::UnloadSound(const std::string& filePath) {
    auto it = m_sounds.find(filePath);
    if (it == m_sounds.end()) {
        std::cerr << "Sound not found to unload: " << filePath << std::endl;
        return false;
    }
    if (!m_system) {
        std::cerr << "Error: AudioManager not initialized." << std::endl;
        return false;
    }

    FMOD_RESULT result = it->second->release();
    ERRCHECK(result);

    if (result == FMOD_OK) {
        m_sounds.erase(it);
        std::cout << "Unloaded sound: " << filePath << std::endl;
        return true;
    }
    return false;
}

// --- Playback Control (Play, Stop, Pause) ---

ChannelId AudioManager::PlaySound(const std::string& filePath, float initialVolume, float initialPan) {
    auto it = m_sounds.find(filePath);
    if (it == m_sounds.end()) {
        std::cerr << "Sound not loaded, cannot play: " << filePath << std::endl;
        return INVALID_CHANNEL_ID;
    }
    if (!m_system) {
        std::cerr << "Error: AudioManager not initialized." << std::endl;
        return INVALID_CHANNEL_ID;
    }

    FMOD::Channel* channel = nullptr;
    // Avvia il canale in pausa per impostare volume/pan prima che inizi effettivamente
    FMOD_RESULT result = m_system->playSound(it->second, nullptr, true, &channel);
    ERRCHECK(result);

    if (channel) {
        // Imposta volume iniziale
        result = channel->setVolume(std::max(0.0f, std::min(1.0f, initialVolume)));
        ERRCHECK(result);

        // Imposta pan iniziale
        result = channel->setPan(std::max(-1.0f, std::min(1.0f, initialPan)));
        ERRCHECK(result);

        // Ora fallo partire
        result = channel->setPaused(false);
        ERRCHECK(result);

        // Assegna e memorizza il ChannelId
        ChannelId newId = m_nextChannelId++;
        m_channels[newId] = channel;
        return newId;
    }

    return INVALID_CHANNEL_ID;
}

bool AudioManager::StopChannel(ChannelId id) {
    FMOD::Channel* channel = GetChannel(id);
    if (!channel) return false; // Canale non trovato o già fermato
    FMOD_RESULT result = channel->stop();
    ERRCHECK(result);
    return result == FMOD_OK;
}

bool AudioManager::SetChannelPaused(ChannelId id, bool pause) {
    FMOD::Channel* channel = GetChannel(id);
    if (!channel) return false;
    FMOD_RESULT result = channel->setPaused(pause);
    ERRCHECK(result);
    return result == FMOD_OK;
}

// --- Channel Properties (Volume, Pan) ---

bool AudioManager::SetChannelVolume(ChannelId id, float volume) {
    FMOD::Channel* channel = GetChannel(id);
    if (!channel) return false;
    // Clamp volume [0.0, 1.0]
    volume = std::max(0.0f, std::min(1.0f, volume));
    FMOD_RESULT result = channel->setVolume(volume);
    ERRCHECK(result);
    return result == FMOD_OK;
}

bool AudioManager::SetChannelPan(ChannelId id, float pan) {
    FMOD::Channel* channel = GetChannel(id);
    if (!channel) return false;
    // Clamp pan [-1.0, 1.0]
    pan = std::max(-1.0f, std::min(1.0f, pan));
    FMOD_RESULT result = channel->setPan(pan);
    ERRCHECK(result);
    return result == FMOD_OK;
}

// --- Update & Utility ---

void AudioManager::Update() {
    if (!m_system) return;

    // Pulisci i canali che hanno finito di suonare dalla nostra mappa interna
    std::vector<ChannelId> stoppedChannels;
    for (auto it = m_channels.begin(); it != m_channels.end();) {
        bool isPlaying = false;
        FMOD_RESULT res = it->second->isPlaying(&isPlaying);

        // Rimuovi se non sta suonando O se la chiamata a isPlaying fallisce (canale invalido)
        if (res != FMOD_OK || !isPlaying) {
            // Rimuovi dalla mappa e avanza l'iteratore
            it = m_channels.erase(it);
        }
        else {
            // Altrimenti avanza l'iteratore
            ++it;
        }
    }

    FMOD_RESULT result = m_system->update();
    ERRCHECK(result);
}

bool AudioManager::IsChannelPlaying(ChannelId id) {
    FMOD::Channel* channel = GetChannel(id);
    if (!channel) return false;
    bool isPlaying = false;
    FMOD_RESULT result = channel->isPlaying(&isPlaying);
    return result == FMOD_OK && isPlaying;
}

std::vector<std::string> AudioManager::GetLoadedSoundPaths() const {
    std::vector<std::string> loadedPaths;
    for (const auto& pair : m_sounds) {
        loadedPaths.push_back(pair.first);
    }
    return loadedPaths;
}

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return s;
}

bool AudioManager::IsSupportedAudioExtension(const std::string& extension) const {
    return m_supportedExtensions.count(toLower(extension));
}

std::vector<std::string> AudioManager::FindSupportedAudioFilesInDirectory(const std::string& directoryPath) const {
    std::vector<std::string> foundFiles;
    namespace fs = std::filesystem;
    fs::path dirPath(directoryPath);

    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        std::cerr << "Error [AudioManager]: The specified directory does not exist or is invalid: "
            << directoryPath << std::endl;
        return foundFiles; // Restituisce vettore vuoto
    }

    try {
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (entry.is_regular_file()) {
                fs::path filePath = entry.path();
                std::string extension = filePath.extension().string();

                if (IsSupportedAudioExtension(extension)) {
                    // Aggiungi il percorso completo alla lista
                    foundFiles.push_back(filePath.string());
                }
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem [AudioManager] error during directory scan: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Filesystem [AudioManager] error during directory scan: " << e.what() << std::endl;
    }

    return foundFiles;
}

FMOD::Channel* AudioManager::GetChannel(ChannelId id) {
    auto it = m_channels.find(id);
    if (it == m_channels.end()) {
        return nullptr;
    }
    return it->second;
}