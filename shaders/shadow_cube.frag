#version 330
precision mediump float;

in vec4 vPos;

void main()
{
    float bias = 0.0001;
    gl_FragDepth = gl_FragCoord.z + bias;
}
