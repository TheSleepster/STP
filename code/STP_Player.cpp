/* ========================================================================
   $File: STP_Player.cpp $
   $Date: Tue, 24 Dec 24: 03:43AM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

constexpr animated_sprite_data PlayerStateSprites[] =
{
    {.AnimationOffset = { 0, 174}, .SpriteSize = {16, 18}, .FrameCount =  2, .FrameTime = {.TimerDuration = 1.5f}, .DelayOnLoop = {.TimerDuration = 2.5f}},
    {.AnimationOffset = {16, 142}, .SpriteSize = {16, 18}, .FrameCount = 12, .FrameTime = {.TimerDuration = 0.05f}, .DelayOnLoop = {.TimerDuration = 2.5f}},
    {.AnimationOffset = {16, 142}, .SpriteSize = {16, 18}, .FrameCount = 12, .FrameTime = {.TimerDuration = 0.05f}, .DelayOnLoop = {.TimerDuration = 2.5f}},
    {.AnimationOffset = {16, 142}, .SpriteSize = {16, 18}, .FrameCount = 12, .FrameTime = {.TimerDuration = 0.05f}, .DelayOnLoop = {.TimerDuration = 2.5f}},
    {.AnimationOffset = {16, 142}, .SpriteSize = {16, 18}, .FrameCount = 12, .FrameTime = {.TimerDuration = 0.05f}, .DelayOnLoop = {.TimerDuration = 2.5f}},
    {.AnimationOffset = {16, 142}, .SpriteSize = {16, 18}, .FrameCount = 12, .FrameTime = {.TimerDuration = 0.05f}, .DelayOnLoop = {.TimerDuration = 2.5f}},
    {.AnimationOffset = {16, 142}, .SpriteSize = {16, 18}, .FrameCount = 12, .FrameTime = {.TimerDuration = 0.05f}, .DelayOnLoop = {.TimerDuration = 2.5f}},
};

internal void
UpdatePlayerInput(game_state *GameState, entity *Player)
{
    GameState->InputAxis.X = real32(IsKeyDown(KEY_D) - IsKeyDown(KEY_A));
    GameState->InputAxis.Y = real32(IsKeyDown(KEY_W) - IsKeyDown(KEY_S));

    Player->IsJumping = IsKeyDown(KEY_SPACE);
    Player->IsDashing = IsKeyDown(KEY_LEFT_SHIFT);
}

internal void
EntityEnterIdleState(entity *Entity)
{
    real32 ElapsedTime = Entity->AnimatedSprite.FrameTime.TimeElapsed;
    int32  FrameIndex  = Entity->AnimatedSprite.CurrentFrameIndex;

    Entity->AnimatedSprite = PlayerStateSprites[ES_IDLE]; 
    Entity->AnimatedSprite.FrameTime.TimeElapsed = ElapsedTime;
    Entity->AnimatedSprite.CurrentFrameIndex = FrameIndex;
}

internal void
EntityEnterRunningState(entity *Entity)
{
    real32 ElapsedTime = Entity->AnimatedSprite.FrameTime.TimeElapsed;
    int32  FrameIndex  = Entity->AnimatedSprite.CurrentFrameIndex;

    Entity->AnimatedSprite = PlayerStateSprites[ES_RUNNING]; 
    Entity->AnimatedSprite.FrameTime.TimeElapsed = ElapsedTime;
    Entity->AnimatedSprite.CurrentFrameIndex = FrameIndex;
}

internal void
EntityUpdateIdleState(entity *Entity)
{
}

internal void
EntityUpdateRunningState(entity *Entity)
{
}

internal void
EntityChangeState(entity *Entity)
{
    Entity->AnimatedSprite.FrameTime.TimeElapsed = 0;
    Entity->AnimatedSprite.CurrentFrameIndex = 0;
    switch(Entity->EntityState)
    {
        case ES_IDLE:    EntityEnterIdleState(Entity); break;
        case ES_RUNNING: EntityEnterRunningState(Entity); break;
    }
}

internal void
UpdatePlayerData(game_state *GameState)
{
    for(uint32 EntityIndex = 0;
        EntityIndex < MAX_ENTITIES;
        ++EntityIndex)
    {
        entity *Player = &GameState->Entities[EntityIndex];
        Player->IsGrounded = true;
        if(Player->Archetype == ARCH_PLAYER)
        {
            Player->PreviousState = Player->EntityState;
            UpdatePlayerInput(GameState, Player);
            if(Player->IsGrounded)
            {
                if(GameState->InputAxis.X == 0)
                {
                    Player->EntityState = ES_IDLE;
                }
                else
                {
                    Player->EntityState = ES_RUNNING;
                }
            }
            else
            {
                if(Player->IsDashing)
                {
                    Player->EntityState = ES_DASHING;
                }
            }
            
            if(Player->PreviousState != Player->EntityState)
            {
                EntityChangeState(Player);
            }

            Player->AnimatedSprite.FrameTime.TimeElapsed += UpdateRate;
            if(Player->AnimatedSprite.FrameTime.TimeElapsed >= Player->AnimatedSprite.FrameTime.TimerDuration)
            {
                Player->AnimatedSprite.CurrentFrameIndex++;
                Player->AnimatedSprite.CurrentFrameIndex %= Player->AnimatedSprite.FrameCount;
                Player->AnimatedSprite.FrameTime.TimeElapsed = 0;
            }
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
PlayerEnterIdleState(game_state *GameState, entity *Entity)
{
    Entity->AnimatedSprite = PlayerStateSprites[ES_IDLE]; 
    Entity->AnimatedSprite.FrameTime.TimeElapsed = 0;
    Entity->AnimatedSprite.CurrentFrameIndex = 0;
}

internal void
PlayerUpdateIdleState(game_state *GameState, entity *Entity)
{
    UpdateAnimationData(Entity);
}

internal void
SetupEntityPlayer(entity *Entity)
{
    Entity->Archetype     = ARCH_PLAYER;
    Entity->MovementSpeed = 3000;
    Entity->RenderSize    = {12, 16};
    Entity->LayerIndex    = LAYER_Player;

    Entity->JumpTimer.TimerDuration   = 0.15f;
    Entity->DashTimer.TimerDuration   = 0.08f;
    Entity->CoyoteTimer.TimerDuration = 0.15f;

    Entity->MaxJumps = 1;
    Entity->JumpCounter = 0;

    Entity->MaxDashes = 1;
    Entity->DashCounter = 0;

    //Entity->Flags |= IS_GRAVITIC;

    Entity->PhysicsBodyData.BodyType = PB_Actor;
    Entity->PhysicsBodyData.CollisionRect.HalfSize = Entity->RenderSize * 0.5f;

    Entity->EntityState    = ES_IDLE;
    Entity->AnimatedSprite = PlayerStateSprites[ES_IDLE];

    Entity->EntityStateManager.Callbacks[ES_IDLE] =
    {
        .OnEnter  = PlayerEnterIdleState,
        .OnUpdate = PlayerUpdateIdleState,
        .OnExit   = nullptr,
    };
}

internal void
HandlePlayerState(game_state *GameState)
{
    for(uint32 EntityIndex = 0;
        EntityIndex < MAX_ENTITIES;
        ++EntityIndex)
    {
        entity *Player = &GameState->Entities[EntityIndex];
        entity_state_machine *EntitySM = &Player->EntityStateManager;

        if(Player->Archetype == ARCH_PLAYER)
        {
            EntitySM->PreviousState = EntitySM->CurrentState;
            UpdatePlayerInput(GameState, Player);

            if(EntitySM->PreviousState != EntitySM->CurrentState)
            {
                entity_state_callback_data *Callback = &EntitySM->Callbacks[EntitySM->PreviousState];
                if(Callback->OnExit)
                {
                    Callback->OnExit(GameState, Player);
                }

                if(Callback->OnEnter)
                {
                    Callback->OnEnter(GameState, Player);
                }
            }
        }
    }
}
