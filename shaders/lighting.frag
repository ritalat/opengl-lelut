#version 330 core

in VertexData {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoord;
} fsIn;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight {
    vec4 direction;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

#define MAX_POINT_LIGHTS 10

struct PointLight {
    vec4 position;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec4 position;
    vec4 direction;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float constant;
    float linear;
    float quadratic;

    float cutoff;
    float outercutoff;
};

layout (std140) uniform Lights {
    SpotLight spotLight;
    DirLight dirLight;
    PointLight pointLights[MAX_POINT_LIGHTS];
};

uniform vec3 viewPos;
uniform Material material;
uniform bool enableSpotLight;
uniform int numPointLights;

out vec4 FragColor;

vec3 calcDirectionalLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 ambient = light.ambient.rgb * vec3(texture(material.diffuse, fsIn.texCoord));

    vec3 lightDir = normalize(-light.direction.xyz);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse.rgb * diff * vec3(texture(material.diffuse, fsIn.texCoord));

    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), material.shininess);
    vec3 specular = light.specular.rgb * spec * vec3(texture(material.specular, fsIn.texCoord));

    return (ambient + diffuse + specular);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    float dist = distance(light.position.xyz, fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));

    vec3 ambient = light.ambient.rgb * vec3(texture(material.diffuse, fsIn.texCoord));
    ambient *= attenuation;

    vec3 lightDir = normalize(light.position.xyz - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse.rgb * diff * vec3(texture(material.diffuse, fsIn.texCoord));
    diffuse *= attenuation;

    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), material.shininess);
    vec3 specular = light.specular.rgb * spec * vec3(texture(material.specular, fsIn.texCoord));
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    float dist = distance(light.position.xyz, fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));

    vec3 lightDir = normalize(light.position.xyz - fragPos);
    float theta = dot(lightDir, normalize(-light.direction.xyz));
    float epsilon = light.cutoff - light.outercutoff;
    float intensity = clamp((theta - light.outercutoff) / epsilon, 0.0, 1.0);

    vec3 ambient = light.ambient.rgb * vec3(texture(material.diffuse, fsIn.texCoord));
    ambient *= (attenuation * intensity);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse.rgb * diff * vec3(texture(material.diffuse, fsIn.texCoord));
    diffuse *= (attenuation * intensity);

    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), material.shininess);
    vec3 specular = light.specular.rgb * spec * vec3(texture(material.specular, fsIn.texCoord));
    specular *= (attenuation * intensity);

    return (ambient + diffuse + specular);
}

void main()
{
    vec3 result = vec3(0.0);
    vec3 norm = normalize(fsIn.normal);
    vec3 viewDir = normalize(viewPos - fsIn.fragPos);

    result += calcDirectionalLight(dirLight, norm, viewDir);

    int lights = numPointLights < MAX_POINT_LIGHTS ? numPointLights : MAX_POINT_LIGHTS;
    for (int i = 0; i < lights; ++i) {
        result += calcPointLight(pointLights[i], norm, fsIn.fragPos, viewDir);
    }

    if (enableSpotLight)
        result += calcSpotLight(spotLight, norm, fsIn.fragPos, viewDir);

    FragColor = vec4(result, 1.0);
}
