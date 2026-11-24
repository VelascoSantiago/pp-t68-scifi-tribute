#version 330 core
in vec3 TexDir;
out vec4 color;
uniform samplerCube skybox;
void main()
{
    color = vec4(texture(skybox, TexDir).rgb, 1.0);
}
