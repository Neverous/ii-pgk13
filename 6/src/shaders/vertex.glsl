#version 330 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

out vec2 UV;
out vec3 VertexPosition;
out vec3 Normal;

uniform mat4 MVP;

void main() {
    gl_Position     = MVP * vec4(vertexPosition, 1.0f);

    UV              = uv;
    VertexPosition  = vertexPosition;
    Normal          = normal;
}
