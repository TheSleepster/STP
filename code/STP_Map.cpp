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
    bool32 *CollisionLayerData;

    size_t LevelLayerCount;
    stp_level_layer_data *Layers;
};

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
                        (TilemapWidthInTiles - 1 - (TileIndex % CurrentWorkingLayer->TilemapLayerWidth) * TILE_SIZE.X)   - LevelData->LevelOffsetX,
                        (TilemapHeightInTiles  - 1 - (TileIndex / CurrentWorkingLayer->TilemapLayerWidth) * TILE_SIZE.Y) + LevelData->LevelOffsetY,
                    };

                    entity *Entity = CreateEntity(GameState);
                    Entity->Archetype = ARCH_TILE;
                    Entity->Position = v2Cast(TileWorldPosition);
                    Entity->StaticSprite = {.AtlasOffset = TileUVData, .SpriteSize = TILE_SIZE};
                    Entity->LayerIndex = int32(LayerCounter--);
                    Entity->PhysicsBodyData.Friction = 12.0f;

                    if(LevelData->CollisionLayerData)
                    {
                        if(LevelData->CollisionLayerData[TileIndex] != 0)
                        {
                            Entity->PhysicsBodyData.BodyType = PB_Static;
                            Entity->PhysicsBodyData.CollisionRect.HalfSize = v2Cast(TILE_SIZE * 0.5f);
                            Entity->PhysicsBodyData.CollisionRect.Position = Entity->Position;
                        }
                    }
                }
                else continue;
            }
        }
    }
    
    return(LevelData);
}

internal stp_level_data*
LoadOGMOLevel(game_state *GameState, ivec2 MapOffset)
{
    stp_level_data *Result = PushStruct(&GameState->GameArena, stp_level_data);
    
    string EntireFile = ReadEntireFileMA(&GameState->GameArena, STR("../data/res/maps/TestLevel.json"));
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

                Result->LevelOffsetX = MapOffset.X;
                Result->LevelOffsetY = MapOffset.Y;

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
                                
                                Result->CollisionLayerData[Index++] = (TileColliderValue != 0);
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
