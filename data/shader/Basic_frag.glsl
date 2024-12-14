//Input
layout(location = 0) in vec2 TextureCoordsIn;
layout(location = 1) in flat vec4 MaterialColor;
layout(location = 2) in flat unsigned int RenderingOptions;

//Binding
layout(binding = 0) uniform sampler2D TextureAtlas;
layout(binding = 1) uniform sampler2D LM_FontAtlas;

// Output
layout(location = 0) out vec4 FragColor;

void main()
{
    if(bool(RenderingOptions & RENDERING_OPTION_FONT))
    {
        vec4 TextureColor = texelFetch(LM_FontAtlas, ivec2(TextureCoordsIn), 0);
        
        if(TextureColor.r == 0.0)
        {
            discard;
        }

        FragColor = TextureColor.r * MaterialColor;
    }
    else 
    {
        vec4 TextureColor = texelFetch(TextureAtlas, ivec2(TextureCoordsIn), 0);

        if(TextureColor.a == 0.0)
        {
            discard;
        }

        FragColor = TextureColor * MaterialColor;
    }
}
