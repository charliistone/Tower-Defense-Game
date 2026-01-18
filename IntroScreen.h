#ifndef INTRO_SCREEN_H
#define INTRO_SCREEN_H

#include "raylib.h"

class IntroScreen {
public:
    void Init();
    void Update();
    void Draw();
    void Unload();
    bool IsFinished() const;

private:
    Texture2D parchment;
    Font storyFont;
    bool finished;
    const char* storyText;
};

#endif
