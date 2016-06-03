#version 330
precision mediump float;

in vec4 vPos;

uniform vec3 uLightPos;

void main()
{
    float bias = 0.0025;
    gl_FragDepth = length(vPos.xyz - uLightPos)/25.0 + bias;
}
