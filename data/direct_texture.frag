#version 330 core

in vec2 tex_coords;

out vec4 colour;

uniform sampler2D frame;

void main()
{ 
    colour = texture(frame, tex_coords);
}
