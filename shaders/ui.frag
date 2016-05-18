#version 330

uniform vec4 uColor;
uniform sampler2D tex;
in vec2 vTexCoord;
out vec4 fragColor;

void main() {
    fragColor = uColor * texture(tex, vTexCoord);
}
