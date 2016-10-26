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
    vec2 texcoord = frag_texcoord.ts * texsize + vec2(0, tex_data.offset);
    if (texcoord.y > texsize.y) texcoord.y -= texsize.y;

    out_color = vec4(texture(tex_sampler, texcoord).rrr, 1.0);
}
