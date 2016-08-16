#version 330

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;

layout(std140) uniform geometry {
  vec3 model_colour;
  mat4 model_to_camera;
  mat4 model_to_camera_n;
  mat4 camera_to_clip;
  mat4 camera_to_clip_n;
};

flat out vec4 geom_colour;
flat out vec4 geom_normal;

void main() {
  vec4 camera_position = model_to_camera * position;
  vec4 camera_normal = vec4(normalize((model_to_camera_n * normal).xyz), 0);
  gl_Position = camera_to_clip * camera_position;
  geom_colour = vec4(model_colour, 1);
  geom_normal = vec4(normalize((camera_to_clip_n * camera_normal).xyz), 0);
}
