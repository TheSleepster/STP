/* ========================================================================
   $File: STP_Entry.cpp $
   $Date: December 09 2024 03:18 pm $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#include <raylib.h>
#include "Intrinsics.h"

#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/String.h"
#include "util/Pairs.h"
#include "util/Sorting.h"
#include "util/Arena.h"

global_variable real32 DeltaTime;

constexpr uint32 MAX_ENTITIES = 1000;

constexpr uint32 IterationCounter = 2;
constexpr real32 TickRate = 1 / IterationCounter;
constexpr real32 InAirFriction = 2.0f;

constexpr real32 EPSILON = 0.04f;

enum physics_body_type
{
    PB_Dynamic,
    PB_Static,
};

enum game_layering
{
    LAYER_Player,
};

enum entity_flags
{
    IS_VALID    = 1 << 0,
    IS_GRAVITIC = 1 << 1,
    FlagCount
};

enum entity_arch
{
    ARCH_Null,
    ARCH_PLAYER,
    ARCH_MAP_WALL,
    ARCH_MAP_FLOOR,
    ARCH_Count
};

struct game_memory
{
    int32        TransientStorageSize;
    memory_block TransientStorage;

    int32        PermanentStorageSize;
    memory_block PermanentStorage;
};

struct aabb
{
    vec2 Position;
    vec2 HalfSize;

    vec2 Min;
    vec2 Max;
};

struct physics_body
{
    int32  BodyType;
    
    vec2   Velocity;
    vec2   Acceleration;
    real32 Friction;

    bool32 IsGrounded;
    bool32 IsColliding;

    aabb   CollisionRect;
};

struct entity
{
    entity_arch  Archetype;
    uint32       Flags;
    uint32       EntityID;
    int32        LayerIndex;

    vec2         Position;
    
    vec2         RenderSize;
    real32       MovementSpeed;

    bool32       IsGrounded;

    physics_body PhysicsBodyData;
};

struct collision_data
{
    bool32  Collision;
    vec2    Position;
    vec2    Depth;
    vec2    Normal;
    real32  Time;
 
    entity *CollidedEntity;
};

struct game_state
{
    ivec2        WindowSizeData;
    Camera2D     SceneCamera;
    real32       Gravity;
    real32       MaxG;
    memory_arena GameArena;

    entity      *Entities;
};

internal void
ProcessMovement(entity *Player)
{
    if(IsKeyDown(KEY_W))
    {
        Player->PhysicsBodyData.Acceleration.Y =  1.0f;
    }

    if(IsKeyDown(KEY_S))
    {
        Player->PhysicsBodyData.Acceleration.Y = -1.0f;
    }

    if(IsKeyDown(KEY_A))
    {
        Player->PhysicsBodyData.Acceleration.X =  1.0f;
    }

    if(IsKeyDown(KEY_D))
    {
        Player->PhysicsBodyData.Acceleration.X = -1.0f;
    }
}

internal entity *
CreateEntity(game_state *GameState)
{
    entity *Result = {};
    for(uint32 Index = 0;
        Index < MAX_ENTITIES;
        ++Index)
    {
        entity *Found = &GameState->Entities[Index];
        if((Found->Flags & IS_VALID) == 0)
        {
            Found->Flags |= IS_VALID;
            Result = Found;
            Result->EntityID = Index;
            Assert(Result);
            break;
        }
    }
    return(Result);
}

internal void
SetupEntityPlayer(entity *Entity)
{
    Entity->Archetype     = ARCH_PLAYER;
    Entity->MovementSpeed = 6000;
    Entity->RenderSize    = {32, 32};
    Entity->LayerIndex    = LAYER_Player;

    Entity->Flags |= IS_GRAVITIC;

    Entity->PhysicsBodyData.BodyType = PB_Dynamic;
    Entity->PhysicsBodyData.CollisionRect.HalfSize = Entity->RenderSize * 0.5f;
}

internal void
SetupEntityFloorTile(entity *Entity)
{
    Entity->Archetype  = ARCH_MAP_FLOOR;
    Entity->RenderSize = {400, 10};
    Entity->LayerIndex    = LAYER_Player;

    Entity->PhysicsBodyData.BodyType = PB_Static;
    Entity->PhysicsBodyData.CollisionRect.HalfSize = Entity->RenderSize * 0.5f;
    Entity->PhysicsBodyData.Friction = 12.0f;
}

internal vec2 
CalculateCollisionDepth(aabb TestBox)
{
    vec2 Result = {};
    
    real32 MinDist = fabsf(TestBox.Min.X);
    Result.X = MinDist;
    Result.Y = 0;

    if(fabsf(TestBox.Max.X) < MinDist)
    {
        MinDist = fabsf(TestBox.Max.X);
        Result.X = TestBox.Max.X;
    }

    if(fabsf(TestBox.Min.Y) < MinDist)
    {
        MinDist = fabsf(TestBox.Min.Y);
        
        Result.X = 0;
        Result.Y = TestBox.Min.Y;
    }

    if(fabsf(TestBox.Max.Y) < MinDist)
    {
        Result.X = 0;
        Result.Y = TestBox.Max.Y;
    }

    return(Result);
}

internal inline bool32
IsWithinBoundsAABB(vec2 Point, aabb AABB)
{
    return(Point.X >= AABB.Min.X && Point.X <= AABB.Max.X &&
           Point.Y >= AABB.Min.Y && Point.Y <= AABB.Max.Y);
}


internal inline aabb
CalculateMinkowskiDifference(aabb A, aabb B)
{
    aabb Result = {};
    Result.Position = A.Position - B.Position;
    Result.HalfSize = A.HalfSize + B.HalfSize;

    Result.Min = Result.Position - Result.HalfSize;
    Result.Max = Result.Position + Result.HalfSize;

    return(Result);
}

internal bool32
IsCollision(aabb A, aabb B)
{
    aabb Test = CalculateMinkowskiDifference(A, B);

    return(0 >= Test.Min.X && 0 <= Test.Max.X &&
           0 >= Test.Min.Y && 0 <= Test.Max.Y);
}

internal collision_data
RaycastIsColliding(vec2 Position, vec2 Magnitude, aabb AABB)
{
    collision_data Result = {};
    real32 LastEntry = -INFINITY;
    real32 FirstExit =  INFINITY; 

    vec2 Min = AABB.Position - AABB.HalfSize;
    vec2 Max = AABB.Position + AABB.HalfSize;

    for(uint32 DimensionIndex = 0;
        DimensionIndex < 2;
        ++DimensionIndex)
    {
        if(Magnitude[DimensionIndex] != 0)
        {
            real32 T1 = (Min[DimensionIndex] - Position[DimensionIndex]) / Magnitude[DimensionIndex];
            real32 T2 = (Max[DimensionIndex] - Position[DimensionIndex]) / Magnitude[DimensionIndex];

            LastEntry = fmaxf(LastEntry, fminf(T1, T2));
            FirstExit = fminf(FirstExit, fmaxf(T1, T2));
        }
        else if(Position[DimensionIndex] <= Min[DimensionIndex] || Position[DimensionIndex] >= Max[DimensionIndex])
        {
            return(Result);
        }
    }

    if(FirstExit > LastEntry && FirstExit > 0 && FirstExit < 1)
    {
        return(Result);
    }

        Result.Position[0]  = Position[0] + Magnitude[0] * LastEntry;
        Result.Position[1]  = Position[1] + Magnitude[1] * LastEntry;

        Result.Time      = LastEntry;
        Result.Collision = true;

        real32 dX = Result.Position[0] - AABB.Position[0];
        real32 dY = Result.Position[1] - AABB.Position[1];
        real32 pX = AABB.HalfSize[0] - fabsf(dX);
        real32 pY = AABB.HalfSize[1] - fabsf(dY);

        if(pX < pY)
        {
            Result.Normal[0] = real32(dX > 0.0f) - real32(dX < 0.0f);
        }
        else
        {
            Result.Normal[1] = real32(dY > 0.0f) - real32(dY < 0.0f);
        }
    
    return(Result);
}

internal collision_data
SweepAndRespond(game_state *GameState, entity *Entity) 
{
    collision_data CollisionData = {};
    CollisionData.Time = 0xBEEF;

    for(uint32 EntityIndex = 0;
        EntityIndex < MAX_ENTITIES;
        ++EntityIndex)
    {
        entity *TestEntity = &GameState->Entities[EntityIndex];
        if(((TestEntity->Flags & IS_VALID) != 0) && TestEntity->EntityID != Entity->EntityID)
        {
            physics_body *Body = &Entity->PhysicsBodyData;
            physics_body *BodyToTest = &TestEntity->PhysicsBodyData;
            
            vec2 Magnitude = BodyToTest->CollisionRect.Position - Body->CollisionRect.Position;
            aabb MinkowskiAABB  = CalculateMinkowskiDifference(Body->CollisionRect, BodyToTest->CollisionRect);
            collision_data Data = RaycastIsColliding(Entity->Position, Magnitude, MinkowskiAABB);
            if(Data.Collision)
            {
                if(Data.Normal.X != 0)
                {
                    Entity->Position.X -= Body->Velocity.X * DeltaTime;
                    Body->Velocity.X = 0;
                }
                else if(Data.Normal.Y != 0)
                {
                    Entity->Position.Y -= Body->Velocity.Y * DeltaTime;
                    Body->Velocity.Y = 0;
                }

                Data.CollidedEntity = TestEntity;
            }
        }
    }
    return(CollisionData);
}

internal void
UpdateEntityPhysicsBodyData(game_state *GameState, entity *Entity)
{
    physics_body *Body = &Entity->PhysicsBodyData;
    if(Body && Body->BodyType == PB_Dynamic)
    {
        if(!Entity->IsGrounded)
        {
            Body->Friction = InAirFriction; 
        }
        Body->Acceleration *= Entity->MovementSpeed;
        Body->Acceleration += -Body->Velocity * Body->Friction;

        vec2 OldEntityV   = Body->Velocity;
        Entity->Position  = 0.5f * Body->Acceleration * (DeltaTime * DeltaTime) + (Body->Velocity * DeltaTime) + Entity->Position;
        Body->Velocity    = (Body->Acceleration * DeltaTime) + OldEntityV;

        Body->CollisionRect.Position = Entity->Position;
        Body->CollisionRect.Min = Body->CollisionRect.Position - Body->CollisionRect.HalfSize;
        Body->CollisionRect.Max = Body->CollisionRect.Position + Body->CollisionRect.HalfSize;

        Body->Acceleration = vec2{0};

        for(uint32 IterationIndex = 0;
            IterationIndex < IterationCounter;
            ++IterationIndex)
        {
            SweepAndRespond(GameState, Entity);
        }
    }
}

internal void
DrawEntity(entity *Entity, Color DrawColor)
{
    DrawRectangle((int)Entity->Position.X - int(Entity->RenderSize.X * 0.5f),
                  (int)Entity->Position.Y - int(Entity->RenderSize.Y * 0.5f),
                  (int)Entity->RenderSize.X,
                  (int)Entity->RenderSize.Y,
                  DrawColor);
}

int 
main()
{
    game_state  GameState = {};

    GameState.WindowSizeData = {1920, 1080};
    GameState.Gravity = -4;
    
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(GameState.WindowSizeData.X, GameState.WindowSizeData.Y, "Save The Prince");
    InitAudioDevice();

    // NOTE(Sleepster): Memory Setup
    {
        game_memory GameMemory = {};
        GameMemory.PermanentStorageSize = Megabytes(100); 
        GameMemory.PermanentStorage.MemoryBlock = malloc(GameMemory.PermanentStorageSize);
        GameMemory.PermanentStorage.BlockOffset = (uint8 *)GameMemory.PermanentStorage.MemoryBlock;

        GameMemory.TransientStorageSize = Megabytes(100); 
        GameMemory.TransientStorage.MemoryBlock = malloc(GameMemory.TransientStorageSize);
        GameMemory.TransientStorage.BlockOffset = (uint8 *)GameMemory.PermanentStorage.MemoryBlock;

        InitializeArena(&GameState.GameArena, Megabytes(50), &GameMemory.PermanentStorage);
        GameState.Entities = PushArray(&GameState.GameArena, entity, MAX_ENTITIES);
    }
    
    entity *Player = CreateEntity(&GameState);
    SetupEntityPlayer(Player);
    Player->Position.Y = 32;

    entity *Floor = CreateEntity(&GameState);
    SetupEntityFloorTile(Floor);

    while(!WindowShouldClose())
    {
        GameState.WindowSizeData.X = GetRenderWidth();
        GameState.WindowSizeData.Y = GetRenderHeight();
        
        GameState.SceneCamera.target = {0, 0};
        GameState.SceneCamera.offset = {real32(GameState.WindowSizeData.X * 0.5f), (real32)(GameState.WindowSizeData.Y * 0.5f)};
        // NOTE(Sleepster): Raylib by default is flipped, meaning a change in the Positive Y direction yields a downward movement.
        // This fixes that but it might introduce some bugs down the line so I'm just putting this here
        GameState.SceneCamera.zoom   = -2.0f;

        Vector2 RaylibMousePos = GetMousePosition();
        Vector2 RaylibWorldToScreenMousePos = GetScreenToWorld2D(RaylibMousePos, GameState.SceneCamera);
        vec2    MousePos = vec2{RaylibWorldToScreenMousePos.x, RaylibWorldToScreenMousePos.y};

        DeltaTime = GetFrameTime();

        ClearBackground(DARKGRAY);
        BeginDrawing();
        BeginMode2D(GameState.SceneCamera);

        for(uint32 EntityIndex = 0;
            EntityIndex < MAX_ENTITIES;
            ++EntityIndex)
        {
            entity *Temp = &GameState.Entities[EntityIndex];
            switch(Temp->Archetype)
            {
                case ARCH_PLAYER:
                {
                    ProcessMovement(Temp);
                    UpdateEntityPhysicsBodyData(&GameState, Temp);

                    vec2 Magnitude = Floor->Position - Temp->Position;
                    DrawEntity(Temp, RED);
                }break;
                default:
                {
                    UpdateEntityPhysicsBodyData(&GameState, Temp);
                    DrawEntity(Temp, ORANGE);
                }break;
            }
        }

        EndMode2D();
        EndDrawing();
    }
}
