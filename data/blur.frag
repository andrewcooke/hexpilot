#version 330 core

in vec2 tex_coords;

uniform sampler2D frame;
uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

out vec4 colour;

const float decay = 0.98;

void main()
{             
    vec2 tex_offset = 1.0 / textureSize(frame, 0);
    vec3 result = texture(frame, tex_coords).rgb * decay * weight[0];
    
    if (horizontal)
    {
        for (int i = 1; i < 5; ++i)
        {
            result += decay * texture(frame, tex_coords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += decay * texture(frame, tex_coords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for (int i = 1; i < 5; ++i)
        {
            result += decay * texture(frame, tex_coords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += decay * texture(frame, tex_coords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    
    colour = vec4(result, 1.0);
}