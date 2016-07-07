#version 330 core

in vec2 tex_coords;

out vec4 colour;

uniform sampler2D frame;

void main()
{ 
    colour = vec4(vec3(1 - texture(frame, tex_coords)), 1);
//    colour = texture(frame, tex_coords);
}
