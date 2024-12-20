#version 330 core

in VertexData {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoord;
    vec4 fragPosLightSpace;
} fsIn;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;
uniform vec3 viewPos;
uniform sampler2D shadowMap;
uniform bool enableShadows;

out vec4 FragColor;

float mapShadow(vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fsIn.fragPosLightSpace.xyz / fsIn.fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;
    float shadowBias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    float shadow = 0.0;

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - shadowBias > pcfDepth ? 1.0 : 0.0;
        }
    }

    return shadow /= 9.0;
}

void main()
{
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fsIn.texCoord));

    vec3 norm = normalize(fsIn.normal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fsIn.texCoord));

    vec3 viewDir = normalize(viewPos - fsIn.fragPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, fsIn.texCoord));

    float shadow = enableShadows ? mapShadow(norm, lightDir) : 0.0;

    vec3 phong = ambient + (1.0 - shadow) * (diffuse + specular);
    FragColor = vec4(phong, 1.0);
}
