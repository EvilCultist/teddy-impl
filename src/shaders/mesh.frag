#version 300 es
precision mediump float;

out vec4 FragColor;

in vec3 loc;

// uniform sampler2D screenTexture;

void main()
{
    // FragColor = vec4(loc, 1.0);
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
