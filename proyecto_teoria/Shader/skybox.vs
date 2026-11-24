#version 330 core
layout (location = 0) in vec3 position;
out vec3 TexDir;
uniform mat4 view;
uniform mat4 projection;
void main()
{
    TexDir = position;
    vec4 p = projection * view * vec4(position, 1.0);
    gl_Position = p.xyww;   // z = w -> profundidad maxima (siempre al fondo)
}
