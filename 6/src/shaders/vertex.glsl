#version 330 core

uniform mat4 MVP;

layout(location = 0) in vec3 vertexPosition;

out vec3 fragmentColor;

void main()
{
    vec4 vertex = vec4(vertexPosition, 1);
    gl_Position = MVP * vertex;
    fragmentColor = vec3(1.0, 1.0, 1.0);
}
