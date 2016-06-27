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
  vec4 c_position = model_to_camera * position;
  vec4 c_normal = vec4(normalize((model_to_camera_n * normal).xyz), 0);
  float brightness_1 = clamp(dot(c_normal, camera_light_pos), 0, 1);
  float brightness_2 = clamp(dot(c_normal, vec4(0,0,1,0)), 0, 1);
  float brightness = 0.6 * brightness_1 + 0.1 * brightness_2 + 0.2;
  interpColour = vec4(brightness * colour, 1.0);
  gl_Position = camera_to_clip * c_position;
}
