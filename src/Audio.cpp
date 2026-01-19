#include "Audio.h"


std::map<std::string, Sound> Audio::sounds;
std::map<std::string, Music> Audio::musics;
Music* Audio::currentMusic = nullptr;

void Audio::Init() {
    InitAudioDevice(); 
}
/* RESOURCE CLEANUP :
 Iterates through all loaded sounds and music streams to unload them from memory
 before closing the audio device. This prevents memory leaks upon game exit.*/
void Audio::Close() {
    
    for (auto& pair : sounds) UnloadSound(pair.second);
    for (auto& pair : musics) UnloadMusicStream(pair.second);
    sounds.clear();
    musics.clear();
    CloseAudioDevice();
}

/* MUSIC STREAMING UPDATE :
 Raylib's music system requires this function to be called every frame.
 It refills the audio buffer; without this, music would play for a fraction of a second and stop.*/
void Audio::Update() {
    if (currentMusic != nullptr) {
        UpdateMusicStream(*currentMusic);
    }
}

void Audio::LoadSFX(std::string name, const char* path) {
    Sound snd = LoadSound(path);
    sounds[name] = snd;
}

void Audio::LoadMusic(std::string name, const char* path) {
    Music mus = LoadMusicStream(path);
    mus.looping = true;
    musics[name] = mus;
}

void Audio::PlaySFX(std::string name, float volume, float pitch) {
    if (sounds.find(name) != sounds.end()) {
        /* SFX PLAYBACK WITH MODIFIERS :
         Sets specific volume and pitch (speed) for the sound effect instance before playing it.
         This allows for variation (e.g., random pitch for footsteps) using a single asset
         to reduce auditory repetition.*/
        SetSoundVolume(sounds[name], volume); 
        SetSoundPitch(sounds[name], pitch);  
        PlaySound(sounds[name]);
    }
}

void Audio::PlayMusic(std::string name) {
    if (musics.find(name) != musics.end()) {
        /* BACKGROUND MUSIC SWITCHING :
         Checks if the requested track is different from the currently playing one.
         If it is, it stops the old track and starts the new one. This prevents the 
         music from restarting from the beginning if this function is called multiple times.*/
        if (currentMusic != &musics[name]) {
            if (currentMusic) StopMusicStream(*currentMusic);
            currentMusic = &musics[name];
            PlayMusicStream(*currentMusic);

            
            ::SetMusicVolume(*currentMusic, 0.5f);
        }
    }
}

void Audio::StopMusic() {
    if (currentMusic) StopMusicStream(*currentMusic);
}

void Audio::SetMusicVolume(float volume) {
    if (currentMusic) {
        
        ::SetMusicVolume(*currentMusic, volume);
    }
}
