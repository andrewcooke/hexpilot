#version 330 core

in vec2 tex_coords;

out vec4 colour;

uniform sampler2D screenTexture;

void main()
{ 
    colour = vec4(vec3(1 - texture(screenTexture, tex_coords)), 1);
}
