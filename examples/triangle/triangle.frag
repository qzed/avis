#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 frag_texcoord;
layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D tex_sampler;

layout(binding = 1) uniform tex_data_ubo {
    int offset;
} tex_data;


void main() {
    vec2 texsize  = textureSize(tex_sampler, 0).st;
    vec2 texcoord = frag_texcoord.ts * texsize;
    texcoord.y = mod(texcoord.y + tex_data.offset, texsize.y);

    float val = texture(tex_sampler, texcoord).r;

    vec3 color = vec3(0.0, 0.0, 0.0);
    color = mix(color, vec3(0.9, 0.9, 0.9), smoothstep(0.00, 0.50, val));   // white
    color = mix(color, vec3(1.0, 0.0, 0.0), smoothstep(0.50, 1.00, val));   // red

    out_color = vec4(color, 1.0);
}
