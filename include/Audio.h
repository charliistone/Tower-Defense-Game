#pragma once
#include "raylib.h"
#include <map>
#include <string>

class Audio {
public:
    static void Init();
    static void Close();
    static void Update();

    // It loads sound effects and music into memory by naming them.
    static void LoadSFX(std::string name, const char* path);
    static void LoadMusic(std::string name, const char* path);

    // Plays a sound effect. By changing the pitch, the same sound can be produced in different tones.
    static void PlaySFX(std::string name, float volume = 1.0f, float pitch = 1.0f);

    static void PlayMusic(std::string name);
    static void StopMusic();

    static void SetMusicVolume(float volume);

private:
    static std::map<std::string, Sound> sounds;
    static std::map<std::string, Music> musics;

    // Holds the currently playing music. It's necessary to know which music to update in the Update() function.
    static Music* currentMusic;
};
