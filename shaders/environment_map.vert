#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 normalMat;

out vec3 FragPos;
out vec3 Normal;

void main()
{
    vec4 worldPos = model * vec4(inPosition, 1.0);
    FragPos = vec3(worldPos);
    Normal = mat3(normalMat) * inNormal;
    gl_Position = projection * view * worldPos;
}
