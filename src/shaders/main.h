#ifndef MAIN_SHADER_H
#define MAIN_SHADER_H

#include <stdint.h>
const uint8_t *main_vertex_source =
    "#version 460 core\n"
    "layout(location = 0) in vec2 in_pos;\n"
    "uniform mat4 projection;\n"
    "out vec2 v_pos;\n"
    "void main() {\n"
    "    vec4 position = projection * vec4(in_pos, 0.0, 1.0);\n"
    "    gl_Position = position;\n"
    "    v_pos = position.xy;\n"
    "}\n";

const uint8_t *main_fragment_source =
    "#version 460 core\n"
    "layout(location = 0) out vec4 FragColor;\n"
    "uniform vec4 color;"
    "in vec2 v_pos;\n"
    "void main() {\n"
    "    FragColor = color;\n"
    "}\n";

#endif // MAIN_SHADER_H
