#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

out VertexData {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoord;
    vec4 fragPosLightSpace;
} vsOut;

void main()
{
    vec4 worldPos = model * vec4(inPosition, 1.0);
    vsOut.fragPos = vec3(worldPos);
    vsOut.normal = mat3(transpose(inverse(model))) * inNormal;
    vsOut.texCoord = inTexCoord;
    vsOut.fragPosLightSpace = lightSpaceMatrix * vec4(vsOut.fragPos, 1.0);
    gl_Position = projection * view * worldPos;
}
