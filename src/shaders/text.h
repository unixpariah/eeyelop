#ifndef TEXT_SHADER_H
#define TEXT_SHADER_H

#include <stdint.h>
const uint8_t *text_vertex_source =
    "#version 460 core\n"
    "\n"
    "layout(location = 0) in vec2 in_pos;\n"
    "uniform vec4 color;\n"
    "uniform mat4 transform[400];\n"
    "uniform mat4 projection;"
    "\n"
    "out VS_OUT {\n"
    "  vec2 pos;\n"
    "  vec2 texCoords;\n"
    "  flat int index;\n"
    "  vec4 color;\n"
    "} vs_out;\n"
    "\n"
    "void main() {\n"
    "  vec4 position =\n"
    "      projection * transform[gl_InstanceID] * vec4(in_pos, 0.0, 1.0);\n"
    "  gl_Position = position;\n"
    "\n"
    "  vs_out.pos = position.xy;\n"
    "  vs_out.texCoords = in_pos;\n"
    "  vs_out.index = gl_InstanceID;\n"
    "  vs_out.color = color;\n"
    "}\n";

const uint8_t *text_fragment_source =
    "#version 460 core\n"
    "layout(location = 0) out vec4 FragColor;\n"
    "\n"
    "uniform sampler2DArray text;\n"
    "uniform int letterMap[400];\n"
    "\n"
    "in VS_OUT {\n"
    "  vec2 pos;\n"
    "  vec2 texCoords;\n"
    "  flat int index;\n"
    "  vec4 color;\n"
    "} fs_in;\n"
    "\n"
    "void main() {\n"
    "  FragColor =\n"
    "      fs_in.color * texture(text, vec3(fs_in.texCoords.xy, "
    "letterMap[fs_in.index])).r;\n"
    "}\n";

#endif // TEXT_SHADER_H
