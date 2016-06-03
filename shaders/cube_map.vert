#version 330
layout(location = 0) in vec4 aPosition;

uniform mat4 uM_Matrix;
		
out vec4 vPosition;

void main () {
	gl_Position = uM_Matrix * vec4(aPosition.xyz, 1.0);
}
