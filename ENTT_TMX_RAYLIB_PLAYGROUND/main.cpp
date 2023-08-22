#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <vector>
#include <string>
#include "entt.hpp"

//tmx includes
#include <tmxlite/Map.hpp>
#include <tmxlite/Layer.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ObjectGroup.hpp>

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

enum class CollisionType {
    Player,
    Wall,
    Enemy
};

struct TransformComponent {
    Vector2 position;
    float speed;
};

struct DrawComponent {
    Vector2 size;
    Color color;
};

struct CollisionComponent {
    Rectangle bounds;
    CollisionType collision_type;
};

struct PlayerComponent {
    bool isPlayer;
};

struct EnemyComponent {
    string name;
    int damage;
};

// Function to draw a single tile
void DrawTile(Texture2D tileset, int tileID, int posX, int posY, int tileWidth, int tileHeight)
{
    // Calculate the position of the tile in the tileset texture based on its tile ID
    int tileX = ((tileID - 1) % (tileset.width / tileWidth)) * tileWidth;
    int tileY = ((tileID - 1) / (tileset.width / tileWidth)) * tileHeight;

    Rectangle sourceRect = { (float)tileX, (float)tileY, (float)tileWidth, (float)tileHeight };
    Rectangle destRect = { (float)posX, (float)posY, (float)tileWidth, (float)tileHeight };

    // Draw the tile using your graphics library's functions (e.g., Raylib's DrawTextureRec)
    DrawTextureRec(tileset, sourceRect, Vector2 {destRect.x, destRect.y}, WHITE);
}

bool HandleDrawingMap(const tmx::Map& map, const Texture2D& tileset, entt::registry &registry, bool mapDrawn)
{
    const auto tileSize = map.getTileSize();
    const int width = map.getTileCount().x;
    const int height = map.getTileCount().y;

    const int tileWidthWithSpacing = tileSize.x + 0; // Adjust the spacing based on your tileset
    const int tileHeightWithSpacing = tileSize.y + 0; // Adjust the spacing based on your tileset
    const int marginX = 0; // Adjust the margin based on your tileset
    const int marginY = 0; // Adjust the margin based on your tileset

    // Render tile layers
    for (const auto& layer : map.getLayers())
    {
        if (layer->getType() == tmx::Layer::Type::Tile)
        {
            const auto& tileLayer = layer->getLayerAs<tmx::TileLayer>();
            const auto& tiles = tileLayer.getTiles();

            // Iterate through the tiles within the layer's dimensions
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    const auto& tile = tiles[(y * width) + x];
                    if (tile.ID != 0)
                    {
                        // Calculate the position of the tile on the screen
                        int tileX = (x * tileWidthWithSpacing) + marginX;
                        int tileY = (y * tileHeightWithSpacing) + marginY;

                        if (!mapDrawn && layer->getName() == "Walls")
                        {
                            const auto& props = layer->getProperties();
                            if (props[0].getBoolValue())
                            {
                                auto collisionBox = registry.create();
                                registry.emplace<CollisionComponent>(collisionBox, Rectangle{(float)tileX, (float)tileY, (float)tileSize.x, (float)tileSize.y}, CollisionType::Wall);
                                registry.emplace<DrawComponent>(collisionBox, Vector2{ (float)tileSize.x, (float)tileSize.y }, RED);
                                registry.emplace<TransformComponent>(collisionBox, Vector2{ (float)tileX, (float)tileY }, 0.f);
                            }
                            cout << "Drawing tile entities" << endl;
                        }

                        // Draw the tile at the calculated position
                        DrawTile(tileset, tile.ID, tileX, tileY, tileSize.x, tileSize.y);
                    }
                }
            }
        }
    }
    return true;
}



void MovementSystem(entt::registry& registry, float deltaTime)
{
    int YMovement = IsKeyDown(KEY_S) - IsKeyDown(KEY_W);
    int XMovement = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);

    auto playerEntities = registry.view<TransformComponent, PlayerComponent>();

    auto v = Vector2Normalize(Vector2{(float)XMovement, (float)YMovement});

    //Wanted to experiment with the lambda
    playerEntities.each([v, deltaTime](TransformComponent& transform, PlayerComponent& player)
    {
        if(player.isPlayer)
            transform.position.y += v.y * transform.speed * deltaTime;
            transform.position.x += v.x * transform.speed * deltaTime;
    });
}

