#version 330 core

in vec2 tex_coords;

out vec4 colour;

uniform sampler2D frame1;
uniform sampler2D frame2;

void main()
{
    vec4 c1 = texture(frame1, tex_coords);
    vec4 c2 = texture(frame2, tex_coords);
    colour = vec4(max(c1.x, c2.x), max(c1.y, c2.y), max(c1.z, c2.z), 1);
}
