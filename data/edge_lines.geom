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

void main()
{

    vec4 n = geom_normal[2];
    vec4 a = gl_in[0].gl_Position;
    vec4 b = gl_in[1].gl_Position;
    vec4 c = gl_in[2].gl_Position;    
    
    vec3 ab = normalize((b - a).xyz);
    vec3 bc = normalize((c - b).xyz);
    vec3 ca = normalize((a - c).xyz);
        
    vec3 xab = cross(ab, n.xyz);
    vec3 xbc = cross(bc, n.xyz);
    vec3 xca = cross(ca, n.xyz);

    vec3 o = ((a + b + c)/3).xyz;
    float k = 0.003 * length(o);
    o = normalize(o);

    float kab = length(cross(xab, o));
    float kbc = length(cross(xbc, o));
    float kca = length(cross(xca, o));
    
    if (kab > 0.1 && kbc > 0.1 && kca > 0.1) {

        xab = xab * k / kab; 
        xbc = xbc * k / kbc; 
        xca = xca * k / kca;
        
        // signs are a mess here
        vec4 da = vec4(dot(xab, ca) * ca + dot(xca, ab) * ab, 0);
        vec4 db = vec4(dot(xbc, ab) * ab + dot(xab, bc) * bc, 0);
        vec4 dc = vec4(dot(xca, bc) * bc + dot(xbc, ca) * ca, 0);
        
        vec4 a0 = camera_to_clip * (a + da);
        vec4 a1 = camera_to_clip * (a - da);
        vec4 b0 = camera_to_clip * (b + db);
        vec4 b1 = camera_to_clip * (b - db);
        vec4 c0 = camera_to_clip * (c + dc);
        vec4 c1 = camera_to_clip * (c - dc);
    
        frag_colour = vec4(model_colour, 1);
        triangle(a0, a1, b0); triangle(b0, a1, b1);
        triangle(b0, b1, c0); triangle(c0, b1, c1);
        triangle(c0, c1, a0); triangle(a0, c1, a1);
    
        frag_colour = vec4(0, 0, 0, 1);
        triangle(a0, b0, c0);
        
    } else {
    
        frag_colour = vec4(0, 0, 0, 1);
        triangle(camera_to_clip * a, camera_to_clip * b, camera_to_clip * c);
        
    }

}
