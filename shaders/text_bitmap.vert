#version 330 core
layout (location = 0) in vec4 vert;

out vec2 TexCoord;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vert.xy, 0.0, 1.0);
    TexCoord = vert.zw;
}
