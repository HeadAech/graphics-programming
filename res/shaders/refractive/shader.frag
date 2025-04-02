#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

// IoRs for red, green, and blue channels
uniform vec3 iorRGB;
uniform float aberrationStrength;
uniform sampler2D screenTexture;

void main()
{
    vec3 I = normalize(Position - cameraPos);
    vec3 N = normalize(Normal);

    vec3 R_red = refract(I + aberrationStrength * vec3(0.01, 0.0, 0.0), N, 1.0 / iorRGB.r);
    vec3 R_green = refract(I, N, 1.0 / iorRGB.g);
    vec3 R_blue = refract(I - aberrationStrength * vec3(0.01, 0.0, 0.0), N, 1.0 / iorRGB.b);

    float r = texture(skybox, R_red).r;
    float g = texture(skybox, R_green).g;
    float b = texture(skybox, R_blue).b;

    vec3 color = vec3(r, g, b);

    FragColor = vec4(color, 1.0);
}