#version 410 core
layout(points) in;
layout(triangle_strip, max_vertices = 512) out;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} gs_in[];

out GS_OUT {
    vec2 TexCoords;
} gs_out;

uniform bool torus;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const float PI = 3.1415926535897932384626433832795;

uniform int segments;
uniform int rings;

void generateTorus()
{
    float R = 1.0; // Major radius
    float r = 0.3; // Minor radius

    mat4 mvp = projection * view * model;

    for(int i = 0; i < segments; i++)
    {
        float u = float(i) * 2.0 * PI / float(segments);
        float u_next = float((i + 1) % segments) * 2.0 * PI / float(segments);

        for(int j = 0; j <= rings; j++)
        {
            float v = float(j) * 2.0 * PI / float(rings);

            // Calculate vertices for current ring segment
            vec3 pos = vec3(
                (R + r * cos(v)) * cos(u),
                r * sin(v),
                (R + r * cos(v)) * sin(u)
            );

            vec3 pos_u = vec3(
                (R + r * cos(v)) * cos(u_next),
                r * sin(v),
                (R + r * cos(v)) * sin(u_next)
            );

            // Emit vertices
            gl_Position = mvp * vec4(pos, 1.0);
            gs_out.TexCoords = vec2(float(i) / float(segments), float(j) / float(rings));
            EmitVertex();

            // Second vertex
            gl_Position = mvp * vec4(pos_u, 1.0);
            gs_out.TexCoords = vec2(float(i + 1) / float(segments), float(j) / float(rings));
            EmitVertex();
        }
        EndPrimitive();
    }
}

void main()
{
    generateTorus();
}