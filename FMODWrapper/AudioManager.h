#pragma once

#include <fmod.hpp>
#include <fmod_errors.h> 
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <iostream>  
#include <algorithm> 
#include <filesystem> 
#include <set>

// Helper per controllo errori FMOD (invariato)
inline void FMOD_ERRCHECK(FMOD_RESULT result, const char* file, int line) {
    if (result != FMOD_OK) {
        std::cerr << "FMOD error! (" << result << ") " << FMOD_ErrorString(result)
            << " - " << file << ":" << line << std::endl;
        exit(-1);
    }
}
#define ERRCHECK(result) FMOD_ERRCHECK(result, __FILE__, __LINE__)

// Handle per i canali riprodotti (invariato)
using ChannelId = int;
const ChannelId INVALID_CHANNEL_ID = -1;

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    // Init System
    bool Initialize(int maxChannels = 128, FMOD_INITFLAGS flags = FMOD_INIT_NORMAL);
    void Shutdown();

    // Load (specificando static/streaming e loop/one-shot)
    // filePath: Percorso del file audio
    // isStreaming: true per caricare come stream (musica), false per caricare in memoria (effetto)
    // isLooping: true per loop infinito di default, false per one-shot
    bool LoadSound(const std::string& filePath, bool isStreaming, bool isLooping);
    bool UnloadSound(const std::string& filePath); // Usa lo stesso path/nome usato per caricare

    // Play: Avvia la riproduzione di un suono caricato
    // Restituisce un ChannelId per controllarlo successivamente
    ChannelId PlaySound(const std::string& filePath, float initialVolume = 1.0f, float initialPan = 0.0f);

    // Stop: Ferma un suono specifico tramite il suo ChannelId
    bool StopChannel(ChannelId id);

    // Pause: Mette in pausa o riprende un suono specifico tramite il suo ChannelId
    bool SetChannelPaused(ChannelId id, bool pause);

    // Volume: Imposta il volume (0.0 a 1.0) per un ChannelId specifico
    bool SetChannelVolume(ChannelId id, float volume);

    // Pan: Imposta il bilanciamento stereo (-1.0 Sinistra, 0.0 Centro, 1.0 Destra)
    bool SetChannelPan(ChannelId id, float pan);

    std::vector<std::string> GetLoadedSoundPaths() const;
    std::vector<std::string> FindSupportedAudioFilesInDirectory(const std::string& directoryPath) const;

    void Update();

    // Utility
    bool IsChannelPlaying(ChannelId id);


private:
    FMOD::System* m_system = nullptr;
    int m_nextChannelId = 0;

    // Mappe per gestire le risorse (Canali Audio)
    std::map<std::string, FMOD::Sound*> m_sounds;   // Mappa da nome/path a suono FMOD
    std::map<ChannelId, FMOD::Channel*> m_channels; // Mappa da ID wrapper a canale FMOD attivo

    // Trova un canale attivo dall'ID (helper interno)
    FMOD::Channel* GetChannel(ChannelId id);

    bool IsSupportedAudioExtension(const std::string& extension) const;
    std::set<std::string> m_supportedExtensions = { ".wav", ".mp3", ".ogg", ".flac", ".aiff" }; // Estensioni audio comuni
};