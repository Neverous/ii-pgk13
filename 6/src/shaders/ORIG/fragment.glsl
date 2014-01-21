#version 330 core

in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;

out vec3 color;

uniform sampler2D textureSampler;
uniform mat4 modelViewMatrix;
uniform vec3 lightPosition_worldspace;

void main() {
    vec3 lightColor = vec3(1, 1, 1);
    float lightPower = 50.0f;

    vec3 materialDiffuseColor = texture2D(textureSampler, UV).rgb;
    vec3 materialAmbientColor = vec3(0.1, 0.1, 0.1) * materialDiffuseColor;
    vec3 materialSpecularColor = vec3(0.3, 0.3, 0.3);

    float distance = length(lightPosition_worldspace - Position_worldspace);

    vec3 n = normalize(Normal_cameraspace);
    vec3 l = normalize(LightDirection_cameraspace);

    float cosTheta = clamp(dot(n, l), 0, 1);

    vec3 eye = normalize(EyeDirection_cameraspace);
    vec3 reflection = reflect(-l, n);

    float cosAlpha = clamp(dot(eye, reflection), 0, 1);

    color = materialAmbientColor +
        materialDiffuseColor * lightColor * lightPower * cosTheta / (distance * distance) +
        materialSpecularColor * lightColor * lightPower * pow(cosAlpha, 5) / (distance * distance);
}
