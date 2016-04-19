#version 410
precision mediump float;

#include shaders/lighting.glsl
#include shaders/geom_pass.glsl

in vec4 vPosition;
in vec4 vWorldPosition;
in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vTangent;
in vec3 vBitangent;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec4 frag_normal;
layout(location = 2) out vec4 frag_material;

uniform sampler2D colorTex; //color texture - rgb: color
uniform sampler2D roughnessTex;
uniform sampler2D metalnessTex;

//world space camera position, to get view vector
uniform vec3 cameraPos;

//light data - (position.xyz, unused) followed by (lightColor.xyz, lightType)
const int lightCount = 5;
uniform vec4 uLightData[3*lightCount];

void main () {
  mat3 TBN = computeTBN(vNormal, vTangent, vBitangent);
  vec3 view = normalize(vWorldPosition.xyz/vWorldPosition.w - cameraPos);
  vec2 UV = POM(vTexCoord, normalize(transpose(TBN) * view));

  vec4 albedo = texture(colorTex, UV);
  float roughness = texture(roughnessTex, UV).r;
  float metalness = texture(metalnessTex, UV).r;
  vec3 normal = normalMap(UV, TBN);

  roughness += 0.01; //there seem to be issues with roughness = 0 due to visibility
  float a = sqrt(roughness);// squaring it makes everything shiny, sqrting it looks like linear roughness
  
  vec3 F0 = MetalToF0(metalness, albedo.rgb);

  vec3 envColor = envMap(F0, normal.xyz, albedo.rgb, view, a, metalness);
  vec3 diffuseLight = vec3(0);
  vec3 specColor = vec3(0);
  
  for (int i=0; i < lightCount; ++i) {
	float lightType = uLightData[3*i+1].w;
	float power = 1;
	vec3 lightDir = normalize(uLightData[3*i].xyz);
	if (lightType == 1) {
		lightDir =  (uLightData[3*i] - vPosition).xyz;
		float lightDist = length(lightDir);

		//Spherical light algorithm from http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
		float sphereRadius = uLightData[3*i].w;
		vec3 reflectedRay = reflect(-view, normal);
		vec3 centerToRay = dot(lightDir, reflectedRay) * reflectedRay - lightDir;
		lightDir = normalize(lightDir + centerToRay * clamp(sphereRadius / length(centerToRay), 0.0, 1.0));
		//todo normalize based on sphere size
	    power = 1.0 / (lightDist * lightDist * uLightData[3*i+2].z + lightDist * uLightData[3*i+2].y + uLightData[3*i+2].x);
	}

    diffuseLight = diffuseLight + (uLightData[3*i+1].xyz * (clamp(dot(lightDir, normal) * power, 0.0, 1.0)));
	
	vec3 halfVec = normalize(view + lightDir);
	float dotNH = clamp(dot(normal, halfVec), 0.0, 1.0);

	float a2 = a*a;
	specColor += GGX_D(dotNH, a2*a2) * SpecularBRDF(uLightData[3*i+1].xyz, normal, view, lightDir, a, F0, 1) * power;
  }

  vec3 diffuseColor = ((1.0-metalness) * albedo.rgb) * diffuseLight;
  vec3 color = diffuseColor + specColor + envColor;
  

  frag_color = vec4(color, albedo.a);
  frag_normal = vec4(normal, 1.0);
  frag_material = vec4(vec3(metalness, 1.0, roughness), 1.0);
}
