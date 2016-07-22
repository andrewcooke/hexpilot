#version 330
 
layout(triangles) in;
layout(triangle_strip, max_vertices=6) out;

flat in vec4 geom_colour[];
flat in vec4 geom_normal[];
flat out vec4 frag_colour;
 
void main()
{
  frag_colour = geom_colour[0];
  float seen = dot(vec4(0, 0, 1, 0), geom_normal[2]);
  vec4 a = gl_in[0].gl_Position;
  vec4 b = gl_in[1].gl_Position;
  vec4 c = gl_in[2].gl_Position;
  gl_Position = a;
  EmitVertex();
  gl_Position = b;
  EmitVertex();
  gl_Position = c;
  EmitVertex();
  EndPrimitive();
  gl_Position = a+0.2*geom_normal[2];
  EmitVertex();
  gl_Position = b+0.2*geom_normal[2];
  EmitVertex();
  gl_Position = c+0.2*geom_normal[2];
  EmitVertex();
  EndPrimitive();
}
