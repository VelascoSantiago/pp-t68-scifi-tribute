#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in ivec4 boneIds;   // skinning por huesos (solo SkinnedModel)
layout (location = 4) in vec4 boneWeights;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 model;
uniform int skinned;
uniform mat4 bones[100];
uniform mat4 view;
uniform mat4 projection;

void main()
{
    mat4 m = model;
    if (skinned == 1)
    {
        mat4 skin = bones[boneIds.x] * boneWeights.x + bones[boneIds.y] * boneWeights.y
                  + bones[boneIds.z] * boneWeights.z + bones[boneIds.w] * boneWeights.w;
        float w = boneWeights.x + boneWeights.y + boneWeights.z + boneWeights.w;
        if (w > 0.0) m = model * skin;
    }
    gl_Position = projection * view * m * vec4(position, 1.0f);
    FragPos = vec3(m * vec4(position, 1.0f));
    Normal = mat3(transpose(inverse(m))) * normal;
    TexCoords = texCoords;
}
