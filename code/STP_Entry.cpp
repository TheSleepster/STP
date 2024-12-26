/* ========================================================================
   $File: STP_Entry.cpp $
   $Date: December 09 2024 03:18 pm $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#include <raylib.h>

#define  RAYGUI_IMPLMENTATION
#include <raygui.h>
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

constexpr uint32 IterationCounter  = 2;
constexpr real32 TickRate          = 1 / IterationCounter;
constexpr real64 UpdateRate        = (1.0 / 60.0);

constexpr uint32 MAX_ENTITIES      = 10000;
constexpr real32 InAirFrictionY    = 2.0f;
constexpr real32 InAirFrictionX    = 24.0f;
constexpr real32 EPSILON           = 0.2f;

constexpr uint32 GAME_WORLD_WIDTH  = 320;
constexpr uint32 GAME_WORLD_HEIGHT = 180;

constexpr ivec2 TILE_SIZE = {8, 8};
constexpr real32 CLIMB_SPEED = 2000; 

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
    LAYER_Player = 10,
};

enum entity_flags
{
    IS_VALID              = 1 << 0,
    IS_GRAVITIC           = 1 << 1,
    IS_PICKUP             = 1 << 2,
    IS_ANIMATED_PLATFORM  = 1 << 3,
    IS_PLATFORM_IN_MOTION = 1 << 4,
    IS_ONE_WAY_COLLISION  = 1 << 5,
    IS_CLIMBABLE          = 1 << 6,
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

    aabb   CollisionRect;
};

struct static_sprite_data
{
    ivec2 AtlasOffset;
    ivec2 SpriteSize;
};

struct timer
{
    real32 StartTime;
    real32 EndTime;
    
    real32 TimeElapsed;
    real32 TimerDuration;
};

struct animated_sprite_data
{
    ivec2  AnimationOffset;
    ivec2  SpriteSize;

    int32  FrameCount;
    int32  CurrentFrameIndex;
    
    timer  FrameTime;
    timer  DelayOnLoop;
};

enum entity_state
{
    ES_IDLE,
    ES_RUNNING,
    ES_CLIMBING,
    ES_DASHING,
    ES_JUMPING,
    ES_FALLING,
    ES_DEAD,
    ES_COUNT
};

struct game_state;
#define SM_STATE_CALLBACK(name) void name(game_state *GameState, entity *Entity)
typedef SM_STATE_CALLBACK(sm_callback);

struct entity_state_callback_data
{
    sm_callback *OnStateEnter;
    sm_callback *OnStateUpdate;
    sm_callback *OnStateExit;
};

struct entity_state_machine
{
    entity_state PreviousState;
    entity_state CurrentState;
    
    entity_state_callback_data Callbacks[ES_COUNT];
};

struct entity
{
    entity_arch  Archetype;
    
    uint32       Flags;
    uint32       EntityID;
    int32        LayerIndex;

    vec2         Position;
    vec2         PreviousPosition;
    vec2         RenderPosition;
    vec2         TargetPositionA;
    vec2         TargetPositionB;

    timer        MovingPlatformTravelTimer;
    timer        MovingPlatformStationaryTimer;
    
    vec2         RenderSize;
    real32       MovementSpeed;
    real32       ExperiencedGravity;

    real32       FacingDirection;

    bool8        IsGrounded;
    bool8        IsJumping;
    bool8        IsDashing;
    bool8        CanJump;
    bool8        CanDash;

    int32        MaxJumps;
    int32        JumpCounter;

    int32        MaxDashes;
    int32        DashCounter;
    int32        AttachedWallSide;
    
    timer        JumpTimer;
    timer        DashTimer;
    timer        CoyoteTimer;
    timer        ClingTimer;
    
    physics_body          PhysicsBodyData;
    entity_state_machine  EntityStateManager;   

    entity_state EntityState;
    entity_state PreviousState;
    entity_on_collide    *OnCollide;

    static_sprite_data    StaticSprite;
    animated_sprite_data  AnimatedSprite;
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

    vec2         InputAxis;
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
SetupEntityFloorTile(entity *Entity)
{
    Entity->Archetype  = ARCH_TILE;
    Entity->RenderSize = {400, 10};
    Entity->LayerIndex    = LAYER_Player;

    Entity->PhysicsBodyData.BodyType = PB_Solid;
    Entity->PhysicsBodyData.CollisionRect.HalfSize = Entity->RenderSize * 0.5f;
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

ENTITY_ON_COLLIDE_RESPONSE(MovePlayerWithPlatform)
{
}

internal void
SetupEntityMovingPlatform(entity *Entity, vec2 PositionA, vec2 PositionB, real32 TravelTimer, real32 StationaryTimer, vec2 Size, int32 LevelIndex)
{
    Entity->Archetype  = ARCH_TILE;
    Entity->RenderSize = Size;
    Entity->Flags     |= IS_ANIMATED_PLATFORM;

    Entity->Position   = PositionA;
    Entity->TargetPositionA = PositionA;
    Entity->TargetPositionB = PositionB;

    Entity->MovingPlatformTravelTimer.TimerDuration     = TravelTimer;
    Entity->MovingPlatformStationaryTimer.TimerDuration = StationaryTimer;

    Entity->PhysicsBodyData.BodyType = PB_Solid;
    Entity->PhysicsBodyData.CollisionRect.Position = PositionA;
    Entity->PhysicsBodyData.CollisionRect.HalfSize = Entity->RenderSize * 0.5f;

    Entity->OnCollide = &MovePlayerWithPlatform;
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
#include "STP_Physics.cpp"
#include "STP_Player.cpp"

#if 0
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
                if(Entity->IsDashing)
                {
                    Body->Velocity = vec2{0};
                }
                Body->Friction = TestBody->Friction;
                vec2 DepthVector = CalculateCollisionDepth(MinkowskiBody);
                Result.Depth = DepthVector;

                // NOTE(Sleepster): ONEWAY COLLISION
                if((TestEntity->Flags & IS_ONE_WAY_COLLISION) != 0)
                {
                    if(Body->Velocity.Y > 0)
                    {
                        continue;
                    }
                }

                // NOTE(Sleepster): WALL CLIMBING
                if((TestEntity->Flags & IS_CLIMBABLE) != 0)
                {
                    if(IsKeyDown(KEY_D) || IsKeyDown(KEY_A))
                    {
                        Entity->IsClinging = true;
                        Entity->AttachedWallSide = (DepthVector.X > 0) ? -1 : 1;
                    }
                }
                
                // NOTE(Sleepster): NO PICKUP COLLISION
                if((TestEntity->Flags & IS_PICKUP) == 0)
                {
                    if(fabsf(DepthVector.Y) >= EPSILON * 2.0f)
                    {
                        Entity->IsGrounded = true;
                    }

                    if(DepthVector.X != 0)
                    {
                        Entity->Position.X += DepthVector.X;
                        Body->Velocity.X = 0;
                    }
                    if(DepthVector.Y != 0)
                    {
                        Entity->Position.Y += DepthVector.Y;
                        Body->Velocity.Y = 0;
                    }

                    break;
                }
            }
            else if((MinkowskiBody.Max.Y - MinkowskiBody.Min.Y) > (EPSILON * 4))
            {
                Entity->IsGrounded = false;
                Entity->IsClinging = false;
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
                vec2 DashDirection = {};
                if(IsKeyDown(KEY_W)) DashDirection.Y =  1.0f;
                if(IsKeyDown(KEY_A)) DashDirection.X =  1.0f;
                if(IsKeyDown(KEY_S)) DashDirection.Y = -1.0f;
                if(IsKeyDown(KEY_D)) DashDirection.X = -1.0f;

                DashDirection = v2Normalize(DashDirection);
                real32 DashSpeed = 600;

                Body->Velocity.Y = 0;
                Body->Velocity = DashDirection * DashSpeed;
            }
        }

        if(Entity->IsClinging)
        {
            if(IsKeyDown(KEY_W))
            {
                Body->Velocity.Y = CLIMB_SPEED * DeltaTime;
            }
            else if(IsKeyDown(KEY_S))
            {
                Body->Velocity.Y = -CLIMB_SPEED * DeltaTime;
            }
            else
            {
                Body->Velocity.Y = 0;
            }
            Body->Velocity.X = 0;
        }

        Body->Velocity    += (Body->Acceleration * DeltaTime);
        Entity->Position  +=  Body->Velocity * DeltaTime;

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

internal void
UpdateMovingPlatforms(game_state *GameState)
{
    for(uint32 EntityIndex = 0;
        EntityIndex < MAX_ENTITIES;
        ++EntityIndex)
    {
        entity *Entity = &GameState->Entities[EntityIndex];
        if(((Entity->Flags & IS_VALID) != 0) && ((Entity->Flags & IS_ANIMATED_PLATFORM) != 0))
        {
            if(Entity->ShouldBeInMotion)
            {
                physics_body *Body = &Entity->PhysicsBodyData;
                Entity->PreviousPosition = Entity->Position;

                vec2 CurrentTarget = Entity->IsMovingTowardsTarget ? Entity->TargetPositionB : Entity->TargetPositionA;
                vec2 Difference    = Entity->IsMovingTowardsTarget ? Entity->TargetPositionA : Entity->TargetPositionB;
                Body->Velocity  = (CurrentTarget - Difference) / Entity->MovingPlatformTravelTimer.TimerDuration;

                Entity->Position += Body->Velocity * DeltaTime;
                Body->CollisionRect.Position = Entity->Position;

                if(v2Distance(CurrentTarget, Entity->Position) <= EPSILON)
                {
                    Entity->IsMovingTowardsTarget = !Entity->IsMovingTowardsTarget;
                    Entity->ShouldBeInMotion = false;
                    Entity->PhysicsBodyData.Velocity = vec2{0};
                    Entity->PreviousPosition = Entity->Position;
                }
            }
            else
            {
                if(Entity->MovingPlatformStationaryTimer.TimeElapsed <= Entity->MovingPlatformStationaryTimer.TimerDuration)
                {
                    Entity->MovingPlatformStationaryTimer.TimeElapsed += DeltaTime;
                }
                else
                {
                    Entity->ShouldBeInMotion = true;
                    Entity->MovingPlatformStationaryTimer.TimeElapsed = 0.0f;
                }
            }
        }
    }
}
#endif 
internal inline texture2d
STPLoadTexture(string Filepath)
{
    texture2d Result = {};
    
    image2d Image = LoadImage(CSTR(Filepath));
    ImageFlipVertical(&Image);

    Result = LoadTextureFromImage(Image);
    return(Result);
}


internal void
DrawEntityAnimatedSprite(game_state *GameState, entity *Entity, vec2 RenderPosition)
{
    ivec2 AtlasOffset = {Entity->AnimatedSprite.AnimationOffset.X + (Entity->AnimatedSprite.SpriteSize.X * Entity->AnimatedSprite.CurrentFrameIndex),
                         Entity->AnimatedSprite.AnimationOffset.Y};
    ivec2 SpriteSize  =  Entity->AnimatedSprite.SpriteSize;

    rect TextureSourceRect =
    {
        real32(AtlasOffset.X),
        real32(AtlasOffset.Y),
        real32(SpriteSize.X) * real32((Entity->FacingDirection > 0 ) ? -1.0f : (Entity->FacingDirection < 0) ? 1.0f : -1.0f),
        real32(SpriteSize.Y) * -1.0f
    };

    vec2 RenderPos = {}; 
    if(RenderPosition != vec2{0})
    {
        RenderPos = RenderPosition;
    }
    else
    {
        RenderPos = Entity->Position;
    }
    rect SpriteDestRect =
    {
        real32(RenderPos.X - int32(Entity->StaticSprite.SpriteSize.X * 0.5f)),
        real32(RenderPos.Y - int32(Entity->StaticSprite.SpriteSize.Y * 0.5f)),
        real32(Entity->AnimatedSprite.SpriteSize.X),
        real32(Entity->AnimatedSprite.SpriteSize.Y)
    };

    DrawTexturePro(GameState->Textures[0], TextureSourceRect, SpriteDestRect, rlvec2{0}, 0.0f, WHITE);
}

int 
main()
{
    game_state  GameState = {};

    GameState.WindowSizeData = {1920, 1080};
    GameState.Gravity = 0;
    GameState.Gravity = -500;
    GameState.MaxG    = -400;
    
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
    Player->Position.X = 20;
    Player->PreviousPosition = Player->Position;

    GameState.Textures[GameState.ActiveTextureCount++] = LoadTexture("../data/res/textures/NewAtlas.png");
    SetTextureFilter(GameState.Textures[0], TEXTURE_FILTER_POINT);
    LoadJSONLevelData(&GameState, STR("../data/res/maps/ldtktest/test.ldtk"));
    //LoadOGMOLevel(&GameState, STR("../data/res/maps/RealTest.json"), 0);

    real32 Accumulator = 0;
    while(!WindowShouldClose())
    {
        GameState.WindowSizeData.X = GetRenderWidth();
        GameState.WindowSizeData.Y = GetRenderHeight();
        
        GameState.SceneCamera.offset = {real32(GameState.WindowSizeData.X * 0.5f), (real32)(GameState.WindowSizeData.Y * 0.5f)};
        // NOTE(Sleepster): Raylib by default is flipped, meaning a change in the Positive Y direction yields a downward movement.
        // This fixes that but it might introduce some bugs down the line so I'm just putting this here

        real32 ZoomX = real32(real32(GameState.WindowSizeData.X) / real32(GAME_WORLD_WIDTH));
        real32 ZoomY = real32(real32(GameState.WindowSizeData.Y) / real32(GAME_WORLD_HEIGHT));
        GameState.SceneCamera.zoom   = -1.0f * fmaxf(ZoomX, ZoomY);

        DeltaTime = GetFrameTime();
        Accumulator += DeltaTime;
        while(Accumulator >= UpdateRate)
        {
            UpdateEntityPhysicsData(&GameState);
            HandlePlayerState(&GameState);
            //UpdateMovingPlatforms(&GameState);
            printf("DeltaTime: %fms\n", DeltaTime * 1000);

            Accumulator -= UpdateRate;
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
            if((Temp->Flags & IS_VALID) != 0)
            switch(Temp->Archetype)
            {
                case ARCH_PLAYER:
                {
                    #if 0
                    ProcessMovement(Temp);

                    vec2 CameraPosition = vec2{GameState.SceneCamera.target.x, GameState.SceneCamera.target.y};
                    v2Approach(&CameraPosition, Temp->Position, 5.0f, DeltaTime);
                    GameState.SceneCamera.target = Vector2{CameraPosition.X, CameraPosition.Y};

                    UpdateEntityPhysicsBodyData(&GameState, Temp);
                    DrawEntity(Temp, RED);
                    #endif

                    Temp->RenderPosition = v2Lerp(Temp->PreviousPosition, real32(Accumulator / (UpdateRate)), Temp->Position);
                    Temp->RenderPosition = {roundf(Temp->RenderPosition.X), roundf(Temp->RenderPosition.Y)};

                    vec2 CameraPosition = vec2{0, 42};
                    GameState.SceneCamera.target = Vector2{CameraPosition.X, CameraPosition.Y};

                    DrawEntityAnimatedSprite(&GameState, Temp, Temp->RenderPosition);
                }break;
                case ARCH_TILE:
                {
                    rect TextureSourceRect =
                    {
                        real32(Temp->StaticSprite.AtlasOffset.X),
                        real32(Temp->StaticSprite.AtlasOffset.Y),
                        real32(Temp->StaticSprite.SpriteSize.X),
                        real32(Temp->StaticSprite.SpriteSize.Y)
                    };

                    Temp->RenderPosition = v2Lerp(Temp->PreviousPosition, real32(Accumulator / (UpdateRate)), Temp->Position);
                    Temp->RenderPosition = {roundf(Temp->RenderPosition.X), roundf(Temp->RenderPosition.Y)};
                    rect SpriteDestRect =
                    {
                        real32(Temp->Position.X - int32(Temp->StaticSprite.SpriteSize.X * 0.5f)),
                        real32(Temp->Position.Y - int32(Temp->StaticSprite.SpriteSize.Y * 0.5f)),
                        real32(Temp->StaticSprite.SpriteSize.X),
                        real32(Temp->StaticSprite.SpriteSize.Y)
                    };

                    if((Temp->Flags & IS_ANIMATED_PLATFORM) == 0)
                    {
                        DrawTexturePro(GameState.Textures[0], TextureSourceRect, SpriteDestRect, rlvec2{0}, 0.0f, WHITE);
                    }
                    else
                    {
                        DrawEntity(Temp, BLUE);
                    }
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
        GameState.InputAxis.X = 0.0f;

        EndMode2D();
        EndDrawing();
    }
}
