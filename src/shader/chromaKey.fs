#version 330 core
out vec4 FragColor;
  
in vec2 TexCoord;

uniform sampler2D CKTexture;

float alphaValue = 6.0f;

void main()
{
    vec4 texColor = texture(CKTexture, TexCoord);
    if (texColor.g > 0.8f && texColor.r < 0.9f && texColor.b < 0.9f) {discard;}
    FragColor = texColor;
}
