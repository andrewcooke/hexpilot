#version 330
 
layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

flat in vec4 geom_colour[];
flat out vec4 frag_colour;
 
void main()
{
  for(int i = 0; i < gl_in.length(); i++) {
    gl_Position = gl_in[i].gl_Position;
    frag_colour = geom_colour[i];
    EmitVertex();
  }
}
