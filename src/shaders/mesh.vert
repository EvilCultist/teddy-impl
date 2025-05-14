#version 300 es
precision mediump float;
layout(location = 0) in vec3 aPos;
// layout(location = 1) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 loc;

void main()
{
    vec4 pos = proj * view * model * vec4(aPos, 1.0);
    gl_Position = pos;
    loc = vec3(pos.x, pos.y, pos.z);
    // TexCoords = aTexCoords;
}
