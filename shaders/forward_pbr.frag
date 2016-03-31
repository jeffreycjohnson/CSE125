#version 430
precision mediump float;

#include shaders/lighting.glsl

in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vTangent;
in vec3 vBitangent;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec4 frag_normal;
layout(location = 2) out vec4 frag_material;

uniform sampler2D colorTex; //color texture - rgb: color | a: team mask
uniform sampler2D matTex; //material texture - r: metalness | g: IOR | b: roughness | a: unused
uniform sampler2D normalTex; //normal texture - rgb: normal | a: unused

//world space camera position, to get view vector
uniform vec3 cameraPos;

//light data - (position.xyz, unused) followed by (lightColor.xyz, lightType)
const int lightCount = 5;
uniform vec4 uLightData[3*lightCount];

void main () {
  vec4 albedo = texture(colorTex, vTexCoord);
  vec3 mat = texture(matTex, vTexCoord).rgb;

  vec3 normal_tangent = 2*texture(normalTex, vTexCoord).rgb - 1;
  vec3 normal = normalize(vTangent * normal_tangent.x + vBitangent * normal_tangent.y + vNormal * normal_tangent.z);
  vec3 view = normalize(cameraPos - vPosition.xyz);


  mat.b += 0.01; //there seem to be issues with roughness = 0 due to visibility
  float a = sqrt(mat.z);// squaring it makes everything shiny, sqrting it looks like linear roughness

  vec3 F0 = IorToF0(mat.y, mat.x, albedo.rgb);

  vec3 envColor = envMap(F0, normal.xyz, albedo.rgb, view, a, mat.x);
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

  vec3 diffuseColor = ((1.0-mat.r) * albedo.rgb) * diffuseLight;
  vec3 color = diffuseColor + specColor + envColor;
  

  frag_color = vec4(color, albedo.a);
  frag_normal = vec4(normal, 1.0);
  frag_material = vec4(mat, 1.0);
}
