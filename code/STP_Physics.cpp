/* ========================================================================
   $File: STP_Physics.cpp $
   $Date: Tue, 24 Dec 24: 03:49AM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

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
TestMinkowskiBox(aabb A, aabb B)
{
    aabb Test = CalculateMinkowskiDifference(A, B);

    return(0 >= Test.Min.X && 0 <= Test.Max.X &&
           0 >= Test.Min.Y && 0 <= Test.Max.Y);
}

bool32 AABBOverlap(aabb A, aabb B)
{
    return (A.Min.X <= B.Max.X && A.Max.X >= B.Min.X &&
            A.Min.Y <= B.Max.Y && A.Max.Y >= B.Min.Y);
}

internal void
ActorMoveX(game_state *GameState, entity *Entity, physics_body *Body, real32 MoveX)
{
    if(MoveX != 0)
    {
        real32 Remainder = Entity->Position.X + MoveX;
    
        aabb TestRect = Entity->PhysicsBodyData.CollisionRect;
        TestRect.Position.X += MoveX;
        TestRect.Min.X      += MoveX;
        TestRect.Max.X      += MoveX;
    
        for(uint32 CollisionIndex = 0;
            CollisionIndex < MAX_ENTITIES;
            ++CollisionIndex)
        {
            entity *TestEntity = &GameState->Entities[CollisionIndex];
            if((TestEntity->Flags & IS_VALID) != 0 &&
               TestEntity->EntityID != Entity->EntityID &&
               TestEntity->PhysicsBodyData.CollisionRect.HalfSize != vec2{0})
            {
                if(AABBOverlap(TestRect, TestEntity->PhysicsBodyData.CollisionRect))
                {
                    if(MoveX > 0)
                    {
                        Remainder = TestEntity->PhysicsBodyData.CollisionRect.Min.X - Entity->PhysicsBodyData.CollisionRect.HalfSize.X * 2.0f;
                    }
                    else if(MoveX < 0)
                    {
                        Remainder = TestEntity->PhysicsBodyData.CollisionRect.Max.X;
                    }

                    if(TestEntity->OnCollide)
                    {
                        TestEntity->OnCollide(Entity, TestEntity);
                    }

                    Entity->PhysicsBodyData.Velocity.X = 0;
                    return;
                }
            }
        }
        Entity->Position.X = Remainder;
        Entity->PhysicsBodyData.CollisionRect.Position.X = Entity->Position.X + (Entity->RenderSize.X * 0.25f);
        Entity->PhysicsBodyData.CollisionRect.Min.X      = (Remainder - (Entity->RenderSize.X * 0.25f)) - Entity->PhysicsBodyData.CollisionRect.HalfSize.X;
        Entity->PhysicsBodyData.CollisionRect.Max.X      = (Remainder + (Entity->RenderSize.X * 0.25f)) + Entity->PhysicsBodyData.CollisionRect.HalfSize.X;
    }
}

internal void
ActorMoveY(game_state *GameState, entity *Entity, physics_body *Body, real32 MoveY)
{
    if(MoveY != 0)
    {
        real32 Remainder = Entity->Position.Y + MoveY;
    
        aabb TestRect = Entity->PhysicsBodyData.CollisionRect;
        TestRect.Position.Y += MoveY;
        TestRect.Min.Y      += MoveY;
        TestRect.Max.Y      += MoveY;
    
        for(uint32 CollisionIndex = 0;
            CollisionIndex < MAX_ENTITIES;
            ++CollisionIndex)
        {
            entity *TestEntity = &GameState->Entities[CollisionIndex];
            if((TestEntity->Flags & IS_VALID) != 0 &&
               TestEntity->EntityID != Entity->EntityID &&
               TestEntity->PhysicsBodyData.CollisionRect.HalfSize != vec2{0})
            {
                if(AABBOverlap(TestRect, TestEntity->PhysicsBodyData.CollisionRect))
                {
                    if(MoveY > 0)
                    {
                        Remainder = TestEntity->PhysicsBodyData.CollisionRect.Min.Y - Entity->PhysicsBodyData.CollisionRect.HalfSize.Y * 2.0f;
                    }
                    else if(MoveY < 0)
                    {
                        Entity->IsGrounded = true;
                        Remainder = TestEntity->PhysicsBodyData.CollisionRect.Max.Y;
                    }

                    if(TestEntity->OnCollide)
                    {
                        TestEntity->OnCollide(Entity, TestEntity);
                    }

                    Entity->PhysicsBodyData.Velocity.Y = 0;
                    return;
                }
            }
        }
        Entity->Position.Y = Remainder;
        Entity->PhysicsBodyData.CollisionRect.Position.Y = Entity->Position.Y + ((Entity->RenderSize.Y - 4) * 0.25f);
        Entity->PhysicsBodyData.CollisionRect.Min.Y      = (Remainder - (Entity->RenderSize.Y * 0.25f)) - Entity->PhysicsBodyData.CollisionRect.HalfSize.Y + 3;
        Entity->PhysicsBodyData.CollisionRect.Max.Y      = (Remainder + (Entity->RenderSize.Y * 0.25f)) + Entity->PhysicsBodyData.CollisionRect.HalfSize.Y;
    }
}

internal void
UpdateEntityPhysicsData(game_state *GameState)
{
    for(uint32 EntityIndex = 0;
        EntityIndex < MAX_ENTITIES;
        ++EntityIndex)
    {
        entity *Entity = &GameState->Entities[EntityIndex];
        physics_body *Body = &Entity->PhysicsBodyData;
        if(Entity && (Entity->Flags & IS_VALID) != 0)
        {
            if(Body && Body->BodyType == PB_Actor)
            {
                Entity->PreviousPosition = Entity->Position;
                Body->Velocity.X = Body->Acceleration.X * (real32)UpdateRate;
                if(Body->Velocity.X > 100.0f)
                {
                    Body->Velocity.X = 100.0f;
                }
                else if(Body->Velocity.X < -100.0f)
                {
                    Body->Velocity.X = -100.0f;
                }

                Body->Velocity.Y = Body->Acceleration.Y * (real32)UpdateRate;
                if(Body->Velocity.Y < -150)
                {
                    Body->Velocity.Y = -150;
                }
                
                vec2  ScaledVelocity  = Body->Velocity * (real32)UpdateRate;
                int32 Steps = int32(ceilf(MAX(fabsf(Body->Velocity.X), fabsf(Body->Velocity.Y)) / TILE_SIZE.X));
                vec2  SteppedVelocity = ScaledVelocity / real32(Steps);
                for(int32 StepIndex = 0;
                    StepIndex < Steps;
                    ++StepIndex)
                {
                    ActorMoveX(GameState, Entity, Body, SteppedVelocity.X);
                    ActorMoveY(GameState, Entity, Body, SteppedVelocity.Y);
                }
            }
        }
    }
}
