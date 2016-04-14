#version 330
in vec2 vTexCoord;

uniform sampler2D inputTex;
uniform sampler2D colorTex;

layout(location = 0) out vec4 fragColor;

void main()
{
	float ao = texture(inputTex, vTexCoord).r;
	vec3 color = texture(colorTex, vTexCoord).rgb;
    fragColor = vec4(vec3(ao) * color, 1);
}