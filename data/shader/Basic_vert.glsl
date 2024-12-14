layout(std430, Binding = 0) buffer TransformSBO
{
    renderertransform Transforms[];
};

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform vec2 ScreenSize;

// Output
layout(location = 0) out vec2 TextureCoordsOut;
layout(location = 1) out flat vec4 MaterialColor;
layout(location = 2) out flat unsigned int RenderingOptions;

void main()
{
    renderertransform Transform = Transforms[gl_InstanceID];
    
    vec2 Vertices[6] =
    {
        Transform.WorldPosition,
        vec2(Transform.WorldPosition + vec2(0.0f, Transform.Size.y)),
        vec2(Transform.WorldPosition + vec2 (Transform.Size.x, 0.0f)),
        vec2(Transform.WorldPosition + vec2(Transform.Size.x, 0.0f)),
        vec2(Transform.WorldPosition + vec2(0.0f, Transform.Size.y)),
        Transform.WorldPosition + Transform.Size,
    };
    
    float Left = Transform.AtlasOffset.x;
    float Top = Transform.AtlasOffset.y;
    float Right = Transform.AtlasOffset.x + Transform.SpriteSize.x;
    float Bottom = Transform.AtlasOffset.y + Transform.SpriteSize.y;

    if(bool(Transform.RenderingOptions & RENDERING_OPTION_FLIP_X))
    {
        float Temp = Left;
        Left = Right;
        Right = Temp;
    }

    if(bool(Transform.RenderingOptions & RENDERING_OPTION_FLIP_Y))
    {
        float Temp = Top;
        Top = Bottom;
        Bottom = Temp;
    }
    
    vec2 TextureCoords[6] =
    {
        vec2(Left, Top),
        vec2(Left, Bottom),
        vec2(Right, Top),
        vec2(Right, Top),
        vec2(Left, Bottom),
        vec2(Right, Bottom),
    };
    
    vec2 VertexPos = Vertices[gl_VertexID];
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(VertexPos, 0.0f, 1.0f);

    TextureCoordsOut = TextureCoords[gl_VertexID];
    MaterialColor = Transform.MaterialColor;
    RenderingOptions = Transform.RenderingOptions;
}
