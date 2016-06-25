#version 330

flat in vec4 interpColour;
out vec4 outputColor;

void main() {
    outputColor = interpColour;
}
