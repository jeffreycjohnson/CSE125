#version 430
precision mediump float;

uniform sampler2D colorTex; //color texture - rgb: color
uniform sampler2D roughnessTex;
uniform sampler2D metalnessTex;
uniform sampler2D heightTex;
uniform sampler2D normalTex; //normal texture - rgb: normal | a: unused

uniform vec3 CameraPos;

in vec2 vTexCoord;
in vec3 vNormal;
in vec4 vPosition;
in vec3 vTangent;
in vec3 vBitangent;

layout (location = 0) out vec4 ColorOut; //color texture - rgb: color | a: metalness
layout (location = 1) out vec4 NormOut; //normal texture - rgb: normal | a: height
layout (location = 2) out vec4 PosOut; //position texture - rgb: position | a: roughness
layout (location = 3) out vec4 foo;

void main()
{
	float roughness = texture(roughnessTex, vTexCoord).r;
	float metalness = texture(metalnessTex, vTexCoord).r;
	float height = texture(heightTex, vTexCoord).r;

    vec3 norm = normalize(vNormal);
    vec3 tangent = normalize(vTangent);
    tangent = normalize(tangent - dot(tangent, norm) * norm);
    vec3 binormal = cross(tangent, norm);
    mat3 model = mat3(tangent, binormal, norm);

    norm = 2 * texture(normalTex, vTexCoord).xyz - vec3(1.0);
    NormOut = vec4(normalize(model * norm) * 0.5 + 0.5, height);

	vec4 color = texture(colorTex, vTexCoord);
    ColorOut = vec4(color.rgb, metalness);
    PosOut = vec4(vPosition.xyz/vPosition.w, roughness);
    foo = vec4(0);
}
