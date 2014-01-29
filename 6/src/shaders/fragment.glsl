#version 330 core

in struct Vertex
{
    vec4 vertex;
    vec2 texcoord;
    vec3 normal;
    vec3 viewDir;
} Vert;

out vec4 out_Color;

struct PointLight
{
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec3 attenuation;
} light[2];

uniform struct Material
{
    sampler2D   colorTexture;
    vec4        ambient;
    vec4        diffuse;
    vec4        specular;
    vec4        emissive;
    float       shininess;
} material;

uniform float lights;
uniform float textures;

void main(void)
{
    vec4 color = texture(material.colorTexture, Vert.texcoord);
    if(textures == 0.0f)
        color = texture(material.colorTexture, vec2(0.0f));

    if(lights > 0.0f)
    {
        light[0].position       = vec4(-600.0f, -100.0f, 100.0f, 1.0f);   // position
        light[0].ambient        = vec4(0.1f, 0.1f, 0.1f, 1.0f);   // ambient
        light[0].diffuse        = vec4(1.0f, 1.0f, 1.0f, 1.0f);   // diffuse
        light[0].specular       = vec4(1.0f, 1.0f, 1.0f, 1.0f);   // specular
        light[0].attenuation    = vec3(0.000001f, 0.0f, 0.00001f);       // attenuation

        light[1].position       = vec4(200.0f, 200.0f, 100.0f, 1.0f);   // position
        light[1].ambient        = vec4(0.1f, 0.1f, 0.1f, 1.0f);   // ambient
        light[1].diffuse        = vec4(1.0f, 1.0f, 1.0f, 1.0f);   // diffuse
        light[1].specular       = vec4(1.0f, 1.0f, 1.0f, 1.0f);   // specular
        light[1].attenuation    = vec3(0.000001f, 0.0f, 0.00001f);       // attenuation

        float attenuation = 0.0f;
        out_Color = vec4(0.0f);

        for(int l = 0; l < 2; ++ l)
        {
            float dist      = distance(light[l].position, Vert.vertex);
            attenuation     += 1.0f / (light[l].attenuation[0] + light[l].attenuation[1] * dist + light[l].attenuation[2] * dist * dist);
            vec3 normal     = normalize(Vert.normal);
            vec3 lightDir   = normalize(light[l].position.xyz - Vert.vertex.xyz);
            vec3 viewDir    = normalize(Vert.viewDir);

            out_Color       = material.emissive;
            out_Color       += material.ambient * light[l].ambient;
            float NdotL     = max(dot(normal, lightDir), 0.0f);
            out_Color       += material.diffuse * light[l].diffuse * NdotL;
            float RdotVpow  = max(pow(dot(reflect(-lightDir, normal), viewDir), material.shininess), 0.0f);
            out_Color       += material.specular * light[l].specular * RdotVpow;
        }

        out_Color       *= color * attenuation;
    }
}
