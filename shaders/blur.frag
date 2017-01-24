R"(
#version 330 core

// Gaussian kernel
uniform float WEIGHT[5] = {0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162};
uniform int MAX_WEIGHT = 5;
// uniform float WEIGHT[3] = {0.375, 0.25, 0.0625};
// uniform int MAX_WEIGHT = 3;

in vec2 nowTexCoord;
out vec4 fragColor;
uniform sampler2D image;
uniform bool horizontal;

void main()
{
     vec2 texOffset = 1.0 / textureSize(image, 0);  // Size of a texel
     vec3 res = texture(image, nowTexCoord).rgb * WEIGHT[0];
     if (horizontal)
     {
         for (int i = 1; i < MAX_WEIGHT; ++i)
         {
            res += texture(image, nowTexCoord + vec2(texOffset.x * i * 2.0, 0.0)).rgb * WEIGHT[i];
            res += texture(image, nowTexCoord - vec2(texOffset.x * i * 2.0, 0.0)).rgb * WEIGHT[i];
         }
     }
     else
     {
         for (int i = 1; i < MAX_WEIGHT; ++i)
         {
             res += texture(image, nowTexCoord + vec2(0.0, texOffset.y * i * 2.0)).rgb * WEIGHT[i];
             res += texture(image, nowTexCoord - vec2(0.0, texOffset.y * i * 2.0)).rgb * WEIGHT[i];
         }
     }
     fragColor = vec4(res, 1.0);
}
)"
