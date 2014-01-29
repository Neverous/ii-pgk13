#version 330 core

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec2 in_Texture;
layout (location = 2) in vec3 in_Normal;

uniform mat4 NormMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

out struct Vertex
{
    vec4 vertex;
    vec2 texcoord;
    vec3 normal;
    vec3 viewDir;
} Vert;

void main(void)
{
    Vert.vertex     = vec4(in_Position, 1.0f);
    Vert.normal     = vec3(NormMatrix * vec4(in_Normal, 0.0f));
    //vec4 camera     = -ViewMatrix * vec4(0.0f, 0.0f, 0.0f, 1.0f);
    //Vert.viewDir    = camera.xyz - Vert.vertex.xyz;
    Vert.texcoord   = in_Texture;

    gl_Position     = ProjectionMatrix * ViewMatrix * Vert.vertex;
}
