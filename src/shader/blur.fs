#version 330 core
out vec4 FragColor;
in  vec2 TexCoords;

uniform sampler2D image;
uniform bool horizontal;

float weight[5] = float[](0.227027, 0.194595, 0.121622, 0.054054, 0.016216);

void main()
{
    vec2 tex_offset = 1.0 / textureSize(image, 0);
    vec3 result = texture(image, TexCoords).rgb * weight[0];
    for (int i = 1; i < 5; i++)
    {
        vec2 offset = horizontal ? vec2(tex_offset.x * i, 0.0)
                                 : vec2(0.0, tex_offset.y * i);
        result += texture(image, TexCoords + offset).rgb * weight[i];
        result += texture(image, TexCoords - offset).rgb * weight[i];
    }
    FragColor = vec4(result, 1.0);
}