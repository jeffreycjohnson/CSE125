#version 430
precision mediump float;

#include shaders/lighting.glsl

in vec4 vPosition;
out vec4 frag_color;

uniform sampler2D colorTex; //color texture - rgb: color | a: metalness
uniform sampler2D normalTex; //normal texture - rgb: normal | a: IOR
uniform sampler2D posTex; //position texture - rgb: position | a: roughness
uniform sampler2DShadow shadowTex;

//world space camera position, to get view vector
uniform vec3 cameraPos;
uniform vec3 uLightColor;
uniform vec3 uLightPosition;
uniform vec3 uLightDirection;
uniform vec3 uLightFalloff;
uniform float uLightSize = 1.0f;
uniform vec2 uScreenSize;
uniform int uLightType;
uniform mat4 uShadow_Matrix;
uniform mat4 uIV_Matrix;

void main () {
  vec4 albedo = texture(colorTex, gl_FragCoord.xy / uScreenSize);
  if(albedo.xyz == vec3(0)) discard;
  vec4 pos = texture(posTex, gl_FragCoord.xy / uScreenSize);
  vec4 normal = texture(normalTex, gl_FragCoord.xy / uScreenSize);
  vec3 mat = vec3(albedo.a, pos.w, normal.w);
  pos = uIV_Matrix * vec4(pos.xyz, 1);
  pos /= pos.w;
  normal.xyz = normal.xyz * 2 - 1;

  vec3 view = normalize(cameraPos - pos.xyz);


  mat.y += 0.0001; //there seem to be issues with roughness = 0 due to visibility
  float a = sqrt(mat.y);// squaring it makes everything shiny, sqrting it looks like linear roughness

  vec3 F0 = IorToF0(mat.z, mat.x, albedo.rgb);

  if(uLightType == 3) {
	  frag_color = vec4(envMap(F0, normal.xyz, albedo.rgb, view, a, mat.x), 1.0);
  }
  else {
	  vec3 lightDir;
	  float lightDist;
	  float shadow = 1;
	  if(uLightType == 0) {
		  lightDir = uLightPosition - pos.xyz;
		  lightDist = length(lightDir);

		  //Spherical light algorithm from http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
		  float sphereRadius = uLightSize;
		  vec3 reflectedRay = reflect(-view, normal.xyz);
		  vec3 centerToRay = dot(lightDir, reflectedRay) * reflectedRay - lightDir;
		  lightDir = normalize(lightDir + centerToRay * clamp(sphereRadius / length(centerToRay), 0.0, 1.0));
		  //todo normalize based on sphere size
	  }
	  else {
		  lightDir = uLightDirection;
		  lightDist = 0;
		  shadow = shadowMap(pos.xyz, uShadow_Matrix, lightDir, shadowTex, normal.xyz);
	  }

	  vec3 diffuseLight = uLightColor * clamp(dot(lightDir, normal.xyz), 0.0, 1.0);
	
	  vec3 halfVec = normalize(view + lightDir);
	  float dotNH = clamp(dot(normal.xyz, halfVec), 0.0, 1.0);
	  
	  float a2 = a*a;
	  vec3 specColor = GGX_D(dotNH, a2*a2) * SpecularBRDF(uLightColor, normal.xyz, view, lightDir, a, F0, 1);

	  vec3 diffuseColor = ((1.0-mat.r) * albedo.rgb) * diffuseLight;
	  vec3 color = falloff(diffuseColor + specColor, lightDist, uLightFalloff);

	  frag_color = vec4(color * shadow, 1.0);
  }
}
