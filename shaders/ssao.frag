#version 330

uniform vec3 uSamples[128];
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

const float posBias = 0.06;
const float whiteVal = 0.6;
const float blackVal = 0.2;
const float intensityScale = 2.0f;

void main() {
	float ao = 0;
	vec2 uvScale = textureSize(posTex, 0).xy / textureSize(rotationTex, 0).xy;
	vec3 pos = texture(posTex, vTexCoord).xyz;
	// convert normal to view space
	vec4 tmp = uV_Matrix * vec4(texture(normalTex, vTexCoord).xyz * 2 - 1, 1);
	vec3 normal = normalize((tmp/tmp.w).xyz);
	// offset the z to avoid self shadowing
	pos += vec3(0,0,posBias);
	vec3 random = texture(rotationTex, vTexCoord * uvScale).xyz;
	random.x = random.x * 2 - 1.f;
	random.y = random.y * 2 - 1.f;
	random = normalize(random);

	// creates a matrix to convert from tangent space to view space
	vec3 tangent = normalize(random - normal * dot(random, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	uint rejected = 0u;
	for(uint i = 0u; i < uSampleCount; i++) {
		// get random points in a normal aligned hemisphere
		vec3 dir = TBN * uSamples[i] * uRadius;
		vec3 samplePos = dir + pos;
		// discard points that are too close to parallel to the surface to about self shadowing
		if(dot(normal, normalize(dir)) > 0.15) {
			// find the screen position of the point
			vec4 sampleUV = uP_Matrix * vec4(samplePos, 1);
			sampleUV /= sampleUV.w;
			sampleUV = sampleUV / 2 + 0.5;

			float sampleDepth = texture(posTex, sampleUV.xy).z;
			// get rid of black halos around abjects
			float factor = smoothstep(0.0, 1.0, uRadius / abs(pos.z - sampleDepth));
			// actual check for occlusion
			ao += (sampleDepth >= samplePos.z ? 1.0 : 0.0) * factor;
		}
		else rejected++;
	}
	// scale final ao value to look better
	finalAO = vec4(vec3(clamp(1.0 - ao / (uSampleCount - rejected), 0.0, whiteVal) * intensityScale - blackVal), 1);
}