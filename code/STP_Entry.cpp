/* ========================================================================
   $File: STP_Entry.cpp $
   $Date: December 09 2024 03:18 pm $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#include <raylib.h>
#include <yyjson.h>

#include "Intrinsics.h"

#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/String.h"
#include "util/Pairs.h"
#include "util/Sorting.h"
#include "util/Arena.h"

typedef Sound           sound;
typedef Color           color;
typedef Texture2D       texture2d;
typedef Image           image2d;
typedef Music           music;
typedef AudioStream     audio_stream;
typedef Wave            wav_file;
typedef Sound           sound_data;
typedef Rectangle       rect;
typedef Vector2         rlvec2;
typedef Shader          shader;

global_variable real32 DeltaTime;

constexpr uint32 MAX_ENTITIES      = 10000;
constexpr uint32 IterationCounter  = 2;
constexpr real32 TickRate          = 1 / IterationCounter;
constexpr real32 InAirFrictionY    = 2.0f;
constexpr real32 InAirFrictionX    = 24.0f;
constexpr real32 EPSILON           = 0.2f;

constexpr uint32 GAME_WORLD_WIDTH  = 640;
constexpr uint32 GAME_WORLD_HEIGHT = 360;

constexpr ivec2 TILE_SIZE = {16, 16};

struct entity;
#define ENTITY_ON_COLLIDE_RESPONSE(name) void name(entity *A, entity *B)
typedef ENTITY_ON_COLLIDE_RESPONSE(entity_on_collide);

// NOTE(Sleepster): Similar to Celeste, Actors will respond with collisions while Solids will move no matter what
enum physics_body_type
{
    PB_Null,
    PB_Actor,
    PB_Solid,
};

enum game_layering
{
    LAYER_Background = 0,
    LAYER_Player = 0,
};

enum entity_flags
{
    IS_VALID    = 1 << 0,
    IS_GRAVITIC = 1 << 1,
    IS_PICKUP   = 1 << 2,
    FlagCount
};

enum entity_arch
{
    ARCH_Null,
    ARCH_PLAYER,
    ARCH_STROBBY,
    ARCH_TILE,
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
    
    vec2  Velocity;
    vec2  Acceleration;
    vec2  Friction;

    bool32 IsGrounded;
    bool32 IsGravitic;
    bool32 IsColliding;

    aabb   CollisionRect;
};

struct static_sprite_data
{
    ivec2 AtlasOffset;
    ivec2 SpriteSize;
};

struct animated_sprite_data
{
    ivec2  StartingSpriteOffset;
    ivec2  SpriteSize;

    int32  FrameCount;
    real32 FrameDuration;
};

struct timer
{
    real32 StartTime;
    real32 EndTime;
    
    real32 TimeElapsed;
    real32 TimerDuration;
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
    bool32       IsJumping;
    bool32       IsDashing;

    int32        MaxJumps;
    int32        JumpCounter;

    int32        MaxDashes;
    int32        DashCounter;
    
    timer        JumpTimer;
    timer        DashTimer;
    timer        CoyoteTimer;
    
    physics_body PhysicsBodyData;

    static_sprite_data StaticSprite;
    animated_sprite_data AnimatedSprite;

    entity_on_collide *OnCollide;
};

struct collision_data
{
    bool32  Collision;
    vec2    Position;
    vec2    Depth;
    ivec2   Normal;
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

    int32        ActiveTextureCount;
    texture2d    Textures[32];

    entity      *Entities;
    entity      *EntitySortingBuffer;
};

internal void
ProcessMovement(entity *Player)
{
    if(IsKeyDown(KEY_SPACE) && Player->IsGrounded && Player->JumpCounter < Player->MaxJumps)
    {
        Player->IsGrounded = false;
        Player->IsJumping  = true;
        Player->JumpCounter++;
    }

    if(IsKeyPressed(KEY_LEFT_SHIFT) &&
       !Player->IsGrounded &&
       fabsf(Player->PhysicsBodyData.Velocity.Y) > 12)
    {
        Player->IsDashing  = true;
        Player->DashCounter++;
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

internal void
DeleteEntity(entity *Entity)
{
    memset(Entity, 0, sizeof(entity));
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
    Entity->MovementSpeed = 3000;
    Entity->RenderSize    = {16, 16};
    Entity->LayerIndex    = LAYER_Player;

    Entity->JumpTimer.TimerDuration   = 0.15f;
    Entity->DashTimer.TimerDuration   = 0.08f;
    Entity->CoyoteTimer.TimerDuration = 0.15f;

    Entity->MaxJumps = 1;
    Entity->JumpCounter = 0;

    Entity->MaxDashes = 1;
    Entity->DashCounter = 0;

    Entity->Flags |= IS_GRAVITIC;

    Entity->PhysicsBodyData.BodyType = PB_Actor;
    Entity->PhysicsBodyData.CollisionRect.HalfSize = Entity->RenderSize * 0.5f;
}

internal void
SetupEntityFloorTile(entity *Entity)
{
    Entity->Archetype  = ARCH_MAP_FLOOR;
    Entity->RenderSize = {400, 10};
    Entity->LayerIndex    = LAYER_Player;

    Entity->PhysicsBodyData.BodyType = PB_Solid;
    Entity->PhysicsBodyData.CollisionRect.HalfSize = Entity->RenderSize * 0.5f;
    Entity->PhysicsBodyData.Friction = {12.0f, 0.0f};
}

internal void
SetupEntityWallTile(entity *Entity)
{
    Entity->Archetype  = ARCH_MAP_WALL;
    Entity->RenderSize = {10, 400};
    Entity->LayerIndex = LAYER_Player;

    Entity->PhysicsBodyData.BodyType = PB_Solid;
    Entity->PhysicsBodyData.CollisionRect.HalfSize = Entity->RenderSize * 0.5f;
}

internal void
SetupEntityStrobby(entity *Entity)
{
    Entity->Archetype  = ARCH_STROBBY;
    Entity->RenderSize = {16, 16};
    Entity->LayerIndex = LAYER_Player;

    Entity->PhysicsBodyData.BodyType = PB_Solid;
    Entity->PhysicsBodyData.CollisionRect.Position = Entity->Position;
    Entity->PhysicsBodyData.CollisionRect.HalfSize = Entity->RenderSize * 0.5f;
     
    Entity->Flags |= IS_PICKUP;
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

#include "STP_Map.cpp"

internal vec2 
CalculateCollisionDepth(aabb TestBox)
{
    vec2 Result = {};
    
    real32 MinDist = fabsf(TestBox.Min.X);
    Result.X = TestBox.Min.X;
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
DoesRayIntersect(vec2 Position, vec2 Magnitude, aabb SumAABB)
{
    collision_data Result = {};

    real32 LastEntry = -INFINITY;
    real32 FirstExit =  INFINITY;

    for(uint32 DI = 0;
        DI < 2;
        ++DI)
    {
        if(Magnitude[DI] != 0)
        {
            real32 Time1 = (SumAABB.Min[DI] - Position[DI]) / Magnitude[DI];
            real32 Time2 = (SumAABB.Max[DI] - Position[DI]) / Magnitude[DI];

            LastEntry = fmaxf(LastEntry, fminf(Time1, Time2));
            FirstExit = fminf(FirstExit, fmaxf(Time1, Time2));
        }
        else if(Position[DI] <= SumAABB.Min[DI] || Position[DI] >= SumAABB.Max[DI])
        {
            return(Result);
        }
    }

    if(FirstExit > LastEntry && FirstExit > 0 && LastEntry < 1)
    {
        Result.Position.X = Position.X + Magnitude.X * LastEntry;
        Result.Position.Y = Position.Y + Magnitude.Y * LastEntry;

        Result.Collision = true;
        Result.Time = LastEntry;

        real32 dx = Result.Position.X - SumAABB.Position.X;
        real32 dy = Result.Position.Y - SumAABB.Position.Y;

        real32 px = SumAABB.HalfSize.X - fabsf(dx);
        real32 py = SumAABB.HalfSize.Y - fabsf(dy);

        if(px < py)
        {
            Result.Normal.X = (dx > 0) ? 1 : -1;
        }
        else
        {
            Result.Normal.Y = (dy > 0) ? 1 : -1;
        }
    }
    
    return(Result);
}

internal collision_data
SweepEntitiesForCollision(game_state *GameState, entity *Entity, vec2 ScaledVelocity) 
{
    collision_data CollisionData = {};
    CollisionData.Time = 0xBEEF;

    physics_body *Body = &Entity->PhysicsBodyData;
    for(uint32 EntityIndex = 0;
        EntityIndex < MAX_ENTITIES;
        ++EntityIndex)
    {
        entity *TestEntity = &GameState->Entities[EntityIndex];
        if(((TestEntity->Flags & IS_VALID) != 0) && TestEntity->EntityID != Entity->EntityID)
        {
            physics_body *BodyToTest = &TestEntity->PhysicsBodyData;
            
            aabb SumAABB = CalculateMinkowskiDifference(BodyToTest->CollisionRect, Body->CollisionRect);
            collision_data Impact = DoesRayIntersect(Entity->Position, ScaledVelocity, SumAABB);
            if(Impact.Collision)
            {
                if(Impact.Time < CollisionData.Time)
                {
                    CollisionData = Impact;
                }
                else if(Impact.Time == CollisionData.Time)
                {
                    if(fabsf(ScaledVelocity.X) > fabsf(ScaledVelocity.Y) && Impact.Normal.X != 0)
                    {
                        CollisionData = Impact;
                    }
                    else if(fabsf(ScaledVelocity.Y) > fabsf(ScaledVelocity.X) && Impact.Normal.Y != 0)
                    {
                        CollisionData = Impact;
                    }
                }
            }
        }
    }

    if(CollisionData.Collision)
    {
        if(CollisionData.Normal.X != 0)
        {
            Entity->Position.X += ScaledVelocity.X;
        }
        else if(CollisionData.Normal.Y != 0)
        {
            Entity->Position.Y += ScaledVelocity.Y;
        }
        else
        {
            Entity->Position += ScaledVelocity;
        }
    }
    
    return(CollisionData);
}

internal collision_data 
EntityCollisionResponse(game_state *GameState, entity *Entity)
{
    collision_data Result = {};
    physics_body *Body = &Entity->PhysicsBodyData;
    for(uint32 EntityIndex = 0;
        EntityIndex < MAX_ENTITIES;
        ++EntityIndex)
    {
        entity *TestEntity = &GameState->Entities[EntityIndex];
        if(((TestEntity->Flags & IS_VALID) != 0) && TestEntity->EntityID != Entity->EntityID)
        {
            physics_body *TestBody = &TestEntity->PhysicsBodyData;
            aabb MinkowskiBody = CalculateMinkowskiDifference(TestBody->CollisionRect, Body->CollisionRect);
            
            // TODO(Sleepster): Might want to change this to support a variety of resolutions
            if(MinkowskiBody.Min.X <= 0 && MinkowskiBody.Max.X >= 0 && MinkowskiBody.Min.Y <= 0 && MinkowskiBody.Max.Y >= 0)
            {
                Result.Collision = true;
                Result.CollidedEntity = TestEntity;
                
                Body->Friction = TestBody->Friction;
                vec2 DepthVector = CalculateCollisionDepth(MinkowskiBody);
                Result.Depth = DepthVector;
                if(fabsf(DepthVector.Y) >= EPSILON * 2.0f)
                {
                    Entity->IsGrounded = true;
                }

                if(DepthVector.X != 0)
                {
                    Entity->Position.X += DepthVector.X;
                    Body->Velocity.X = 0;
                }
                else if(DepthVector.Y != 0)
                {
                    Entity->Position.Y += DepthVector.Y;
                    Body->Velocity.Y = 0;
                }
                break;
            }
            else if((MinkowskiBody.Max.Y - MinkowskiBody.Min.Y) > (EPSILON * 4))
            {
                Entity->IsGrounded = false;
                Body->Friction.Y = InAirFrictionY;
                Body->Friction.X = InAirFrictionX;
            }
        }
    }
    return(Result);
}

// TODO(Sleepster): Maybe make this "UpdatePlayerPhysicsBodyData"
internal void
UpdateEntityPhysicsBodyData(game_state *GameState, entity *Entity)
{
    physics_body *Body = &Entity->PhysicsBodyData;
    if(Body && Body->BodyType == PB_Actor)
    {
        if(((Entity->Flags & IS_GRAVITIC) != 0) && !Entity->IsGrounded)
        {
            Body->Velocity.Y += (GameState->Gravity * DeltaTime);
            if(Entity->CoyoteTimer.TimeElapsed < Entity->CoyoteTimer.TimerDuration)
            {
                Entity->CoyoteTimer.TimeElapsed += DeltaTime;
            }
        }
        else if(((Entity->Flags & IS_GRAVITIC) != 0) && Entity->IsGrounded)
        {
            Entity->CoyoteTimer.TimeElapsed = 0.0f;
        }
        
        Body->Acceleration *= Entity->MovementSpeed;
        Body->Acceleration += -Body->Velocity * Body->Friction;
        vec2 OldEntityV   = Body->Velocity;

        // NOTE(Sleepster): JUMPING 
        if(Entity->IsJumping)
        {
            Body->Friction.Y = InAirFrictionY;
            Body->Friction.X = InAirFrictionX;
            if(IsKeyReleased(KEY_SPACE))
            {
                Entity->IsJumping = false;
            }
            
            Entity->JumpTimer.TimeElapsed += DeltaTime;
            if(Entity->JumpTimer.TimeElapsed >= Entity->JumpTimer.TimerDuration)
            {
                Entity->JumpTimer.TimeElapsed = 0.0f;
                Entity->IsJumping = false;
            }
            else
            {
                Body->Velocity.Y = 250.0f;
            }
        }

        // NOTE(Sleepster): COYOTE TIMING
        if(IsKeyDown(KEY_SPACE) && Entity->JumpCounter < Entity->MaxJumps)
        {
            bool CanCoyoteJump   = (Entity->CoyoteTimer.TimeElapsed < Entity->CoyoteTimer.TimerDuration);
            bool CanJumpNormally = Entity->IsGrounded;

            if(CanCoyoteJump || CanJumpNormally)
            {
                Entity->IsGrounded = false;
                Entity->IsJumping  = true;
                Entity->JumpCounter++;
        
                Entity->CoyoteTimer.TimeElapsed = Entity->CoyoteTimer.TimerDuration; 
            }
        }

        // NOTE(Sleepster): DASHING
        if(Entity->IsDashing &&
           !Entity->IsGrounded &&
           fabsf(Body->Acceleration.X) > 0.0f &&
           Entity->DashCounter <= Entity->MaxDashes)
        {
            Body->Velocity.Y = 0.0f;
            Entity->DashTimer.TimeElapsed += DeltaTime;
            if(Entity->DashTimer.TimeElapsed >= Entity->DashTimer.TimerDuration)
            {
                Entity->DashTimer.TimeElapsed = 0.0f;
                Entity->IsDashing = false;
            }
            else
            {
                Body->Velocity.X += 50.0f * Sign(Body->Velocity.X);
            }
        }

        Body->Velocity    += (Body->Acceleration * DeltaTime);
        Entity->Position  += Body->Velocity * DeltaTime;

        Body->CollisionRect.Position = Entity->Position;
        Body->CollisionRect.Min = Body->CollisionRect.Position - Body->CollisionRect.HalfSize;
        Body->CollisionRect.Max = Body->CollisionRect.Position + Body->CollisionRect.HalfSize;
        Body->Acceleration = vec2{0};

        vec2 ScaledVelocity = v2Scale(Body->Velocity, DeltaTime * TickRate);

        collision_data IsColliding = {};
        for(uint32 IterationIndex = 0;
            IterationIndex < IterationCounter;
            ++IterationIndex)
        {
            SweepEntitiesForCollision(GameState, Entity, ScaledVelocity);
            IsColliding = EntityCollisionResponse(GameState, Entity);
        }

        if(IsColliding.Collision)
        {
            if(IsColliding.CollidedEntity)
            {
                if(IsColliding.CollidedEntity->Archetype == ARCH_TILE &&
                   fabsf(IsColliding.Depth.X) == 0.0f)
                {
                    Entity->DashCounter = 0;
                    Entity->JumpCounter = 0;
                    Entity->IsDashing = false;
                    Entity->IsGrounded = true;
                }
                
                if(IsColliding.CollidedEntity->OnCollide != 0)
                {
                    IsColliding.CollidedEntity->OnCollide(Entity, IsColliding.CollidedEntity);
                }
            }
        }
    }
}

internal void
AdjustStaticObjectPosition(entity *Entity, vec2 Position)
{
    physics_body *Body = &Entity->PhysicsBodyData;
    
    Entity->Position = Position;
    Body->CollisionRect.Position = Entity->Position;
    Body->CollisionRect.Min = Body->CollisionRect.Position - Body->CollisionRect.HalfSize;
    Body->CollisionRect.Max = Body->CollisionRect.Position + Body->CollisionRect.HalfSize;
}

internal inline texture2d
STPLoadTexture(string Filepath)
{
    texture2d Result = {};
    
    image2d Image = LoadImage(CSTR(Filepath));
    ImageFlipVertical(&Image);

    Result = LoadTextureFromImage(Image);
    return(Result);
}

int 
main()
{
    game_state  GameState = {};

    GameState.WindowSizeData = {1920, 1080};
    GameState.Gravity = -500;
    GameState.MaxG = -400;
    
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(GameState.WindowSizeData.X, GameState.WindowSizeData.Y, "Save The Prince");

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
        GameState.Entities            = PushArray(&GameState.GameArena, entity, MAX_ENTITIES);
        GameState.EntitySortingBuffer = PushArray(&GameState.GameArena, entity, MAX_ENTITIES);
    }
    
    entity *Player = CreateEntity(&GameState);
    SetupEntityPlayer(Player);
    Player->Position.Y = 42;

    GameState.Textures[GameState.ActiveTextureCount++] = LoadTexture("../data/res/textures/Dungeon/Assets/Assets.png");
    LoadOGMOLevel(&GameState, ivec2{-150, 210});

    real32 Accumulator = 0;
    while(!WindowShouldClose())
    {
        GameState.WindowSizeData.X = GetRenderWidth();
        GameState.WindowSizeData.Y = GetRenderHeight();
        
        //v2Approach(&DrawFrame->SceneCamera.Position, DrawFrame->SceneCamera.Target, 5.0f, Time.Delta);
        GameState.SceneCamera.offset = {real32(GameState.WindowSizeData.X * 0.5f), (real32)(GameState.WindowSizeData.Y * 0.5f)};
        // NOTE(Sleepster): Raylib by default is flipped, meaning a change in the Positive Y direction yields a downward movement.
        // This fixes that but it might introduce some bugs down the line so I'm just putting this here

        real32 ZoomX = real32(real32(GameState.WindowSizeData.X) / real32(GAME_WORLD_WIDTH));
        real32 ZoomY = real32(real32(GameState.WindowSizeData.Y) / real32(GAME_WORLD_HEIGHT));
        GameState.SceneCamera.zoom   = -1.0f * fmaxf(ZoomX, ZoomY);

        Vector2 RaylibMousePos = GetMousePosition();
        Vector2 RaylibWorldToScreenMousePos = GetScreenToWorld2D(RaylibMousePos, GameState.SceneCamera);
        vec2    MousePos = vec2{RaylibWorldToScreenMousePos.x, RaylibWorldToScreenMousePos.y};

        DrawFPS(0, 10);
        DeltaTime = GetFrameTime();
        Accumulator += DeltaTime;
        if(Accumulator >= 1.0f)
        {
            printm("FrameTime: %f\n", DeltaTime * 100);
            Accumulator = 0;
        }

        if(IsKeyPressed(KEY_Y))
        {
            Player = CreateEntity(&GameState);
            SetupEntityPlayer(Player);
            Player->Position.Y = 42;
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        BeginMode2D(GameState.SceneCamera);
        RadixSort((void *)GameState.Entities, (void *)GameState.EntitySortingBuffer, MAX_ENTITIES, sizeof(entity), offsetof(entity, LayerIndex), 21);
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

                    vec2 CameraPosition = vec2{GameState.SceneCamera.target.x, GameState.SceneCamera.target.y};
                    v2Approach(&CameraPosition, Temp->Position, 10.0f, DeltaTime);
                    GameState.SceneCamera.target = Vector2{CameraPosition.X, CameraPosition.Y};

                    UpdateEntityPhysicsBodyData(&GameState, Temp);
                    DrawEntity(Temp, RED);

                }break;
                case ARCH_TILE:
                {
                    rect TextureSourceRect =
                    {
                        real32(Temp->StaticSprite.AtlasOffset.X),
                        real32(Temp->StaticSprite.AtlasOffset.Y),
                        real32(Temp->StaticSprite.SpriteSize.X) * -1.0f,
                        real32(Temp->StaticSprite.SpriteSize.Y) * -1.0f
                    };

                    rect SpriteDestRect =
                    {
                        real32(Temp->Position.X - int32(Temp->StaticSprite.SpriteSize.X * 0.5f)),
                        real32(Temp->Position.Y - int32(Temp->StaticSprite.SpriteSize.Y * 0.5f)),
                        real32(Temp->StaticSprite.SpriteSize.X),
                        real32(Temp->StaticSprite.SpriteSize.Y)
                    };

                    DrawTexturePro(GameState.Textures[0], TextureSourceRect, SpriteDestRect, rlvec2{0}, 0.0f, WHITE);
                }break;
                case ARCH_STROBBY:
                {
                    DrawEntity(Temp, ORANGE);
                };
                default:
                {
                    DrawEntity(Temp, ORANGE);
                }break;
            }
        }

        EndMode2D();
        EndDrawing();
    }
}
