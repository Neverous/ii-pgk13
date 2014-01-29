#version 330 core

in vec2 UV;
in vec3 VertexPosition;
in vec3 Normal;

out vec3 color;

uniform sampler2D textureSampler;

uniform mat4    INV;

// material
uniform vec4    diffuse;
uniform vec4    ambient;
uniform vec4    specular;
uniform vec4    emissive;
uniform float   shininess;

struct lightSource
{
    vec4 position;
    vec4 diffuse;
    vec4 specular;
    float constantAttenuation, linearAttenuation, quadraticAttenuation;
    float spotCutoff, spotExponent;
    vec3 spotDirection;
};

const int numberOfLights = 2;
lightSource lights[numberOfLights];
lightSource light0 = lightSource(
    vec4(20.0,  20.0,  40.0, 1.0),
    vec4(1.0,  1.0,  1.0, 1.0),
    vec4(1.0,  1.0,  1.0, 1.0),
    0.0, 0.01, 0.0,
    180.0, 0.0,
    vec3(0.0, 0.0, 0.0));

lightSource light1 = lightSource(
    vec4(40.0, 40.0,  40.0, 1.0),
    vec4(2.0,  0.0,  0.0, 1.0),
    vec4(0.1,  0.1,  0.1, 1.0),
    0.0, 0.001, 0.0,
    80.0, 10.0,
    vec3(0.0, 1.0, 0.0));

vec4 scene_ambient = vec4(0.2, 0.2, 0.2, 1.0);

void main() {
    lights[0] = light0;
    lights[1] = light1;

    vec3 normalDirection    = normalize(Normal);
    vec3 viewDirection      = normalize(vec3(INV * vec4(0, 0, 0, 1.0)) - VertexPosition);
    vec3 lightDirection;
    float attenuation;

    vec3 totalLighting = vec3(scene_ambient) * vec3(ambient);
    color = vec3(0, 0, 0);

    for(int index = 0; index < numberOfLights; ++ index) // for all light sources
    {
        if(0.0 == lights[index].position.w) // directional light?
        {
            attenuation = 1.0; // no attenuation
            lightDirection = normalize(vec3(lights[index].position));
        }

        else // point light or spotlight (or other kind of light)
        {
            vec3 positionToLightSource = vec3(lights[index].position) - VertexPosition;
            float distance = length(positionToLightSource);
            lightDirection = normalize(positionToLightSource);
            attenuation = 1.0 / (lights[index].constantAttenuation
                    + lights[index].linearAttenuation * distance
                    + lights[index].quadraticAttenuation * distance * distance);

            if(lights[index].spotCutoff <= 90.0) // spotlight?
            {
                float clampedCosine = max(0.0, dot(-lightDirection, normalize(lights[index].spotDirection)));
                if (clampedCosine < cos(radians(lights[index].spotCutoff))) // outside of spotlight cone?
                    attenuation = 0.0;

                else
                    attenuation = attenuation * pow(clampedCosine, lights[index].spotExponent);
            }
        }

        vec3 diffuseReflection = attenuation
            * vec3(lights[index].diffuse) * vec3(diffuse)
            * max(0.0, dot(normalDirection, lightDirection));

        vec3 specularReflection;
        if(dot(normalDirection, lightDirection) < 0.0) // light source on the wrong side?
            specularReflection = vec3(0.0, 0.0, 0.0); // no specular reflection

        else // light source on the right side
        {
            specularReflection = attenuation * vec3(lights[index].specular) * vec3(specular)
                * pow(max(0.0, dot(reflect(-lightDirection, normalDirection), viewDirection)), shininess);
        }

        color += specularReflection;
        totalLighting += diffuseReflection;
    }

    color += totalLighting * texture2D(textureSampler, UV).rgb;
}
