#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 frag_texcoord;
layout(location = 0) out vec4 out_color;
layout(binding = 0) uniform sampler2D tex_sampler;


void main() {
    out_color = vec4(texture(tex_sampler, frag_texcoord).rgb, 1.0);
}
