#include "raylib.h"
#include <iostream>
#include <vector>
#include <string>
#include "entt.hpp"

//ope
using namespace std;

//Handle menu systems
enum class GameState {
    Menu,
    Play,
    Pause,
    Win,
    Restart
};

struct TransformComponent {
    Vector2 position;
    float speed;
};

struct PaddleComponent
{
    int playerNumber;
};

struct DrawComponent {
    Vector2 size;
    Color color;
};

void MovementSystem(entt::registry& registry, float deltaTime)
{
    int YMovement = IsKeyDown(KEY_S) - IsKeyDown(KEY_W);
    auto playerEntities = registry.view<TransformComponent, PaddleComponent>();

    //Wanted to experiment with the lambda
    playerEntities.each([YMovement, deltaTime](TransformComponent& transform, PaddleComponent& paddle)
    {
        if(paddle.playerNumber == 1)
            transform.position.y += YMovement * transform.speed * deltaTime;
    });
}

void DrawSystem(entt::registry& registry)
{
    auto drawableEntities = registry.view<DrawComponent, TransformComponent>();

    for (auto ent : drawableEntities)
    {
        auto &transform = drawableEntities.get<TransformComponent>(ent);
        auto &draw = drawableEntities.get<DrawComponent>(ent);

        DrawRectangle(transform.position.x, transform.position.y, draw.size.x, draw.size.y, draw.color);
    }
}

int main()
{
    InitWindow(800, 600, "Pong");
    SetWindowState(FLAG_VSYNC_HINT);
    SetTargetFPS(60);

    //Toggle between different states
    GameState gameState = GameState::Menu;

    int playerWon = 0; //Which player won state

    //Create registry
    entt::registry registry;

    //Create entities
    entt::entity playerLeftEntity = registry.create();
    registry.emplace<TransformComponent>(playerLeftEntity, Vector2{10.f, 10.f}, 100.f);
    registry.emplace<PaddleComponent>(playerLeftEntity, 1);
    registry.emplace<DrawComponent>(playerLeftEntity, Vector2{10.f, 100.f}, WHITE);


    while (!WindowShouldClose())
    {

        float dt = GetFrameTime();

        //I would have made a helper function for this so I can just render stuff, but I'll do that later
        string startText = "Press Start to Begin Play";
        int startTextWidth = MeasureText(startText.c_str(), 30) / 2;

        string pauseText = "Press Start to Resume";
        int pauseTextWidth = MeasureText(startText.c_str(), 30) / 2;

        /*
        string wonText = "PLAYER " + to_string(playerWon) + " WON!";
        int wonTextWidth = MeasureText(wonText.c_str(), 30) / 2;

        string retartText = "Press Start to Restart";
        int restartTextWidth = MeasureText(retartText.c_str(), 30) / 2;
        */


        BeginDrawing();
        ClearBackground(BLACK);

        switch (gameState)
        {
        case GameState::Menu:
            DrawText(startText.c_str(), (GetScreenWidth() / 2) - startTextWidth, GetScreenHeight() / 2, 30, WHITE);
            if (IsKeyDown(KEY_ENTER) || IsKeyDown(GAMEPAD_BUTTON_MIDDLE_RIGHT))
                gameState = GameState::Play;

            break;
        case GameState::Play: //Draw gameplay

            MovementSystem(registry, dt);
            DrawSystem(registry);

            //Show pause screen
            if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(GAMEPAD_BUTTON_MIDDLE_RIGHT))
            {
                gameState = GameState::Pause;
            }

            break;
        case GameState::Pause:
            //Draw Pause Menu
            DrawText(pauseText.c_str(), (GetScreenWidth() / 2) - pauseTextWidth, GetScreenHeight() / 2, 30, WHITE);
            if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(GAMEPAD_BUTTON_MIDDLE_RIGHT))
            {
                gameState = GameState::Play;
            }
            break;
        case GameState::Win:
            if (IsKeyDown(KEY_ENTER) || IsKeyDown(GAMEPAD_BUTTON_MIDDLE_RIGHT))
                gameState = GameState::Restart;
            break;
        case GameState::Restart:
            //TODO: Add restart logic

            //Go back to play state.
            gameState = GameState::Play;
            break;
        default:
            break;
        }

        EndDrawing();
    }

    CloseWindow();
}