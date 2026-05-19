#version 330 core
out vec4 FragColor;
in  vec2 TexCoords;

uniform sampler2D screenTexture;
uniform int       camMode;         // 0=FOLLOW 1=FIRST_PERSON 2=FREE
uniform vec2      resolution;

uniform int filterBlur;
uniform int filterSharpen;
uniform int filterScanline;
uniform int       bloomEnabled;
uniform sampler2D bloomTexture;

vec3 convolve(mat3 kernel)
{
    vec2 texOffset = 1.0 / resolution;
    vec3 result = vec3(0.0);
    for (int i = -1; i <= 1; i++)
        for (int j = -1; j <= 1; j++)
            result += texture(screenTexture, TexCoords + vec2(float(j), float(i)) * texOffset).rgb
                      * kernel[j+1][i+1];
    return result;
}

vec3 strongBlur()
{
    vec2 texOffset = (1.0 / resolution) * 2.0;
    vec3 result = vec3(0.0);
    float weights[5] = float[](0.227027, 0.194595, 0.121622, 0.054054, 0.016216);
    
    result += texture(screenTexture, TexCoords).rgb * weights[0];
    for (int i = 1; i < 5; i++)
    {
        result += texture(screenTexture, TexCoords + vec2(texOffset.x * float(i), 0.0)).rgb * weights[i];
        result += texture(screenTexture, TexCoords - vec2(texOffset.x * float(i), 0.0)).rgb * weights[i];
        result += texture(screenTexture, TexCoords + vec2(0.0, texOffset.y * float(i))).rgb * weights[i];
        result += texture(screenTexture, TexCoords - vec2(0.0, texOffset.y * float(i))).rgb * weights[i];
    }
    return result / 2.0;
}

void main()
{
    vec3 color;

    if (filterBlur == 1)
    {
        color = strongBlur();
    }
    else if (filterSharpen == 1)
    {
        mat3 laplacian = mat3(
            0.0, -1.0,  0.0,
            -1.0,  5.0, -1.0,
            0.0, -1.0,  0.0
        );

        vec3 original = texture(screenTexture, TexCoords).rgb;
        
        vec3 sharpened = convolve(laplacian);
        float lumaOrig    = dot(original,  vec3(0.299, 0.587, 0.114));
        float lumaSharp   = dot(sharpened, vec3(0.299, 0.587, 0.114));
        
        float scale = lumaSharp / max(lumaOrig, 0.001);
        scale = clamp(scale, 0.0, 3.0);
        color = original * scale;
    }
    else
    {
        color = texture(screenTexture, TexCoords).rgb;
    }

    if (bloomEnabled == 1)
    {
        vec3 bloom = texture(bloomTexture, TexCoords).rgb;
        color += bloom;
    }

    if (filterScanline == 1)
    {
        float scanline = mod(gl_FragCoord.y, 3.0) < 1.0 ? 0.92 : 1.0;
        color *= scanline;
    }

    // tonemapping last
    color = color / (color + vec3(1.0));
    FragColor = vec4(color, 1.0);
}