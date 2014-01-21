#version 330 core

uniform mat4 MVP;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 uv;

out vec2 UV;

void main()
{
    vec4 vertex = vec4(vertexPosition, 1);
    gl_Position = MVP * vertex;
    UV = uv;
}
