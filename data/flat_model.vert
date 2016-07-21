#version 330

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(std140) uniform geometry {
  vec3 colour;
  vec4 camera_light_pos;
  mat4 model_to_camera;
  mat4 model_to_camera_n;
  mat4 camera_to_clip;
};

flat out vec4 interpColour;

void main() {
  vec4 camera_position = model_to_camera * position;
  gl_Position = camera_to_clip * camera_position;
  interpColour = vec4(colour, 1);
}
