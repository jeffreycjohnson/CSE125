#version 330

layout(location = 0) in vec4 aPosition;
layout(location = 2) in vec2 aTexCoord;
out vec2 vTexCoord;

void main(){
	gl_Position = vec4(aPosition.xy,0.1,1);
	vTexCoord = aTexCoord;
}
