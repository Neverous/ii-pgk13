#version 110

uniform mat4 MVP;

attribute vec3 vertexPosition;

void main()
{
    vec4 vertex = vec4(vertexPosition, 1.0);
    gl_Position = MVP * vertex;
}
