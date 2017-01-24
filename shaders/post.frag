R"(
#version 330 core

uniform float EXPOSURE = 1.0;
uniform float INV_GAMMA = 0.454545;

in vec2 nowTexCoord;
out vec4 fragColor;
uniform sampler2D image;
uniform sampler2D blur;

void main()
{
    vec3 res = texture(image, nowTexCoord).rgb + texture(blur, nowTexCoord).rgb;
    res = vec3(1.0) - exp(-res * EXPOSURE);  // HDR correction
    res = pow(res, vec3(1.0 * INV_GAMMA));  // Gamma correction
    fragColor = vec4(res, 1.0);
}
)"
