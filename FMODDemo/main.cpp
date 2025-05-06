#include "AudioManager.h"
#include <iostream>
#include <string>
#include <vector>
#include <limits> 
#include <thread>
#include <chrono>
#include <map>   

struct ActiveChannelInfo {
    std::string soundName;
    
};

void clearInputBuffer() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

template <typename T>
T getNumericInput(const std::string& prompt) {
    T value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            clearInputBuffer(); 
            return value;
        }
        else {
            std::cerr << "Invalid input. Try again.\n";
            std::cin.clear(); 
            clearInputBuffer();
        }
    }
}

std::string getStringInput(const std::string& prompt) {
    std::string value;
    std::cout << prompt;
    std::getline(std::cin, value); 
    return value;
}


int main() {
    AudioManager audioManager;
    std::map<ChannelId, ActiveChannelInfo> activeChannels; 

    std::cout << "-------------------------------------------------  \n";
    std::cout << "      FMOD Wrapper Demo by Stefano Gambarin       \n";
    std::cout << "-------------------------------------------------\n\n";

    std::cout << "Initialization AudioManager... \n";
    
    // 1. Inizializzazione
    int channelNumber = getNumericInput<int>("Enter number of channels for initialization (1-4095): ");
    if (channelNumber > 0 && channelNumber <4096) { 
        if (!audioManager.Initialize(channelNumber)) {
            std::cerr << "Errore: Impossibile inizializzare AudioManager!\n";
            return 1;
        }
    }
    else {
        if (!audioManager.Initialize()) {
            std::cerr << "Errore: Impossibile inizializzare AudioManager!\n";
            return 1;
        }
    }
   

    bool running = true;
    while (running) {
        audioManager.Update();

        // Show Menu Options
        std::cout << "\n--- Main Menu ---\n";
        std::cout << "01. Load Sound (Sample/Static)\n";
        std::cout << "02. Load Sound (Stream)\n";
        std::cout << "03. Load Sounds from folder\n";
        std::cout << "04. Unload Sound\n";
        std::cout << "05. Play Sound\n";
        std::cout << "06. Stop Channel\n";
        std::cout << "07. Pause/Play Channel\n";
        std::cout << "08. Set Channel Volume\n";
        std::cout << "09. Set Channel PAN\n";
        std::cout << "10. Show Active Channel\n";
        std::cout << " 0. Exit\n";
        std::cout << "-----------------------\n";

        int choice = getNumericInput<int>("Choose an option: ");

        switch (choice) {
        case 1: // Carica Sample
        case 2: // Carica Stream
        {
            bool isStreaming = (choice == 2);
            std::string filePath = getStringInput("Insert file path without \"\": ");
            
            
            std::cout << "Loop sound? (y/n): ";
            char loopChar;
            std::cin >> loopChar;
            clearInputBuffer();
            bool isLooping = (loopChar == 'y' || loopChar == 'Y');

            if (audioManager.LoadSound(filePath, isStreaming, isLooping)) {
                std::cout << "Sound loaded successfully: " << filePath << "\n";
            }
            else {
                std::cerr << "Sound loading error.\n";
            }
            break;
        }
        case 3:
        {            
            std::string audioFolderPath = getStringInput("Insert folder path without \"\": ");
            //std::string audioFolderPath = "..\\..\\audio";
            std::cout << "Scan directory '" << audioFolderPath << "' for audio files...\n";

            // 1. Trova i file supportati
            std::vector<std::string> audioFiles = audioManager.FindSupportedAudioFilesInDirectory(audioFolderPath);

            if (audioFiles.empty()) {
                std::cout << "No supported audio files found in '" << audioFolderPath << "'.\n";
                break;
            }

            std::cout << "Found " << audioFiles.size() << " supported audio files.\n";
            int loadedCount = 0;
            bool skipRemaining = false;

            // 2. Itera sui file trovati e chiedi all'utente
            for (const std::string& filePath : audioFiles) {
                if (skipRemaining) break; // Se l'utente ha scelto di saltare il resto

                std::cout << "\n-------------------------------------\n";
                std::cout << "File: " << filePath << "\n";
                std::cout << "-------------------------------------\n";

                char loadChoice = ' ';
                while (loadChoice != 'y' && loadChoice != 'n' && loadChoice != 's') {
                    std::cout << "  Load this file? (y=Yes / n=No / s= skip this and all remaining): ";
                    std::cin >> loadChoice;
                    clearInputBuffer();
                    loadChoice = std::tolower(loadChoice);
                    if (loadChoice != 'y' && loadChoice != 'n' && loadChoice != 's') {
                        std::cerr << "Invalid input. Try again.\n";
                    }
                }

                if (loadChoice == 's') {
                    std::cout << "Skipping all the remaining files...\n";
                    skipRemaining = true;
                    continue; // Salta al prossimo ciclo del for (che verrà interrotto)
                }

                if (loadChoice == 'y') {
                    // Impostazioni streaming/loop
                    bool isStreaming = false;
                    bool isLooping = false;
                    char streamChoice = ' ';
                    char loopChoice = ' ';

                    while (streamChoice != 'y' && streamChoice != 'n') {
                        std::cout << "Load as Streaming? (y/n): ";
                        std::cin >> streamChoice;
                        clearInputBuffer();
                        streamChoice = std::tolower(streamChoice);
                        if (streamChoice != 'y' && streamChoice != 'n') {
                            std::cerr << "Invalid input. Try again.\n";
                        }
                    }
                    isStreaming = (streamChoice == 'y');

                    while (loopChoice != 'y' && loopChoice != 'n') {
                        std::cout << "Loop sound? (y/n=One-Shot): ";
                        std::cin >> loopChoice;
                        clearInputBuffer();
                        loopChoice = std::tolower(loopChoice);
                        if (loopChoice != 'y' && loopChoice != 'n') {
                            std::cerr << "Invalid input. Try again.\n";
                        }
                    }
                    isLooping = (loopChoice == 'y');

                    // Carica con le impostazioni scelte
                    std::cout << "  Loading ("
                        << (isStreaming ? "Stream" : "Sample") << ", "
                        << (isLooping ? "Loop" : "OneShot") << ")...\n";
                    if (audioManager.LoadSound(filePath, isStreaming, isLooping)) {
                        loadedCount++;
                        std::cout << "    -> OK: Loaded successfully.\n";
                    }
                    else {
                        std::cerr << "    ->  ERROR while loading.\n";
                    }
                }
                else { // loadChoice == 'n'
                    std::cout << "    -> Skipped.\n";
                }
            } // Fine ciclo for sui file

            std::cout << "\nScanning and loading completed. "
                << loadedCount << " files loaded.\n";
            break;
        }


        case 4: // Scarica Suono
        {
            std::vector<std::string> loadedSounds = audioManager.GetLoadedSoundPaths();
            if (loadedSounds.empty()) {
                std::cerr << "No sound loaded! Load one first (Option 1,2 or 3).\n";
                break; // Torna al menu principale
            }
            
            std::cout << "\n--- Select sound to unload ---\n";
            for (size_t i = 0; i < loadedSounds.size(); ++i) {
                std::cout << (i + 1) << ". " << loadedSounds[i] << "\n";
            }
            std::cout << "--------------------------------------\n";

            int soundChoice = 0;
            while (true) {
                soundChoice = getNumericInput<int>("Enter sound number: ");
                if (soundChoice >= 1 && soundChoice <= static_cast<int>(loadedSounds.size())) {
                    break; // Scelta valida
                }
                else {
                    std::cerr << "Invalid number. Try again.\n";
                }
            }

            std::string selectedSoundPath = loadedSounds[soundChoice - 1];
            if (audioManager.UnloadSound(selectedSoundPath)) {
                std::cout << "Unloaded sound: " << selectedSoundPath << "\n";
            }
            else {
                std::cerr << "Error unloading sound (may not be loaded)\n";
            }
            break;
        }
        case 5: // Riproduci Suono
        {
            // Ottieni l'elenco dei suoni caricati dall'AudioManager
            std::vector<std::string> loadedSounds = audioManager.GetLoadedSoundPaths();

            if (loadedSounds.empty()) {
                std::cerr << "No sound loaded! Load one first (Option 1,2 or 3).\n";
                break; // Torna al menu principale
            }

            std::cout << "\n--- Select Sound to Play ---\n";
            for (size_t i = 0; i < loadedSounds.size(); ++i) {
                std::cout << (i + 1) << ". " << loadedSounds[i] << "\n";
            }
            std::cout << "--------------------------------------\n";

            int soundChoice = 0;
            while (true) {
                soundChoice = getNumericInput<int>("Enter sound number: ");
                if (soundChoice >= 1 && soundChoice <= static_cast<int>(loadedSounds.size())) {
                    break; // Scelta valida
                }
                else {
                    std::cerr << "Invalid number. Try again.\n";
                }
            }

            // Ottieni il percorso del file selezionato (indice 0-based)
            std::string selectedSoundPath = loadedSounds[soundChoice - 1];

            // Chiedi volume e pan come prima
            float volume = getNumericInput<float>("Initial Volume (0.0-1.0): ");
            float pan = getNumericInput<float>("Initial PAN (-1.0 a 1.0): ");

            // Riproduci il suono selezionato
            ChannelId newId = audioManager.PlaySound(selectedSoundPath, volume, pan);
            if (newId != INVALID_CHANNEL_ID) {
                activeChannels[newId] = { selectedSoundPath }; // Memorizza l'ID e il nome
                std::cout << "Sound '" << selectedSoundPath << "' playing. Channel ID: " << newId << "\n";
            }
            else {
                // Questo non dovrebbe accadere se la lista è corretta, ma per sicurezza...
                std::cerr << "Sound playback error '" << selectedSoundPath << "'.\n";
            }
            break;
        }
        case 6: // Stoppa Canale
        {
            ChannelId id = getNumericInput<ChannelId>("Enter Channel ID to stop: ");
            if (activeChannels.count(id)) {
                if (audioManager.StopChannel(id)) {
                    std::cout << "Channel " << id << " stopped.\n";
                    activeChannels.erase(id);
                }
                else {
                    std::cerr << "Error Stopping Channel " << id << ".\n";
                }
            }
            else {
                std::cerr << "Channel ID " << id << " not found or no longer active.\n";
            }
            break;
        }
        case 7: // Pausa/Riprendi Canale
        {
            ChannelId id = getNumericInput<ChannelId>("Enter Channel ID to pause/resume: ");
            if (activeChannels.count(id)) {
                std::cout << "Pause (p) o Resume (r)? ";
                char pauseChar;
                std::cin >> pauseChar;
                clearInputBuffer();
                bool pause = (pauseChar == 'p' || pauseChar == 'P');
                if (audioManager.SetChannelPaused(id, pause)) {
                    std::cout << "Channel " << id << (pause ? " paused." : " resumed.") << "\n";
                }
                else {
                    std::cerr << "Error setting pause/resume by channel " << id << ".\n";
                }
            }
            else {
                std::cerr << "Channel ID " << id << " not found or no longer active.\n";
            }
            break;
        }
        case 8: // Imposta Volume Canale
        {
            ChannelId id = getNumericInput<ChannelId>("Insert Channel ID: ");
            if (activeChannels.count(id)) {
                float volume = getNumericInput<float>("New volume (0.0-1.0): ");
                if (audioManager.SetChannelVolume(id, volume)) {
                    std::cout << "Channel volume " << id << " set to " << volume << ".\n";
                }
                else {
                    std::cerr << "Error setting volume per channel " << id << ".\n";
                }
            }
            else {
                std::cerr << "Channel ID " << id << " not found or no longer active.\n";
            }
            break;
        }
        case 9: // Imposta Pan Canale
        {
            ChannelId id = getNumericInput<ChannelId>("Insert Channel ID: ");
            if (activeChannels.count(id)) {
                float pan = getNumericInput<float>("New PAN (-1.0 a 1.0): ");
                if (audioManager.SetChannelPan(id, pan)) {
                    std::cout << "Channel PAN " << id << " set to " << pan << ".\n";
                }
                else {
                    std::cerr << "Error setting pan per channel " << id << ".\n";
                }
            }
            else {
                std::cerr << "Channel ID " << id << " not found or no longer active.\n";
            }
            break;
        }
        case 10: // Mostra Canali Attivi
        {
            std::cout << "\n--- Active Channels ---\n";
            std::vector<ChannelId> toRemove;
            for (const auto& pair : activeChannels) {
                if (!audioManager.IsChannelPlaying(pair.first)) {
                    toRemove.push_back(pair.first);
                }
            }
            for (ChannelId idToRemove : toRemove) {
                activeChannels.erase(idToRemove);
            }

            if (activeChannels.empty()) {
                std::cout << "(No active channels)\n";
            }
            else {
                for (const auto& pair : activeChannels) {
                    std::cout << "ID: " << pair.first << " - Sound: " << pair.second.soundName << "\n";
                }
            }
            std::cout << "---------------------\n";
            break;
        }
        case 0: // Esci
        {
            running = false;
            break;
        }
        default:
            std::cerr << "Invalid option.\n";
            break;
        }
        if (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    // 8. Shutdown
    std::cout << "\AudioManager shutdown...\n";
    audioManager.Shutdown();
    std::cout << "Demo finished.\n";

    return 0;
}