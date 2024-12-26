/* ========================================================================
   $File: STP_Map.cpp $
   $Date: Sat, 14 Dec 24: 05:06PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

struct stp_level_layer_data
{
    string Name;

    int32  OffsetX;
    int32  OffsetY;

    int32  CellWidth;
    int32  CellHeight;

    int32  TilemapLayerWidth;
    int32  TilemapLayerHeight;

    size_t  TilemapTileCount;
    int32  *TilemapTileIndices;

    string TilesetFilepath;
};

struct stp_level_data
{
    int32 LevelWidth;
    int32 LevelHeight;

    int32 LevelOffsetX;
    int32 LevelOffsetY;
    
    size_t LevelTilemapCount;
    string *Tilesets;

    size_t  CollisionLayerIndexCount;
    int32  *CollisionLayerData;

    size_t  LevelEntityCount;
    entity *LevelEntities;

    size_t LevelLayerCount;
    stp_level_layer_data *Layers;
};

// NOTE(Sleepster): Entity A is the entity that we would like to act upon, Entity B Is the entity doing the acting
internal
ENTITY_ON_COLLIDE_RESPONSE(SpikeCollision)
{
    DeleteEntity(A);
}

internal
ENTITY_ON_COLLIDE_RESPONSE(StrobbyCollision)
{
    DeleteEntity(B);
    A->DashCounter = 0;
}

#if 0
internal stp_level_data*
ProcessLoadedMapData(game_state *GameState, stp_level_data *LevelData)
{
    size_t LayerCounter = LevelData->LevelLayerCount;
    for(uint32 LayerIndex = 0;
        LayerIndex < LevelData->LevelLayerCount;
        ++LayerIndex)
    {
        stp_level_layer_data *CurrentWorkingLayer = &LevelData->Layers[LayerIndex];
        if(CurrentWorkingLayer->TilemapTileIndices)
        {
            // TODO(Sleepster): This is really stupid but we have to do this for now! 
            int32 TilemapWidthInTiles  = GameState->Textures[0].width / TILE_SIZE.X;
            int32 TilemapHeightInTiles = GameState->Textures[0].height / TILE_SIZE.Y;

            for(int32 TileIndex = 0;
                TileIndex < CurrentWorkingLayer->TilemapTileCount;
                ++TileIndex)
            {
                int32 TileID = CurrentWorkingLayer->TilemapTileIndices[TileIndex];
                if(TileID != -1)
                {
                    ivec2 TileUVData =
                        {
                            (TileID % TilemapWidthInTiles) * TILE_SIZE.X,
                            (TileID / TilemapWidthInTiles) * TILE_SIZE.Y
                        };

                    ivec2 TileWorldPosition =
                        {
                            (TilemapWidthInTiles   - 1 - (TileIndex % CurrentWorkingLayer->TilemapLayerWidth) * TILE_SIZE.X) - LevelData->LevelOffsetX,
                            (TilemapHeightInTiles  - 1 - (TileIndex / CurrentWorkingLayer->TilemapLayerWidth) * TILE_SIZE.Y) + LevelData->LevelOffsetY,
                        };

                    entity *Entity = CreateEntity(GameState);
                    Entity->Archetype = ARCH_TILE;
                    Entity->Position = v2Cast(TileWorldPosition);
                    Entity->StaticSprite = {.AtlasOffset = TileUVData, .SpriteSize = TILE_SIZE};
                    Entity->LayerIndex = int32(LayerCounter--);

                    if(LevelData->CollisionLayerData)
                    {
                        if(LevelData->CollisionLayerData[TileIndex] != 0)
                        {
                            Entity->PhysicsBodyData.BodyType = PB_Solid;
                            Entity->PhysicsBodyData.CollisionRect.HalfSize = v2Cast(TILE_SIZE * 0.5f);
                            Entity->PhysicsBodyData.CollisionRect.Position = Entity->Position;

                            Entity->PhysicsBodyData.CollisionRect.Min      = Entity->Position - Entity->PhysicsBodyData.CollisionRect.HalfSize; 
                            Entity->PhysicsBodyData.CollisionRect.Max      = Entity->Position + Entity->PhysicsBodyData.CollisionRect.HalfSize; 

                            switch(LevelData->CollisionLayerData[TileIndex])
                            {
                                case 2:
                                {
                                    Entity->OnCollide = &SpikeCollision;
                                }break;
                                case 3:
                                {
                                    Entity->Flags |= IS_ONE_WAY_COLLISION;
                                }break;
                                case 4:
                                {
                                    Entity->Flags |= IS_CLIMBABLE;
                                }break;
                            }
                        }
                    }
                }
            }
        }
    }
    return(LevelData);
}

internal stp_level_data*
LoadOGMOLevel(game_state *GameState, string Filepath, int32 LevelIndex)
{
    stp_level_data *Result = PushStruct(&GameState->GameArena, stp_level_data);
    
    string EntireFile = ReadEntireFileMA(&GameState->GameArena, Filepath);
    if(EntireFile != NULLSTR)
    {
        JSON_doc *JSONData = JSON_read(CSTR(EntireFile), EntireFile.Length, 0);
        if(JSONData)
        {
            JSON_val *JSONRoot = JSON_doc_get_root(JSONData);
            if(JSONRoot)
            {
                Result->LevelWidth   = JSON_get_int(JSON_obj_get(JSONRoot, "width"));
                Result->LevelHeight  = JSON_get_int(JSON_obj_get(JSONRoot, "height"));

                Result->LevelOffsetX = JSON_get_int(JSON_obj_get(JSONRoot, "OffsetX"));
                Result->LevelOffsetY = JSON_get_int(JSON_obj_get(JSONRoot, "OffsetY"));

                JSON_val *LevelTilemapLayers = JSON_obj_get(JSONRoot, "layers");
                Result->LevelLayerCount = JSON_arr_size(LevelTilemapLayers);
                Result->Layers = PushArray(&GameState->GameArena, stp_level_layer_data, Result->LevelLayerCount);

                size_t    LayerIndex    = 0;
                size_t    MaxLayerCount = 0;
                JSON_val *Value = 0;
                JSON_arr_foreach(LevelTilemapLayers, LayerIndex, MaxLayerCount, Value)
                {
                    stp_level_layer_data *WorkingLayer = &Result->Layers[LayerIndex];

                    WorkingLayer->Name = STR(JSON_get_str(JSON_obj_get(Value, "name")));
                    WorkingLayer->OffsetX = JSON_get_int(JSON_obj_get(Value, "offsetX"));
                    WorkingLayer->OffsetY = JSON_get_int(JSON_obj_get(Value, "offsetY"));

                    WorkingLayer->CellWidth  = JSON_get_int(JSON_obj_get(Value, "gridCellWidth"));
                    WorkingLayer->CellHeight = JSON_get_int(JSON_obj_get(Value, "gridCellHeight"));

                    WorkingLayer->TilemapLayerWidth  = JSON_get_int(JSON_obj_get(Value, "gridCellsX"));
                    WorkingLayer->TilemapLayerHeight = JSON_get_int(JSON_obj_get(Value, "gridCellsY"));

                    WorkingLayer->TilesetFilepath = STR(JSON_get_str(JSON_obj_get(Value, "tileset")));

                    JSON_val *TilemapArray = JSON_obj_get(Value, "data");
                    if(TilemapArray)
                    {
                        WorkingLayer->TilemapTileCount = JSON_arr_size(TilemapArray);
                        WorkingLayer->TilemapTileIndices = PushArray(&GameState->GameArena, int32, WorkingLayer->TilemapTileCount);

                        size_t TileIndex = 0;
                        size_t MaxTileIndex = 0;
                        JSON_val *TileValue;

                        int32 Index = 0;
                        JSON_arr_foreach(TilemapArray, TileIndex, MaxTileIndex, TileValue)
                        {
                            WorkingLayer->TilemapTileIndices[Index++] = JSON_get_int(TileValue);
                        }
                    }

                    const char *LayerName = JSON_get_str(JSON_obj_get(Value, "name"));
                    if(LayerName && (strcmp(LayerName, "collision_layer") == 0))
                    {
                        JSON_val *CollisionGrid = JSON_obj_get(Value, "grid");
                        if(CollisionGrid)
                        {
                            Result->CollisionLayerIndexCount = JSON_arr_size(CollisionGrid);
                            Result->CollisionLayerData       = PushArray(&GameState->GameArena, bool32, Result->CollisionLayerIndexCount);

                            size_t TileIndex = 0;
                            size_t MaxTileIndex = 0;
                            JSON_val *TileValue;

                            int32 Index = 0;
                            JSON_arr_foreach(CollisionGrid, TileIndex, MaxTileIndex, TileValue)
                            {
                                const char *TileColliderStringValue = JSON_get_str(TileValue);
                                int32 TileColliderValue = atoi(TileColliderStringValue);
                                
                                Result->CollisionLayerData[Index++] = TileColliderValue;
                            }
                        }
                    }
                    else if(LayerName && (strcmp(LayerName, "entity_layer") == 0))
                    {
                        JSON_val *EntityArray = JSON_obj_get(Value, "entities");
                        if(EntityArray)
                        {
                            Result->LevelEntityCount = JSON_arr_size(EntityArray);
                            Result->LevelEntities    = PushArray(&GameState->GameArena, entity, Result->LevelEntityCount);
                            
                            size_t EntityIndex   = 0;
                            size_t MaxEntityIndex= 0;
                            JSON_val *EntityValue;
                            JSON_arr_foreach(EntityArray, EntityIndex, MaxEntityIndex, EntityValue)
                            {
                                entity *NewEntity = CreateEntity(GameState);

                                JSON_val *Values    = JSON_obj_get(EntityValue, "values");
                                JSON_val *Archetype = JSON_obj_get(Values, "entity_archetype");
                                int32     EntityArchetype = JSON_get_int(Archetype);

                                real32    EntityPositionX = (real32)JSON_get_int(JSON_obj_get(EntityValue, "x"));
                                real32    EntityPositionY = (real32)JSON_get_int(JSON_obj_get(EntityValue, "y"));

                                NewEntity->Archetype = (entity_arch)EntityArchetype;
                                NewEntity->Position  = vec2{-EntityPositionX - Result->LevelOffsetX,
                                    -EntityPositionY + Result->LevelOffsetY};
                                switch(NewEntity->Archetype)
                                {
                                    case ARCH_STROBBY:
                                    {
                                        SetupEntityStrobby(NewEntity);
                                        NewEntity->OnCollide = &StrobbyCollision;
                                    }break;
                                    // NOTE(Sleepster): I'm lazy asf and running out of time so ALL tiles in the entity layer are moving
                                    case ARCH_TILE:
                                    {
                                        JSON_val *TargetJSON = JSON_obj_get(EntityValue, "values");
                                        real32 EntityTargetPositionBx = (real32)JSON_get_int(JSON_obj_get(TargetJSON, "second_target_x"));
                                        real32 EntityTargetPositionBy = (real32)JSON_get_int(JSON_obj_get(TargetJSON, "second_target_y"));
                                        real32 EntityTravelTime       = (real32)JSON_get_int(JSON_obj_get(TargetJSON, "platform_travel_time"));
                                        real32 EntityStationaryTime   = (real32)JSON_get_int(JSON_obj_get(TargetJSON, "platform_stationary_time"));
                                        real32 PlatformSizeX          = (real32)JSON_get_int(JSON_obj_get(TargetJSON, "platform_size_x"));
                                        real32 PlatformSizeY          = (real32)JSON_get_int(JSON_obj_get(TargetJSON, "platform_size_y"));

                                        EntityTargetPositionBx = -EntityTargetPositionBx - Result->LevelOffsetX;
                                        EntityTargetPositionBy = -EntityTargetPositionBy + Result->LevelOffsetY;

                                        real32 TargetDifferenceX = EntityTargetPositionBx - NewEntity->Position.X;
                                        real32 TargetDifferenceY = EntityTargetPositionBy - NewEntity->Position.Y;

                                        SetupEntityMovingPlatform(NewEntity,
                                                                  NewEntity->Position,
                                                                  {NewEntity->Position.X + TargetDifferenceX, NewEntity->Position.Y - TargetDifferenceY},
                                                                  EntityTravelTime,
                                                                  EntityStationaryTime,
                                                                  vec2{PlatformSizeX, PlatformSizeY},
                                                                  1);
                                    }break;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                cl_Error("Could not extract the JSON file root\n");
            }
        }
        else
        {
            cl_Error("Failure to get the file's JSON data, is this a JSON File?\n");
        }
    }
    else
    {
        cl_Error("Failure to read the file data!\n");
    }

    return(ProcessLoadedMapData(GameState, Result));
}
#endif

enum level_layer_type
{
    TYPE_none,
    TYPE_entities,
    TYPE_tilemap_data,
    TYPE_count
};

struct ldtk_entity_data
{
    int32  EntityArchetype;
    int32  WorldX;
    int32  WorldY;
};

struct ldtk_tile_data
{
    ivec2 Position;
    ivec2 AtlasOffset;

    int32 TileValue;
};

struct ldtk_level_layer_data
{
    string Identifier;
    int32  Type;

    int32  WidthInTiles;
    int32  HeightInTiles;

    int32  TileSize;

    int32  TotalOffsetX;
    int32  TotalOffsetY;

    size_t            LevelEntityCount;
    ldtk_entity_data *LevelEntities;

    int32             GridWidth;
    int32             GridHeight;

    int32             TotalTileCount;
    ldtk_tile_data   *TileData;
};

struct ldtk_level_data
{
    int32 PixelWidth;
    int32 PixelHeight;

    size_t                 LayerCount;
    ldtk_level_layer_data *LevelLayers;
};

struct ldtk_map_data
{
    int32            MapID;

    size_t           MapLevelCount;
    ldtk_level_data *LevelData;
};

internal ldtk_map_data*
ProccessJSONLevelData(game_state *GameState, ldtk_map_data *MapData)
{
    for(uint32 LevelIndex = 0;
        LevelIndex < MapData->MapLevelCount;
        ++LevelIndex)
    {
        for(uint32 LayerIndex = 0;
            LayerIndex < MapData->LevelData[LevelIndex].LayerCount;
            ++LayerIndex)
        {
            if(MapData->LevelData[LevelIndex].LevelLayers[LayerIndex].LevelEntities)
            {
                for(uint32 EntityIndex = 0;
                    EntityIndex < MapData->LevelData[LevelIndex].LevelLayers[LayerIndex].LevelEntityCount;
                    ++EntityIndex)
                {
                    ldtk_entity_data *ActiveData = &MapData->LevelData[LevelIndex].LevelLayers[LayerIndex].LevelEntities[EntityIndex];
                    entity *Entity = CreateEntity(GameState);
                    Entity->Archetype = (entity_arch)ActiveData->EntityArchetype;
                    switch(Entity->Archetype)
                    {
                        case ARCH_STROBBY:
                        {
                            SetupEntityStrobby(Entity);
                            Entity->OnCollide = &StrobbyCollision;

                        }break;
                        case ARCH_TILE:
                        {
                        }break;
                    }

                    Entity->Position  = vec2{MapData->LevelData[LevelIndex].PixelHeight - ActiveData->WorldX - Entity->RenderSize.X,
                                            (MapData->LevelData[LevelIndex].PixelHeight - ActiveData->WorldY - Entity->RenderSize.Y)};
                    Entity->PhysicsBodyData.CollisionRect.Position = Entity->Position;
                    Entity->PhysicsBodyData.CollisionRect.HalfSize = (Entity->RenderSize * 0.5f);
                }
            }
            else if(MapData->LevelData[LevelIndex].LevelLayers[LayerIndex].TileData &&
                    (strcmp(CSTR(MapData->LevelData[LevelIndex].LevelLayers[LayerIndex].Identifier), "tile_grid")) == 0)
            {
                ldtk_level_layer_data *CollisionLayer = {};
                for(uint32 TestLayerIndex = 0;
                    TestLayerIndex < MapData->LevelData[LevelIndex].LayerCount;
                    ++TestLayerIndex)
                {
                    ldtk_level_layer_data *TestLayer = &MapData->LevelData[LevelIndex].LevelLayers[TestLayerIndex];
                    if(TestLayer && (strcmp(CSTR(TestLayer->Identifier), "collision_mask")) == 0)
                    {
                        CollisionLayer = TestLayer;
                        break;
                    }
                }
                
                for(int32 TileIndex = 0;
                    TileIndex < MapData->LevelData[LevelIndex].LevelLayers[LayerIndex].TotalTileCount;
                    ++TileIndex)
                {
                    ldtk_tile_data *Tile = &MapData->LevelData[LevelIndex].LevelLayers[LayerIndex].TileData[TileIndex];
                    if(Tile->TileValue > 0)
                    {
                        entity *Entity = CreateEntity(GameState);
                        //int32 TileValue;
                        Entity->Archetype = ARCH_TILE;
                        Entity->Position  = vec2{MapData->LevelData[LevelIndex].PixelWidth  - (real32)Tile->Position.X - 8,
                            MapData->LevelData[LevelIndex].PixelHeight - (real32)Tile->Position.Y - 8};
                        Entity->StaticSprite.AtlasOffset = Tile->AtlasOffset;
                        Entity->StaticSprite.SpriteSize  = {8, 8};
                        Entity->LayerIndex = LAYER_Background;

                        if(CollisionLayer && CollisionLayer->TileData[TileIndex].TileValue > 0)
                        {
                            Entity->PhysicsBodyData.BodyType = PB_Solid;
                            Entity->PhysicsBodyData.CollisionRect.Position = Entity->Position;
                            Entity->PhysicsBodyData.CollisionRect.HalfSize = v2Cast(TILE_SIZE * 0.5f);

                            Entity->PhysicsBodyData.CollisionRect.Min      = Entity->Position - Entity->PhysicsBodyData.CollisionRect.HalfSize * 2.0f; 
                            Entity->PhysicsBodyData.CollisionRect.Max      = Entity->Position - Entity->PhysicsBodyData.CollisionRect.HalfSize * 2.0f; 
                            switch(CollisionLayer->TileData[TileIndex].TileValue)
                            {
                                case 2: Entity->OnCollide = &SpikeCollision; break;
                                case 3: Entity->Flags    |= IS_ONE_WAY_COLLISION; break;
                                case 4: Entity->Flags    |= IS_CLIMBABLE; break;
                            }
                        }
                    }
                }
            }
        }
    }
    return(MapData);
}

internal ldtk_map_data*
LoadJSONLevelData(game_state *GameState, string Filepath)
{
    string EntireFile = ReadEntireFileMA(&GameState->GameArena, Filepath);
    ldtk_map_data *Result = PushStruct(&GameState->GameArena, ldtk_map_data);
    if(EntireFile != NULLSTR)
    {
        JSON_doc *JSONData = JSON_read(CSTR(EntireFile), EntireFile.Length, 0);
        if(JSONData)
        {
            JSON_val *MapRoot = JSON_doc_get_root(JSONData);
            if(MapRoot)
            {
                JSON_val *LevelsArray = JSON_obj_get(MapRoot, "levels");
                Result->MapLevelCount = JSON_arr_size(LevelsArray);
                Result->LevelData     = PushArray(&GameState->GameArena, ldtk_level_data, Result->MapLevelCount);
                size_t    LevelIndex = 0;
                size_t    LevelCount = 0;
                JSON_val *LevelData  = 0;

                JSON_arr_foreach(LevelsArray, LevelIndex, LevelCount, LevelData)
                {
                    ldtk_level_data *CurrentLevel = &Result->LevelData[LevelIndex];
                    CurrentLevel->PixelWidth  = JSON_get_int(JSON_obj_get(LevelData, "pxWid"));
                    CurrentLevel->PixelHeight = JSON_get_int(JSON_obj_get(LevelData, "pxHei"));
                    JSON_val *LayerArray = JSON_obj_get(LevelData, "layerInstances");
                    size_t    LayerCount = JSON_arr_size(LayerArray);
                    CurrentLevel->LayerCount = LayerCount; 
                    if(LayerCount > 0)
                    {
                        CurrentLevel->LevelLayers = PushArray(&GameState->GameArena, ldtk_level_layer_data, LayerCount);
                    }

                    size_t    LayerIndex = 0;
                    size_t    MaxLayer = 0;
                    JSON_val *LayerData = 0;
                    JSON_arr_foreach(LayerArray, LayerIndex, MaxLayer, LayerData)
                    {
                        ldtk_level_layer_data *CurrentLayer = &CurrentLevel->LevelLayers[LayerIndex];
                        CurrentLayer->Identifier    = STR(JSON_get_str(JSON_obj_get(LayerData, "__identifier")));
                        CurrentLayer->WidthInTiles  = JSON_get_int(JSON_obj_get(LayerData,     "__cWid"));
                        CurrentLayer->HeightInTiles = JSON_get_int(JSON_obj_get(LayerData,     "__cHei"));
                        CurrentLayer->TileSize      = JSON_get_int(JSON_obj_get(LayerData,     "__gridSize")); 
                        CurrentLayer->TotalOffsetX  = JSON_get_int(JSON_obj_get(LayerData,     "__pxTotalOffsetX"));
                        CurrentLayer->TotalOffsetX  = JSON_get_int(JSON_obj_get(LayerData,     "__pxTotalOffsetY"));

                        const char *LayerType = JSON_get_str(JSON_obj_get(LayerData, "__type"));
                        if(CurrentLayer->Identifier != NULLSTR && (strcmp(LayerType, "Entities") == 0))
                        {
                            CurrentLayer->Type = TYPE_entities;

                            JSON_val *EntityArray = JSON_obj_get(LayerData, "entityInstances");
                            CurrentLayer->LevelEntityCount = JSON_arr_size(EntityArray);
                            if(CurrentLayer->LevelEntityCount > 0)
                            {
                                CurrentLayer->LevelEntities = PushArray(&GameState->GameArena, ldtk_entity_data,
                                                                        CurrentLayer->LevelEntityCount);
                            }

                            size_t    EntityIndex    = 0;
                            size_t    MaxEntityIndex = 0;
                            JSON_val *EntityData     = 0;
                            JSON_arr_foreach(EntityArray, EntityIndex, MaxEntityIndex, EntityData)
                            {
                                CurrentLayer->LevelEntities[EntityIndex].WorldX = JSON_get_int(JSON_obj_get(EntityData, "__worldX"));
                                CurrentLayer->LevelEntities[EntityIndex].WorldY = JSON_get_int(JSON_obj_get(EntityData, "__worldY"));

                                JSON_val *EntityMetadata = JSON_obj_get(EntityData, "fieldInstances");
                                size_t    DataIndex    = 0;
                                size_t    MaxDataIndex = 0;
                                JSON_val *MetaData     = 0;
                                JSON_arr_foreach(EntityMetadata, DataIndex, MaxDataIndex, MetaData)
                                {
                                    const char *Identifier = JSON_get_str(JSON_obj_get(MetaData, "__identifier"));
                                    if((strcmp(Identifier, "entity_archetype")) == 0)
                                    {
                                        CurrentLayer->LevelEntities[EntityIndex].EntityArchetype = JSON_get_int(JSON_obj_get(MetaData, "__value"));
                                    }
                                }
                            }
                        }
                        else if(CurrentLayer->Identifier != NULLSTR && (strcmp(LayerType, "IntGrid") == 0))
                        {
                            CurrentLayer->Type = TYPE_tilemap_data;
                            JSON_val *GridData = JSON_obj_get(LayerData, "intGridCsv");
                            size_t GridSize    = JSON_arr_size(GridData);
                            if(GridSize > 0)
                            {
                                CurrentLayer->TileData = PushArray(&GameState->GameArena, ldtk_tile_data, GridSize);
                            }

                            size_t    GridIndex = 0;
                            size_t    MaxIndex  = 0;
                            JSON_val *GridValue = 0;
                            int32 MapIndex = 0;

                            JSON_arr_foreach(GridData, GridIndex, MaxIndex, GridValue)
                            {
                                int32 Temp = JSON_get_int(GridValue);
                                if(Temp > 0)
                                {
                                    CurrentLayer->TileData[MapIndex++].TileValue = Temp;
                                }
                            }
                            CurrentLayer->TotalTileCount = MapIndex; 

                            JSON_val *AutoTilingData = JSON_obj_get(LayerData, "autoLayerTiles");
                            if(AutoTilingData)
                            {
                                GridIndex = 0;
                                MaxIndex  = 0;
                                GridValue = 0;
                                MapIndex = 0;
                                JSON_arr_foreach(AutoTilingData, GridIndex, MaxIndex, GridValue)
                                {
                                    ldtk_tile_data *CurrentTile = &CurrentLayer->TileData[MapIndex++];
                                    JSON_val *PixelPositionData = JSON_obj_get(GridValue, "px");
                                    size_t    DimensionCount = 0;
                                    size_t    MaxDimension   = 0;
                                    JSON_val *DimensionValue = 0;

                                    int32 Dimension = 0;
                                    JSON_arr_foreach(PixelPositionData, DimensionCount, MaxDimension, DimensionValue)
                                    {
                                        CurrentTile->Position.Elements[Dimension++] = JSON_get_int(DimensionValue);
                                    }

                                    JSON_val *TexelCoordArray = JSON_obj_get(GridValue, "src");
                                    DimensionCount = 0;
                                    MaxDimension   = 0;
                                    DimensionValue = 0;

                                    Dimension = 0;
                                    JSON_arr_foreach(TexelCoordArray, DimensionCount, MaxDimension, DimensionValue)
                                    {
                                        CurrentTile->AtlasOffset.Elements[Dimension++] = JSON_get_int(DimensionValue);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                cl_Error("Failed to extract the level Root!\n");
            }
        }
        else
        {
            cl_Error("Failure to extract the JSON data!\n");
        }
    }
    else
    {
        cl_Error("Failure to Load the map file!\n");
    }
    return(ProccessJSONLevelData(GameState, Result));
}
