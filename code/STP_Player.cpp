/* ========================================================================
   $File: STP_Player.cpp $
   $Date: Tue, 24 Dec 24: 03:43AM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

constexpr animated_sprite_data PlayerStateSprites[] =
{
    // IDLE
    {.AnimationOffset = {  0, 174}, .SpriteSize = {16, 18}, .FrameCount =  2, .FrameTime = {.TimerDuration = 1.0f}, .DelayOnLoop = {.TimerDuration = 2.5f}},
    // RUNNING
    {.AnimationOffset = { 16, 142}, .SpriteSize = {16, 18}, .FrameCount = 12, .FrameTime = {.TimerDuration = 0.05f}, .DelayOnLoop = {.TimerDuration = 2.5f}},
    // CLIMBING
    {.AnimationOffset = { 16, 142}, .SpriteSize = {16, 18}, .FrameCount = 12, .FrameTime = {.TimerDuration = 0.05f}, .DelayOnLoop = {.TimerDuration = 2.5f}},
    // DASHING
    {.AnimationOffset = { 16, 142}, .SpriteSize = {16, 18}, .FrameCount = 12, .FrameTime = {.TimerDuration = 0.05f}, .DelayOnLoop = {.TimerDuration = 2.5f}},
    // JUMPING
    {.AnimationOffset = {112, 142}, .SpriteSize = {16, 18}, .FrameCount = 1,  .FrameTime = {.TimerDuration = 0.05f}, .DelayOnLoop = {.TimerDuration = 2.5f}},
    // FALLING
    {.AnimationOffset = {112, 142}, .SpriteSize = {16, 18}, .FrameCount = 1,  .FrameTime = {.TimerDuration = 0.05f}, .DelayOnLoop = {.TimerDuration = 2.5f}},
    // DEATH
    {.AnimationOffset = { 16, 142}, .SpriteSize = {16, 18}, .FrameCount = 12, .FrameTime = {.TimerDuration = 0.05f}, .DelayOnLoop = {.TimerDuration = 2.5f}},
};

internal void
EntitySMChangeState(game_state *GameState, entity *Entity, entity_state ChangedState)
{
    entity_state_machine *EntitySM = &Entity->EntityStateManager;
    if(EntitySM->CurrentState != ChangedState)
    {
        entity_state_callback_data *PreviousStateCallback = &EntitySM->Callbacks[EntitySM->CurrentState];
        entity_state_callback_data *NewStateCallback      = &EntitySM->Callbacks[ChangedState];
        if(PreviousStateCallback && PreviousStateCallback->OnStateExit)
        {
            PreviousStateCallback->OnStateExit(GameState, Entity);
        }

        EntitySM->PreviousState = EntitySM->CurrentState;
        EntitySM->CurrentState  = ChangedState;
        if(NewStateCallback && NewStateCallback->OnStateEnter)
        {
            NewStateCallback->OnStateEnter(GameState, Entity);
        }
    }
}

internal void
UpdateAnimationData(entity *Player)
{
    Player->AnimatedSprite.FrameTime.TimeElapsed += UpdateRate;
    if(Player->AnimatedSprite.FrameTime.TimeElapsed >= Player->AnimatedSprite.FrameTime.TimerDuration)
    {
        Player->AnimatedSprite.CurrentFrameIndex++;
        Player->AnimatedSprite.CurrentFrameIndex %= Player->AnimatedSprite.FrameCount;
        Player->AnimatedSprite.FrameTime.TimeElapsed = 0;
    }
}

internal void
ChangePlayerStateAnimation(entity *Entity, entity_state State)
{
    Entity->AnimatedSprite = PlayerStateSprites[State]; 
    Entity->AnimatedSprite.FrameTime.TimeElapsed = 0;
    Entity->AnimatedSprite.CurrentFrameIndex = 0;
}

internal void
PlayerEnterIdleState(game_state *GameState, entity *Entity)
{
    ChangePlayerStateAnimation(Entity, ES_IDLE);
}

internal void
PlayerEnterRunningState(game_state *GameState, entity *Entity)
{
    Entity->ExperiencedGravity = GameState->Gravity;
    ChangePlayerStateAnimation(Entity, ES_RUNNING);
}

internal void
PlayerEnterJumpingState(game_state *GameState, entity *Entity)
{
    Entity->JumpTimer.TimeElapsed = 0;
    if(Entity->JumpCounter < 0)
    {
        Entity->CanJump = false;
    }
    else
    {
        Entity->JumpCounter--;
        Entity->IsGrounded = false;
        Entity->PhysicsBodyData.Acceleration.Y = 20000.0f;
        ChangePlayerStateAnimation(Entity, ES_JUMPING);
    }
}

internal void
PlayerEnterFallingState(game_state *GameState, entity *Entity)
{
    Entity->ExperiencedGravity = GameState->Gravity * 2.15f;
    ChangePlayerStateAnimation(Entity, ES_FALLING);
}

internal void
PlayerEnterDashingState(game_state *GameState, entity *Entity)
{
    Entity->DashTimer.TimeElapsed = 0;
    if(Entity->JumpCounter >= 0)
    {
        Entity->CanDash = false;
    }
    else
    {
        Entity->DashCounter--;
        ChangePlayerStateAnimation(Entity, ES_DASHING);
    }
}

internal void
PlayerUpdateIdleState(game_state *GameState, entity *Entity)
{
    UpdateAnimationData(Entity);
}

internal void
PlayerUpdateRunningState(game_state *GameState, entity *Entity)
{
    UpdateAnimationData(Entity);
    if(GameState->InputAxis.X != 0)
    {
        int32 RunDirection = Sign(GameState->InputAxis.X);
        Entity->PhysicsBodyData.Acceleration.X = 100000.0f * -RunDirection; 
    }
}

internal void
PlayerUpdateJumpingState(game_state *GameState, entity *Entity)
{
    Entity->JumpTimer.TimeElapsed += UpdateRate;
    if(Entity->JumpTimer.TimeElapsed >= Entity->JumpTimer.TimerDuration)
    {
        Entity->IsJumping = false;
        Entity->JumpTimer.TimeElapsed = 0;
        EntitySMChangeState(GameState, Entity, ES_FALLING);
    }
    else
    {
        Entity->PhysicsBodyData.Acceleration.Y = 30000.0f;
        UpdateAnimationData(Entity);
    }
}

internal void
PlayerUpdateDashingState(game_state *GameState, entity *Entity)
{
    Entity->DashTimer.TimeElapsed += UpdateRate;
    if(Entity->DashTimer.TimeElapsed >= Entity->DashTimer.TimerDuration)
    {
        Entity->IsDashing = false;
        EntitySMChangeState(GameState, Entity, ES_FALLING);
    }
    else
    {
        int32 RunDirectionX = Sign(GameState->InputAxis.X);
        int32 RunDirectionY = Sign(GameState->InputAxis.Y);

        Entity->PhysicsBodyData.Acceleration.X = 400.0f * -RunDirectionX;
        Entity->PhysicsBodyData.Acceleration.Y = 400.0f * -RunDirectionY;
        Entity->PhysicsBodyData.Acceleration   = v2Normalize(Entity->PhysicsBodyData.Acceleration);
    }
}

internal void
PlayerUpdateFallingState(game_state *GameState, entity *Entity)
{
    if(GameState->InputAxis.X != 0)
    {
        int32 RunDirection = Sign(GameState->InputAxis.X);
        Entity->PhysicsBodyData.Acceleration.X = 40000.0f * -RunDirection; 
    }

    real32 GravityMult = 1.0f;
    if(Entity->PhysicsBodyData.Velocity.Y > 0)
    {
        GravityMult = Lerp(1.0f, fabsf(Entity->PhysicsBodyData.Velocity.Y) / 500.0f, 0.5f);
    }
    else
    {
        GravityMult = Lerp(1.0f, fabsf(Entity->PhysicsBodyData.Velocity.Y) / 400.0f, 1.5f);
    }
    Entity->PhysicsBodyData.Acceleration.Y += real32(Entity->ExperiencedGravity) * GravityMult;
}

internal void
SetupEntityPlayer(entity *Entity)
{
    Entity->Archetype     = ARCH_PLAYER;
    Entity->MovementSpeed = 3000;
    Entity->RenderSize    = {16, 18};
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

    Entity->EntityState    = ES_IDLE;
    Entity->AnimatedSprite = PlayerStateSprites[ES_IDLE];

    Entity->EntityStateManager.Callbacks[ES_IDLE] =
    {
        .OnStateEnter  = PlayerEnterIdleState,
        .OnStateUpdate = PlayerUpdateIdleState,
        .OnStateExit   = nullptr
    };
    Entity->EntityStateManager.Callbacks[ES_RUNNING] =
    {
        .OnStateEnter  = PlayerEnterRunningState,
        .OnStateUpdate = PlayerUpdateRunningState,
        .OnStateExit   = nullptr
    };
    Entity->EntityStateManager.Callbacks[ES_JUMPING] =
    {
        .OnStateEnter  = PlayerEnterJumpingState,
        .OnStateUpdate = PlayerUpdateJumpingState,
        .OnStateExit   = nullptr
    };
    Entity->EntityStateManager.Callbacks[ES_FALLING] =
    {
        .OnStateEnter  = PlayerEnterFallingState,
        .OnStateUpdate = PlayerUpdateFallingState,
        .OnStateExit   = nullptr
    };
}

internal void
UpdatePlayerInput(game_state *GameState, entity *Player)
{
    if(IsKeyDown(KEY_A) || IsKeyDown(KEY_D))
    {
        GameState->InputAxis.X  = real32(IsKeyDown(KEY_D) ? 1.0f : -1.0f);
        Player->FacingDirection = real32(IsKeyDown(KEY_D) ? 1.0f : -1.0f);
    }
    if(Player->IsGrounded)
    {
        Player->CanJump = true;
        Player->CanDash = false;
        Player->JumpCounter = Player->MaxJumps;

        if(GameState->InputAxis.X != 0 && Player->EntityStateManager.CurrentState != ES_RUNNING)
        {
            EntitySMChangeState(GameState, Player, ES_RUNNING);
        }
        else if(GameState->InputAxis.X == 0.0f && Player->EntityStateManager.CurrentState != ES_IDLE)
        {
            EntitySMChangeState(GameState, Player, ES_IDLE);
        }

        if(IsKeyDown(KEY_SPACE) && Player->CanJump)
        {
            Player->IsJumping = true;
            EntitySMChangeState(GameState, Player, ES_JUMPING);
        }
    }
    else
    {
        Player->CanDash = true;
        if(Player->EntityStateManager.CurrentState != ES_JUMPING &&
           Player->EntityStateManager.CurrentState != ES_DASHING)
        {
            EntitySMChangeState(GameState, Player, ES_FALLING);
        }

        if(IsKeyPressed(KEY_LEFT_SHIFT) && Player->CanDash)
        {
            Player->IsDashing = true;
            EntitySMChangeState(GameState, Player, ES_DASHING);
        }
    }

    if(IsKeyDown(KEY_W) || IsKeyDown(KEY_S))
    {
        GameState->InputAxis.Y = real32(IsKeyDown(KEY_W) ? 1.0f : -1.0f);
    }
}

internal void
HandlePlayerState(game_state *GameState)
{
    for(uint32 EntityIndex = 0;
        EntityIndex < MAX_ENTITIES;
        ++EntityIndex)
    {
        entity *Player = &GameState->Entities[EntityIndex];
        entity_state_callback_data *Callbacks = &Player->EntityStateManager.Callbacks[Player->EntityStateManager.CurrentState];
        if(Player->Archetype == ARCH_PLAYER)
        {
            UpdatePlayerInput(GameState, Player);
            if(Callbacks && Callbacks->OnStateUpdate)
            {
                Callbacks->OnStateUpdate(GameState, Player);
            }

            if(GameState->InputAxis.X == 0 && fabsf(Player->PhysicsBodyData.Velocity.X) > 0.0f)
            {
                Player->PhysicsBodyData.Acceleration.X *= 0.15;
            }
            return;
        }
    }
}
