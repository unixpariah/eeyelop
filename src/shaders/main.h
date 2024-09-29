#ifndef MAIN_SHADER_H
#define MAIN_SHADER_H

const char *vertex_shader_source =
    "#version 460 core\n"
    "layout(location = 0) in vec2 in_pos;\n"
    "uniform mat4 projection;\n"
    "out vec2 v_pos;\n"
    "void main() {\n"
    "    vec4 position = projection * vec4(in_pos, 0.0, 1.0);\n"
    "    gl_Position = position;\n"
    "    v_pos = position.xy;\n"
    "}\n";

const char *fragment_shader_source =
    "#version 460 core\n"
    "layout(location = 0) out vec4 FragColor;\n"
    "in vec2 v_pos;\n"
    "void main() {\n"
    "    vec4 color = vec4(1.0);\n"
    "    FragColor = color;\n"
    "}\n";

#endif // MAIN_SHADER_H
