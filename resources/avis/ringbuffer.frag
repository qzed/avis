#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 frag_texcoord;
layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D tex_sampler;

layout(binding = 1) uniform tex_data_ubo {
    int offset;
} tex_data;


#define xview   vec2(0.9, 1.0)
#define scale   0.3;


vec2 project(float xmin, float xmax, float ysize, float y_offset, vec2 v) {
    return vec2(mix(xmin, xmax, v.x), mod(v.y * ysize + y_offset, ysize));
}

void main() {
    vec2 texsize  = textureSize(tex_sampler, 0).st;
    vec2 texcoord = project(texsize.x * xview.x, texsize.x * xview.y, texsize.y, tex_data.offset, frag_texcoord.ts);

    float val = texture(tex_sampler, texcoord).r;

    vec3 color = vec3(0.0, 0.0, 0.0);
    color = mix(color, vec3(1.0, 1.0, 1.0), smoothstep(0.00, 1.00, val));   // white
    color = mix(color, vec3(1.0, 0.5, 0.0), smoothstep(1.00, 3.00, val));   // orange
    color = mix(color, vec3(1.0, 0.0, 0.0), smoothstep(3.00, 6.00, val));   // red

    out_color = vec4(color, 1.0);
}
