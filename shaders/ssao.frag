#version 330

uniform vec3 uSamples[64];
uniform uint uSampleCount;
uniform float uRadius;
uniform sampler2D posTex;
uniform sampler2D normalTex;
uniform sampler2D rotationTex;


uniform mat4 uM_Matrix;
uniform mat4 uV_Matrix;
uniform mat4 uP_Matrix;

in vec2 vTexCoord;
out vec4 finalAO;

void main() {
	float ao = 0;
	vec2 uvScale = textureSize(posTex, 0).xy / textureSize(rotationTex, 0).xy;
	vec3 pos = texture(posTex, vTexCoord).xyz;
	vec4 tmp = uV_Matrix * vec4(texture(normalTex, vTexCoord).xyz * 2 - 1, 1);
	vec3 normal = normalize((tmp/tmp.w).xyz);
	pos -= normal * 0.05;
	vec3 random = texture(rotationTex, vTexCoord * uvScale).xyz;
	random.x = random.x * 2 - 1.f;
	random.y = random.y * 2 - 1.f;
	random = normalize(random);

	vec3 tangent = normalize(random - normal * dot(random, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	for(uint i = 0u; i < uSampleCount; i++) {
	vec3 s = uSamples[i];
	s.z = -s.z;
		vec3 samplePos = TBN * s * uRadius + pos;
		vec4 sampleUV = uP_Matrix * vec4(samplePos, 1);
		sampleUV /= sampleUV.w;
		sampleUV = sampleUV / 2 + 0.5;

		float sampleDepth = texture(posTex, sampleUV.xy).z;
		float factor = smoothstep(0.0, 1.0, uRadius / abs(pos.z - sampleDepth));
		ao += (sampleDepth >= samplePos.z ? 1.0 : 0.0);
	}
	finalAO = vec4(vec3(1.0 - (ao / uSampleCount)), 1);
}