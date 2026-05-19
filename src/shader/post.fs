#version 330 core
out vec4 FragColor;
in  vec2 TexCoords;

uniform sampler2D screenTexture;
uniform int       camMode;         // 0=FOLLOW 1=FIRST_PERSON 2=FREE
uniform vec2      resolution;

uniform int filterBlur;
uniform int filterVignette;
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
                      * kernel[i+1][j+1];
    return result;
}

void main()
{
    vec3 color = texture(screenTexture, TexCoords).rgb;

    if (bloomEnabled == 1)
    {
        vec3 bloom = texture(bloomTexture, TexCoords).rgb;
        color += bloom;
    }

    // --- Gaussian blur (G) ---
    if (filterBlur == 1)
    {
        mat3 gaussian = mat3(
            1.0/16.0, 2.0/16.0, 1.0/16.0,
            2.0/16.0, 4.0/16.0, 2.0/16.0,
            1.0/16.0, 2.0/16.0, 1.0/16.0
        );
        color = convolve(gaussian);
    }

    // --- Sharpening / Laplacian (F) ---
    if (filterSharpen == 1)
    {
        mat3 laplacian = mat3(
             0.0, -0.5,  0.0,
            -0.5,  3.0, -0.5,
             0.0, -0.5,  0.0
        );
        color = convolve(laplacian);
    }

    // --- Scanlines (J) ---
    if (filterScanline == 1)
    {
        float scanline = mod(gl_FragCoord.y, 3.0) < 1.0 ? 0.92 : 1.0;
        color *= scanline;
    }

    // --- Vignette (H) ---
    if (filterVignette == 1)
    {
        vec2  uv      = TexCoords - 0.5;
        float vignette = pow(clamp(1.0 - dot(uv * 1.8, uv * 1.8), 0.0, 1.0), 0.6);
        color *= vignette;
    }

    color = color / (color + vec3(1.0));
    //FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
    FragColor = vec4(color, 1.0);
}