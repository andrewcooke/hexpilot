#version 330 core

layout (location = 0) in vec2 position;

out vec2 tex_coords;

void main()
{
    gl_Position = vec4(position.x, position.y, 0.0f, 1.0f); 
    tex_coords = (position + 1.0f) / 2.0f;
}
