#version 330 core

layout (location = 0) in vec3 position_modelspace;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal_modelspace;

out vec2 UV;
out vec3 Position_worldspace;
out vec3 Normal_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;

uniform mat4 mvp;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform vec3 lightPosition_worldspace;

void main() {
    gl_Position = mvp * vec4(position_modelspace, 1.0f);

    Position_worldspace = (modelMatrix * vec4(position_modelspace, 1.0f)).xyz;

    vec3 position_modelspaceCamera = (viewMatrix * modelMatrix * vec4(position_modelspace, 1.0f)).xyz;
    EyeDirection_cameraspace = vec3(0, 0, 0) - position_modelspaceCamera;

    vec3 lightPosition_worldspaceCamera = (viewMatrix * vec4(lightPosition_worldspace, 1.0f)).xyz;
    LightDirection_cameraspace = lightPosition_worldspaceCamera + EyeDirection_cameraspace;

    Normal_cameraspace = (viewMatrix * modelMatrix * vec4(normal_modelspace, 0.0f)).xyz;
    UV = uv;
}
