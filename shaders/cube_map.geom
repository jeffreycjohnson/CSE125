#version 330

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 uVP_Matrices[6];
out vec4 vPos;

void main() {
	for(int f = 0; f < 6; f++) {
		gl_Layer = f;
		for(int v = 0; v < 3; v++) {
			vPos = gl_in[v].gl_Position;
			gl_Position = uVP_Matrices[f] * vPos;
			EmitVertex();
		}
		EndPrimitive();
	}
}