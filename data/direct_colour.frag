#version 330

flat in vec4 frag_colour;
out vec4 colour;

void main() {
    colour = frag_colour;
}
