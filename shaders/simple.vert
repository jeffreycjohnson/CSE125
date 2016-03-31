#version 330

layout(location = 0) in vec4 aPosition;

uniform mat4 uM_Matrix;
uniform mat4 uV_Matrix;
uniform mat4 uP_Matrix;
uniform vec4 uPosition;
uniform vec4 uScale;

void main(){
	vec4 pos = aPosition * uScale + uPosition;
	pos.w = 1;
	gl_Position = uP_Matrix * uV_Matrix * uM_Matrix * pos;
}
