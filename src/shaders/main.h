#ifndef MAIN_SHADER_H
#define MAIN_SHADER_H

const char *vertex_shader_source =
    "#version 460 core\n"
    "layout(location = 0) in vec2 in_pos;\n"
    "layout(std140) uniform UniformBlock { mat4 projection; };\n"
    "out vec2 v_pos;\n"
    "void main() {\n"
    "    vec4 position = projection * vec4(in_pos, 0.0, 1.0);\n"
    "    gl_Position = position;\n"
    "    v_pos = position.xy;\n"
    "}\n";

const char *fragment_shader_source =
    "#version 460 core\n"
    "layout(location = 0) out vec4 FragColor;\n"
    "uniform vec4 u_startcolor;\n"
    "uniform vec4 u_endcolor;\n"
    "uniform float u_degrees;\n"
    "in vec2 v_pos;\n"
    "void main() {\n"
    "    vec2 uv = v_pos - 0.5;\n"
    "    float angle = radians(u_degrees);\n"
    "    vec2 rotatedUV = vec2(cos(angle) * uv.x - sin(angle) * uv.y,\n"
    "                          sin(angle) * uv.x + cos(angle) * uv.y) + 0.5;\n"
    "    float gradientFactor = smoothstep(0.0, 1.0, rotatedUV.x);\n"
    "    vec4 color = mix(u_startcolor, u_endcolor, gradientFactor);\n"
    "    FragColor = color;\n"
    "}\n";

#endif // MAIN_SHADER_H
