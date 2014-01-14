#version 330 core

uniform vec4 box;
uniform mat4 MVP;

layout(location = 0) in vec3 vertexPosition;

out vec3 fragmentColor;

void main()
{
    vec4 vertex = vec4(vertexPosition, 1);
    vertex.x = box.x + (box.y - box.x) * vertex.x / 2048.0;
    vertex.y = box.z + (box.w - box.z) * vertex.y / 2048.0;
    vertex.z -= 900.0;
    gl_Position = MVP * vertex;
    gl_Position.z = 0.0;

    if(vertex.z < 0)
        fragmentColor = vec3(0x34 / 255.0, 0x65 / 255.0, 0xA4 / 255.0);

    else if(vertex.z < 500)
        fragmentColor = vec3(0x73 / 255.0, vertex.z / 500.0 * 0xD2 / 255.0, 0x16 / 255.0);

    else if(vertex.z < 1000)
        fragmentColor = vec3(vertex.z / 500.0 - 0xED / 255.0, 0xD4 / 255.0, 0x00 / 255.0);

    else if(vertex.z < 1500)
        fragmentColor = vec3(0xCC / 255.0, 2. - vertex.z / 500.0, 0x00 / 255.0);

    else
        fragmentColor = vec3(0xD3 / 255.0, 0xD7 / 255.0, 0xCF / 255.0);
}
