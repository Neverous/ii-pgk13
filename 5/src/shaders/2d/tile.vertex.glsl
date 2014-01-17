#version 120
#extension GL_EXT_gpu_shader4: enable

uniform vec4 box;
uniform mat4 MVP;

attribute float height;

varying vec3 fragmentColor;

void main()
{
    int y = gl_VertexID / 1025;
    int x = gl_VertexID - y * 1025;
    vec4 vertex = vec4(x, y, height, 1);
    vertex.x = box.x + (box.y - box.x) * vertex.x / 1024.0;
    vertex.y = box.z + (box.w - box.z) * vertex.y / 1024.0;

    gl_Position = MVP * vertex;
    if(vertex.z == 32768.0)
        gl_Position.z = -1000.0;

    vertex.z -= 1000.0;
    if(vertex.z < 0.0)
        fragmentColor = vec3(0.0, 0.0, 1.0);

    else if(vertex.z < 500.0)
        fragmentColor = vec3(0.0, vertex.z / 500.0,    0.0);

    else if(vertex.z < 1000.0)
        fragmentColor = vec3(vertex.z / 500.0 - 1.0, 1.0, 0.0);

    else if(vertex.z < 1500.0)
        fragmentColor = vec3(1.0, 2.0 - vertex.z / 500.0, 0.0);

    else
        fragmentColor = vec3(1.0, 1.0, 1.0);
}
