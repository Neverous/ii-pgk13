#version 330 core

uniform mat4 MVP;

layout(location = 0) in vec3 vertexPosition;

out vec3 fragmentColor;

void main()
{
    vec4 vertex = vec4(vertexPosition, 1);
    gl_Position = MVP * vertex;

    if(vertex.z < 0)
        fragmentColor = vec3(0x34 / 255.0f, 0x65 / 255.0f, 0xA4 / 255.0f);

    else if(vertex.z < 500)
        fragmentColor = vec3(0x73 / 255.0f, vertex.z / 500.0f * 0xD2 / 255.0f, 0x16 / 255.0f);

    else if(vertex.z < 1000)
        fragmentColor = vec3(vertex.z / 500.0f - 0xED / 255.0f, 0xD4 / 255.0f, 0x00 / 255.0f);

    else if(vertex.z < 1500)
        fragmentColor = vec3(0xCC / 255.0f, 2. - vertex.z / 500.0f, 0x00 / 255.0f);

    else
        fragmentColor = vec3(0xD3 / 255.0f, 0xD7 / 255.0f, 0xCF / 255.0f);
}
