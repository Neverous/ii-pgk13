#version 330 core

uniform mat4 MVP;

layout(location = 0) in vec3 vertexPosition;

void main()
{
    vec4 vertex = vec4(vertexPosition, 1);
    gl_Position = MVP * vertex;
}
