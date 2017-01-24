R"(
#version 330 core

uniform vec3 color;
layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 brightColor;

void main()
{
    fragColor = vec4(color, 1.0);
}
)"
