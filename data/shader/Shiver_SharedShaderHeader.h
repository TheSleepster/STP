/* date = June 17th 2024 3:14 pm */

#ifndef _SHIVER__SHARED_SHADER_HEADER_H
#define _SHIVER__SHARED_SHADER_HEADER_H

#ifdef ENGINE // ENGINE ONLY
#include "Intrinsics.h"
#include "util/Math.h"
#include "util/MemoryArena.h"
#include "../../code/Shiver_Renderer.h"

#else // SHADER ONLY
#endif

// SHARED

const unsigned int RENDERING_OPTION_FLIP_X = 0x00000001u;
const unsigned int RENDERING_OPTION_FLIP_Y = 0x00000002u;
const unsigned int RENDERING_OPTION_FONT   = 0x00000004u;

struct material 
{
    vec4 Color;
};

struct renderertransform
{
    vec4 MaterialColor;
    vec2 WorldPosition; // Position in the world, it is top left aligned;
    vec2 Size;          // Displayed size of the Sprite
    ivec2 AtlasOffset;  // Atlas offset
    ivec2 SpriteSize;   // Size of the sprite inside the atlas
                        
    unsigned int RenderingOptions;
    unsigned int LayerMask;

    int padding[2];
};

#endif //_SHIVER__SHARED_SHADER_HEADER_H
