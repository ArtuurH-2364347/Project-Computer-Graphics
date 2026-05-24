#version 330 core
#define NR_POINT_LIGHTS 32

struct PointLight {
    vec3  position;
    vec3  color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TextCoord;
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

uniform sampler2D  ourTexture;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform vec3       viewPos;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseTex)
{
    vec3  lightDir    = normalize(light.position - fragPos);
    float diff        = max(dot(normal, lightDir), 0.0);
    vec3  halfDir     = normalize(lightDir + viewDir);
    float spec        = pow(max(dot(normal, halfDir), 0.0), 32.0);
    float dist        = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant
                              + light.linear    * dist
                              + light.quadratic * dist * dist);

    vec3 diffuse  = diff * diffuseTex * light.color * light.intensity;
    vec3 specular = spec * vec3(0.3)  * light.color * light.intensity;

    return (diffuse + specular) * attenuation;
}

void main()
{
    vec3 norm       = normalize(Normal);
    vec3 viewDir    = normalize(viewPos - FragPos);
    vec3 diffuseTex = vec3(texture(ourTexture, TextCoord));

    const float AMBIENT_STRENGTH = 0.08;
    vec3 result = AMBIENT_STRENGTH * diffuseTex;

    for (int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, diffuseTex);

    FragColor = vec4(result, 1.0);

    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    BrightColor = brightness > 0.7 ? vec4(result, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);

}