#include "IntroScreen.h"

void IntroScreen::Init()
{
    parchment = LoadTexture("assets/Intro/parchment.png");
    storyFont = LoadFontEx("assets/Intro/OctubreMedieval.otf", 48, 0, 0);

    storyText =
        "The fortress stands alone at the edge of the realm.\n"
        "Its walls have endured countless winters,\n"
        "but never a night like this.\n\n"
        "From the darkened lands beyond the horizon,\n"
        "the enemy advances without mercy.\n"
        "War drums echo through the valley,\n"
        "and the ground trembles beneath their march.\n\n"
        "There will be no reinforcements.\n"
        "No heroes riding at dawn.\n"
        "Only stone, steel, and the will to hold the line.\n\n"
        "Defenses are raised as the first fires appear.\n"
        "Towers take shape along the walls.\n"
        "Every shot must count.\n"
        "Every moment must be earned.\n\n"
        "The siege begins now.\n"
        "The fate of the fortress rests in your command.";
    finished = false;
}

void IntroScreen::Update()
{
    if (IsKeyPressed(KEY_ENTER))
    {
        finished = true;
    }
}

void IntroScreen::Draw()
{
    ClearBackground(BLACK);
    Rectangle src = { 0.0f, 0.0f, (float)parchment.width, (float)parchment.height };
    Rectangle dst = { -50.0f, -50.0f, 1280.0f + 100.0f, 720.0f + 100.0f };
    Vector2 origin = { 0.0f, 0.0f };
    DrawTexturePro(parchment, src, dst, origin, 0.0f, WHITE);
    Vector2 textPos = { 180.0f, 60.0f };

    DrawTextEx(
        storyFont,
        storyText,
        textPos,
        30.0f,
        1.5f,
        DARKBROWN

    );

    DrawText(
        "Press ENTER to continue",
        1280 / 2 - 110,
        720 - 25,
        20,
        DARKGRAY
    );
}


void IntroScreen::Unload()
{
    UnloadFont(storyFont);
    UnloadTexture(parchment);
}


bool IntroScreen::IsFinished() const
{
    return finished;
}