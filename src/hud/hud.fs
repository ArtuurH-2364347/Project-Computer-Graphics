#version 330 core
out vec4 FragColor;
  
in vec2 TexCoord;

uniform sampler2D hudTexture;

float alphaValue = 6.0f;

void main()
{
    vec4 texColor = vec4(vec3(texture(hudTexture, TexCoord)), alphaValue);
    FragColor = texColor;
}

