uniform sampler2D heightTex;
uniform sampler2D normalTex; //normal texture - rgb: normal | a: unused

uniform float uDepthScale;
uniform bool uParallax = false;

// computes a matrix to transform from tangent space to world space
mat3 computeTBN(vec3 normal, vec3 tangent, vec3 bitangent)
{
    normal = normalize(normal);
    tangent = normalize(tangent);
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    bitangent = -normalize(bitangent);
    return mat3(tangent, bitangent, normal);
}

// Parallex Occlusion Mapping
vec2 POM(vec2 UV, vec3 view)
{
	if(!uParallax) return UV;

	// do fewer steps when viewed head on
	const float minLayers = 4;
	const float maxLayers = 36;
	float layers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), view))); 
	float layerDepth = 1.0 / layers;

	// how far to warp the uvs
	vec2 maxDelta = view.xy / view.z * uDepthScale;
	vec2 layerDelta = maxDelta / layers;

	float prevDepth = 0.0;
	float currentDepth = texture(heightTex, UV).r;
	float currentLayerDepth = 0.0;
	// step through the height field until a collision is found
	while(currentLayerDepth < currentDepth)
	{
		prevDepth = currentDepth;
		currentLayerDepth += layerDepth;
		UV -= layerDelta;
		currentDepth = texture(heightTex, UV).r;
	}

	// interpolate between the point that collides and the one before it
	vec2 prevUV = UV + layerDelta;
	currentDepth -= currentLayerDepth;
	prevDepth -= (currentLayerDepth - layerDepth);
	float weight = currentDepth / (currentDepth - prevDepth);
	return UV * (1.0 - weight) + prevUV * weight;
}

vec3 normalMap(vec2 UV, mat3 TBN)
{
    vec3 normal = 2 * texture(normalTex, UV).xyz - vec3(1.0);
    return vec3(normalize(TBN * normal));
}