#version 330
 
layout(triangles) in;
layout(triangle_strip, max_vertices=18) out;

flat in vec4 geom_colour[];
flat in vec4 geom_normal[];
flat out vec4 frag_colour;

const float line_width = 0.002;
const vec3 view = vec3(0, 0, -1);

void main()
{

  frag_colour = geom_colour[0];
  vec4 n = geom_normal[2];
  float seen = dot(view, n.xyz);
  
  if (seen > 0) {

    vec4 a = gl_in[0].gl_Position;
    vec4 b = gl_in[1].gl_Position;
    vec4 c = gl_in[2].gl_Position;
    
    vec3 ab = (b - a).xyz;
    vec3 bc = (c - b).xyz;
    vec3 ca = (a - c).xyz;
    
    vec3 xab = normalize(cross(ab, n.xyz));
    vec3 xbc = normalize(cross(bc, n.xyz));
    vec3 xca = normalize(cross(ca, n.xyz));
  
    vec4 d = vec4(xab * line_width / length(cross(view, xab)), 0);
    gl_Position = a + a.w * d; EmitVertex();
    gl_Position = b + b.w * d; EmitVertex();
    gl_Position = a - a.w * d; EmitVertex();
    EndPrimitive();
    gl_Position = b + b.w * d; EmitVertex();
    gl_Position = b - b.w * d; EmitVertex();
    gl_Position = a - a.w * d; EmitVertex();
    EndPrimitive();

    d = vec4(xbc * line_width / length(cross(view, xbc)), 0);
    gl_Position = b + b.w * d; EmitVertex();
    gl_Position = c + c.w * d; EmitVertex();
    gl_Position = b - b.w * d; EmitVertex();
    EndPrimitive();
    gl_Position = c + c.w * d; EmitVertex();
    gl_Position = c - c.w * d; EmitVertex();
    gl_Position = b - b.w * d; EmitVertex();
    EndPrimitive();

    d = vec4(xca * line_width / length(cross(view, xca)), 0);
    gl_Position = c + c.w * d; EmitVertex();
    gl_Position = a + a.w * d; EmitVertex();
    gl_Position = c - c.w * d; EmitVertex();
    EndPrimitive();
    gl_Position = a + a.w * d; EmitVertex();
    gl_Position = a - a.w * d; EmitVertex();
    gl_Position = c - c.w * d; EmitVertex();
    EndPrimitive();

   }
}
