#pragma once
#include "raylib.h"
#include <map>
#include <string>

class Audio {
public:
    static void Init();
    static void Close();
    static void Update();

    static void LoadSFX(std::string name, const char* path);
    static void LoadMusic(std::string name, const char* path);

    
    static void PlaySFX(std::string name, float volume = 1.0f, float pitch = 1.0f);

    static void PlayMusic(std::string name);
    static void StopMusic();

    static void SetMusicVolume(float volume);

private:
    static std::map<std::string, Sound> sounds;
    static std::map<std::string, Music> musics;
    static Music* currentMusic;
};
