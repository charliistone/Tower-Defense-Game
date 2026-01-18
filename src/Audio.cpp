#include "Audio.h"

// Statik değişkenleri tanımla
std::map<std::string, Sound> Audio::sounds;
std::map<std::string, Music> Audio::musics;
Music* Audio::currentMusic = nullptr;

void Audio::Init() {
    InitAudioDevice(); // Raylib ses motorunu başlat
}

void Audio::Close() {
    // Tüm sesleri bellekten sil
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
        SetSoundVolume(sounds[name], volume); // Ses şiddeti
        SetSoundPitch(sounds[name], pitch);   // Ses inceliği
        PlaySound(sounds[name]);
    }
}

void Audio::PlayMusic(std::string name) {
    if (musics.find(name) != musics.end()) {
        if (currentMusic != &musics[name]) {
            if (currentMusic) StopMusicStream(*currentMusic);
            currentMusic = &musics[name];
            PlayMusicStream(*currentMusic);

            // --- HATA BURADAYDI, DÜZELTİLDİ ---
            // Başına :: koyarak "Raylib'in fonksiyonunu kullan" dedik
            ::SetMusicVolume(*currentMusic, 0.5f);
        }
    }
}

void Audio::StopMusic() {
    if (currentMusic) StopMusicStream(*currentMusic);
}

void Audio::SetMusicVolume(float volume) {
    if (currentMusic) {
        // Burada da Raylib fonksiyonunu çağırmak için :: kullanıyoruz
        ::SetMusicVolume(*currentMusic, volume);
    }
}