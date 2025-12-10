OF_GLSL_SHADER_HEADER

// base vertex shader for lighting code (Modified from class code to work for cloud meshes).

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat3 normalMatrix;

in vec4 position; // Standard position.
in vec3 normal; // Standard normal.

out vec3 FragPos; // To set position of shader.
out vec3 Normal; // To set the normal of shader.

void main() {
    vec4 viewPos = modelViewMatrix * position; // Position in world space.
    FragPos = viewPos.xyz; // Set position value.
    Normal  = normalize(normalMatrix * normal); // Set normal value.
    gl_Position = modelViewProjectionMatrix * position; // Set the position of the shader.

    //FragPos = vec3((worldMatrix * position).xyz); // position in world space
    //Normal = normalize((worldMatrix * vec4(normal.xyz,0)).xyz); // normals in world space; note w=0
    //gl_Position = projectionMatrix * viewMatrix * worldMatrix * position;
}
