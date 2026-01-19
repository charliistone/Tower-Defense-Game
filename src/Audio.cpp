#include "Audio.h"


std::map<std::string, Sound> Audio::sounds;
std::map<std::string, Music> Audio::musics;
Music* Audio::currentMusic = nullptr;

void Audio::Init() {
    InitAudioDevice(); 
}

void Audio::Close() {
    
    for (auto& pair : sounds) UnloadSound(pair.second);
    for (auto& pair : musics) UnloadMusicStream(pair.second);
    sounds.clear();
    musics.clear();
    CloseAudioDevice();
}

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
        SetSoundVolume(sounds[name], volume); 
        SetSoundPitch(sounds[name], pitch);  
        PlaySound(sounds[name]);
    }
}

void Audio::PlayMusic(std::string name) {
    if (musics.find(name) != musics.end()) {
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
