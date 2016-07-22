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
  vec4 fudge = vec4(camera_position.x, camera_position.y, camera_position.z - 0.001, camera_position.w);
  gl_Position = camera_to_clip * fudge;
  frag_colour = vec4(0, 0, 0, 1);
}
