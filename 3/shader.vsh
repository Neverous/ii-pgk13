#version 330 core

uniform vec3 moveVector;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;

out vec3 fragmentColor;

void main()
{
    gl_Position.xyz = vertexPosition + moveVector;
    gl_Position.w = 1.;

    fragmentColor = vertexColor;
}
