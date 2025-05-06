
# 🎧 FMOD Wrapper by Stefano Gambarin

A lightweight wrapper around FMOD Studio API for managing sound loading, playback, and channels in a clean and flexible way.

## 🔧 Features

- Init System
- Load (static e straming)
- Play, Pause
- Stop
- Loop e one-shot sounds
- Pan (Left/Right)
- Volume
- Audio Channel


## 📁 Supported Audio Formats
Supported file extensions are:

- .wav
- .mp3
- .ogg
- .flac
- .aiff


## ▶️ Commands Manual

Here’s a functional overview of the available operations:

1. **Load Sound (Sample/Static)**  
   Load a sound file into memory for low-latency playback (ideal for short sound effects).

2. **Load Sound (Stream)**  
   Load a sound file as a stream from disk (ideal for long music tracks).

3. **Load Sound from folder**  
   Scan a folder and load all supported audio files found inside.

4. **Unload Sound**  
   Unload a previously loaded sound by its channel ID.

5. **Play Sound**  
   Play a loaded sound and get back a channel ID to control it.

6. **Stop Channel**  
   Stop a specific sound by its Channel ID.

7. **Pause/Play Channel**  
   Pause or resume a sound via its Channel ID.

8. **Set Channel Volume**  
   Set volume (0.0 to 1.0) for a given Channel ID.

9. **Set Channel Pan**  
   Set stereo pan (-1.0 left to 1.0 right) for a Channel ID.

10. **Show Active Channel**  
    Display the ID of the most recently played sound channel.

0. **Exit**  
    Close the application and shut down the audio system.