void DrawSystem(entt::registry& registry)
{
    auto drawableEntities = registry.view<DrawComponent, TransformComponent, CollisionComponent>();

    for (auto ent : drawableEntities)
    {
        auto &transform = drawableEntities.get<TransformComponent>(ent);
        auto &draw = drawableEntities.get<DrawComponent>(ent);
        auto &coll = drawableEntities.get<CollisionComponent>(ent);

        //Update collision bounds tooo
        coll.bounds.x = transform.position.x;
        coll.bounds.y = transform.position.y;
        coll.bounds.height = draw.size.y;
        coll.bounds.width = draw.size.x;

        DrawRectangle(transform.position.x, transform.position.y, draw.size.x, draw.size.y, draw.color);
    }
}

int collisionTimer = 0;
void CollisionSystem(entt::registry& registry)
{
    auto playerView = registry.view<CollisionComponent, PlayerComponent>();
    auto otherEntView = registry.view<CollisionComponent>(entt::exclude<PlayerComponent>);
    auto enemyView = registry.view<EnemyComponent>();

    for (entt::entity ent : playerView)
    {
        auto& collision_bounds_1 = playerView.get<CollisionComponent>(ent);

        for (auto ent2 : otherEntView)
        {
            auto& collision_bounds_2 = otherEntView.get<CollisionComponent>(ent2);
            if (ent != ent2)
            {
                if (CheckCollisionRecs(collision_bounds_1.bounds, collision_bounds_2.bounds))
                {
                    switch (collision_bounds_2.collision_type)
                    {
                        case CollisionType::Player:
                        {
                            cout << "Player collision detected" << endl;
                            break;
                        }
                         
                        case CollisionType::Enemy:
                        {
                            auto& eComp = enemyView.get<EnemyComponent>(ent2);
                            cout << "Enemy collision detected: " << "His name is " << eComp.name << " and he does " << eComp.damage << " damage to you" << endl;
                            break;
                        }

                        case CollisionType::Wall:
                        {
                            cout << "Wall collision detected" << endl;
                            break;
                        }
                        default:
                            break;
                    }
                    ++collisionTimer;
                    cout << "Collision Detected " << collisionTimer << " times" << endl;
                }
            }
        }
    }
}

int main()
{
    InitWindow(640, 640, "Entt Raylib TMXLite Example");
    SetWindowState(FLAG_VSYNC_HINT);
    SetTargetFPS(60);

    //Toggle between different states
    GameState gameState = GameState::Menu;

    int playerWon = 0; //Which player won state

    //Tilemap texture
    Texture2D tilemap_texture = LoadTexture("resources/dungeon-example/dungeon_tileset.png");

    tmx::Map map;
    map.load("resources/dungeon-example/dungeon.tmx");

    //Create registry
    entt::registry registry;

    //Create entities
    entt::entity playerLeftEntity = registry.create();
    registry.emplace<TransformComponent>(playerLeftEntity, Vector2{10.f, 10.f}, 100.f);
    registry.emplace<DrawComponent>(playerLeftEntity, Vector2{10.f, 10.f}, WHITE);
    registry.emplace<CollisionComponent>(playerLeftEntity, Rectangle{10, 10, 10, 10}, CollisionType::Player);
    registry.emplace<PlayerComponent>(playerLeftEntity, true);
    
    //Create basic enemy
    entt::entity enemyEntity = registry.create();
    registry.emplace<TransformComponent>(enemyEntity, Vector2{100.f, 100.f}, 10.f);
    registry.emplace<DrawComponent>(enemyEntity, Vector2{10.f, 10.f}, RED);
    registry.emplace<CollisionComponent>(enemyEntity, Rectangle{10, 10, 10, 10}, CollisionType::Enemy);
    registry.emplace<EnemyComponent>(enemyEntity, "Steve-o", 32);

    bool mapDrawn = false;

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

            mapDrawn = HandleDrawingMap(map, tilemap_texture, registry, mapDrawn);
            MovementSystem(registry, dt);
            DrawSystem(registry);
            CollisionSystem(registry);


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