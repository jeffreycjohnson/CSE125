#version 330
precision mediump float;

#include shaders/geom_pass.glsl

uniform sampler2D colorTex; //color texture - rgb: color
uniform sampler2D roughnessTex;
uniform sampler2D metalnessTex;

uniform vec3 cameraPos;

in vec2 vTexCoord;
in vec3 vNormal;
in vec4 vPosition;
in vec4 vWorldPosition;
in vec3 vTangent;
in vec3 vBitangent;

layout (location = 0) out vec4 ColorOut; //color texture - rgb: color | a: metalness
layout (location = 1) out vec4 NormOut; //normal texture - rgb: normal | a: unused
layout (location = 2) out vec4 PosOut; //position texture - rgb: position | a: roughness
layout (location = 3) out vec4 foo;

void main()
{
	mat3 TBN = computeTBN(vNormal, vTangent, vBitangent);
	vec2 UV = POM(vTexCoord, normalize(transpose(TBN) * (vWorldPosition.xyz/vWorldPosition.w - cameraPos)));
	float height = uParallax ? texture(heightTex, UV).r * uDepthScale : 0.0;
	
	float roughness = texture(roughnessTex, UV).r;
	float metalness = texture(metalnessTex, UV).r;

    NormOut = vec4(normalMap(UV, TBN) * 0.5 + 0.5, 1.0);
    ColorOut = vec4(texture(colorTex, UV).rgb, metalness);
    PosOut = vec4(vPosition.xyz/vPosition.w - vec3(0, 0, height), roughness);
    foo = vec4(0);
}
