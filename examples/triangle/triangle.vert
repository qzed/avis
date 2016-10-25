#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_texcoord;

layout(location = 0) out vec2 frag_texcoord;


out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = vec4(in_position, 0.0, 1.0);
    frag_texcoord = in_texcoord;
}
