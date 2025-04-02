#version 410 core

out vec4 FragColor;

in GS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform bool torus;

void main()
{
    vec4 texColor = texture(texture_diffuse1, fs_in.TexCoords);

    FragColor = texColor;
}