layout(set = 0, binding = 0) uniform SceneData
{
    mat4 matrixV;
    mat4 matrixP;
    mat4 matrixVP;
    vec3 ambientColor;
    vec3 mainLightDir;
    vec3 mainLightColor;
    vec3 cameraPos;
};