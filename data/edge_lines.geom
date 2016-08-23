#version 330
 
layout(triangles) in;
flat in vec4 geom_colour[];
flat in vec4 geom_normal[];

layout(std140) uniform geometry {
  vec3 model_colour;
  mat4 model_to_camera;
  mat4 model_to_camera_n;
  mat4 camera_to_clip;
  mat4 camera_to_clip_n;
  float line_width;
};

layout(triangle_strip, max_vertices=21) out;
flat out vec4 frag_colour;

const vec3 view = vec3(0, 0, 1);


void triangle(in vec4 a, in vec4 b, in vec4 c) {
    gl_Position = a; EmitVertex();
    gl_Position = b; EmitVertex();
    gl_Position = c; EmitVertex();
    EndPrimitive();
}

void inset(in vec4 corner, in vec3 edge1norm, in vec4 margin1, in vec3 edge2norm, in vec4 margin2, 
           out vec4 inset) {
    float sine = length(cross(edge1norm, edge2norm));
    vec3 step1 = edge2norm * length(margin1) / sine;
    vec3 step2 = edge1norm * length(margin2) / sine;
    inset = camera_to_clip * (corner + vec4(step2 - step1, 0));
}

void main()
{

    vec4 n = geom_normal[2];
    vec4 a = gl_in[0].gl_Position;
    vec4 b = gl_in[1].gl_Position;
    vec4 c = gl_in[2].gl_Position;    
    
    vec3 ab = normalize((b - a).xyz);
    vec3 bc = normalize((c - b).xyz);
    vec3 ca = normalize((a - c).xyz);
        
    vec4 xab = vec4(cross(ab, n.xyz), 0);
    vec4 xbc = vec4(cross(bc, n.xyz), 0);
    vec4 xca = vec4(cross(ca, n.xyz), 0);

    vec3 o = ((a + b + c)/3).xyz;
    float k = 0.001 * length(o);
    o = normalize(o);

    float kab = length(cross(xab.xyz, o));
    float kbc = length(cross(xbc.xyz, o));
    float kca = length(cross(xca.xyz, o));
    
    if (kab > 0.1 && kbc > 0.1 && kca > 0.1) {

        xab = xab * k / kab; 
        xbc = xbc * k / kbc; 
        xca = xca * k / kca;

        frag_colour = vec4(model_colour, 1);

        vec4 a0 = camera_to_clip * (a + xab);
        vec4 a1 = camera_to_clip * (a - xab);
        vec4 b0 = camera_to_clip * (b + xab);
        vec4 b1 = camera_to_clip * (b - xab);
        triangle(a0, a1, b0); triangle(b0, a1, b1);
        
        b0 = camera_to_clip * (b + xbc);
        b1 = camera_to_clip * (b - xbc);
        vec4 c0 = camera_to_clip * (c + xbc);
        vec4 c1 = camera_to_clip * (c - xbc);
        triangle(b0, b1, c0); triangle(c0, b1, c1);
        
        c0 = camera_to_clip * (c + xca);
        c1 = camera_to_clip * (c - xca);
        a0 = camera_to_clip * (a + xca);
        a1 = camera_to_clip * (a - xca);
        triangle(c0, c1, a0); triangle(a0, c1, a1);
    
    
        frag_colour = vec4(0, 0, 0, 1);
   
        inset(a, ab, xab, ca, xca, a0);
        inset(b, bc, xbc, ab, xab, b0);
        inset(c, ca, xca, bc, xbc, c0);
        triangle(a0, b0, c0);
        
    } else {
    
        frag_colour = vec4(0, 0, 0, 1);
        triangle(camera_to_clip * a, camera_to_clip * b, camera_to_clip * c);
        
    }

}
