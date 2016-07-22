#version 330

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(std140) uniform geometry {
  vec3 model_colour;
  vec4 camera_light_pos;
  mat4 model_to_camera;
  mat4 model_to_camera_n;
  mat4 camera_to_clip;
};

flat out vec4 frag_colour;

void main() {
  vec4 camera_position = model_to_camera * position;
  vec4 camera_normal = vec4(normalize((model_to_camera_n * normal).xyz), 0);
  float brightness_1 = clamp(dot(camera_normal, camera_light_pos), 0, 1);
  float brightness_2 = clamp(dot(camera_normal, vec4(0,0,1,0)), 0, 1);
  float brightness = 0.6 * brightness_1 + 0.1 * brightness_2 + 0.2;
  frag_colour = vec4(brightness * model_colour, 1.0);
  gl_Position = camera_to_clip * camera_position;
}
