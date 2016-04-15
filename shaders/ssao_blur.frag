#version 330
in vec2 vTexCoord;

uniform sampler2D inputTex;
uniform sampler2D colorTex;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(inputTex, 0));
    float result = 0.0;
    for (float x = -1.5; x <= 1.5; ++x) 
    {
        for (float y = -1.5; y <= 1.5; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(inputTex, vTexCoord + offset).r;
        }
    }
    fragColor = texture(colorTex, vTexCoord) * result / (4.0 * 4.0);
}