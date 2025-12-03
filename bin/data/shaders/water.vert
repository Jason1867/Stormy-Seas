#version 150

uniform mat4 modelViewProjectionMatrix;
uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform float time;

in vec4 position;
in vec3 normal;

out vec3 FragPos;
out vec3 Normal;

void main() {
    vec4 p = position;
    float waveX = sin(p.x * 0.05 + time) * 10.0;
    float waveZ = cos(p.z * 0.05 + time * 0.7) * 5.0;
    p.y += waveX + waveZ;

    FragPos = vec3(worldMatrix * p);

    mat3 normalMatrix = mat3(
        worldMatrix[0].xyz,
        worldMatrix[1].xyz,
        worldMatrix[2].xyz
    );
    vec3 n = normal;
    n.y += cos(p.x*0.05+time)*0.05 - sin(p.z*0.05+time*0.7)*0.05;
    Normal = normalize(normalMatrix * n);

    gl_Position = projectionMatrix * viewMatrix * worldMatrix * p;
}
