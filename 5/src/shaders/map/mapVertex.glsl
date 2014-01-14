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

    gl_Position = MVP * vertex;
    gl_Position.z = vertex.z == 32768 ? -1000.0 : 0.0;

    vertex.z -= 500.0;
    if(vertex.z < 0)
        fragmentColor = vec3(0.0, 0.0, 1.0);

    else if(vertex.z < 500)
        fragmentColor = vec3(0.0, vertex.z / 500.0,    0.0);

    else if(vertex.z < 1000)
        fragmentColor = vec3(vertex.z / 500.0 - 1.0, 1.0, 0.0);

    else if(vertex.z < 1500)
        fragmentColor = vec3(1.0, 2.0 - vertex.z / 500.0, 0.0);

    else
        fragmentColor = vec3(1.0, 1.0, 1.0);
}
