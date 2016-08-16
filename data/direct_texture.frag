#version 330 core

in vec2 tex_coords;

uniform sampler2D frame;

out vec4 colour;

void main()
{ 
    colour = texture(frame, tex_coords);
}
