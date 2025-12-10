OF_GLSL_SHADER_HEADER

// Vertex shader for stormy ocean with Gerstner waves

uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform float time;

// Wave parameters - 5 waves
uniform float amplitudes[5];
uniform float wavelengths[5];
uniform float speeds[5];
uniform vec2 directions[5];

attribute vec4 position;
attribute vec2 texcoord;

varying vec3 fragPosition;       // View space position
varying vec3 fragNormal;         // View space normal
varying vec2 fragTexCoord;
varying float waveHeight;

const float PI = 3.14159265359;

// Simple noise function
float noise(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

float smoothNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = noise(i);
    float b = noise(i + vec2(1.0, 0.0));
    float c = noise(i + vec2(0.0, 1.0));
    float d = noise(i + vec2(1.0, 1.0));
    
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

vec3 gerstnerWave(vec3 pos, float t, out vec3 tangent, out vec3 binormal) {
    vec3 p = pos;
    tangent = vec3(1.0, 0.0, 0.0);
    binormal = vec3(0.0, 0.0, 1.0);
    
    // Add spatial variation to break up uniformity
    float spatialVar = smoothNoise(pos.xz * 0.003);
    
    for (int i = 0; i < 5; i++) {
        // Less wavelength variation for more uniform, realistic waves
        float wavelengthVar = wavelengths[i] * (0.85 + 0.3 * smoothNoise(pos.xz * 0.005 + float(i) * 10.0));
        float k = 2.0 * PI / wavelengthVar;
        float c = speeds[i] * (0.95 + 0.1 * spatialVar);
        
        // Varied phase offsets
        float phase = smoothNoise(vec2(float(i) * 0.1, spatialVar)) * 2.0 * PI;
        
        // Minimal direction variation - wind keeps waves aligned
        float angleOffset = smoothNoise(pos.xz * 0.008 + float(i)) * 0.15;
        float cosA = cos(angleOffset);
        float sinA = sin(angleOffset);
        vec2 dir = vec2(
            directions[i].x * cosA - directions[i].y * sinA,
            directions[i].x * sinA + directions[i].y * cosA
        );
        dir = normalize(dir);
        
        // Minimal amplitude variation for smooth, consistent swells
        float randAmp = amplitudes[i] * (0.9 + 0.2 * smoothNoise(pos.xz * 0.006 + float(i) * 5.0));
        
        float dot_kd = k * dot(dir, pos.xz);
        float theta = dot_kd - c * t + phase;
        float cosTheta = cos(theta);
        float sinTheta = sin(theta);
        
        // Very low steepness for gentle, gradual ocean swells
        float steepness = 0.15 + 0.1 * smoothNoise(pos.xz * 0.008);
        float Q = steepness / (k * randAmp * 5.0);
        
        p.x += Q * randAmp * dir.x * cosTheta;
        p.z += Q * randAmp * dir.y * cosTheta;
        p.y += randAmp * sinTheta;
        
        float WA = k * randAmp;
        float S = sinTheta;
        float C = cosTheta;
        
        tangent += vec3(
            -Q * dir.x * dir.x * WA * S,
            dir.x * WA * C,
            -Q * dir.x * dir.y * WA * S
        );
        
        binormal += vec3(
            -Q * dir.x * dir.y * WA * S,
            dir.y * WA * C,
            -Q * dir.y * dir.y * WA * S
        );
    }
    
    // Subtle surface chop (smaller, faster ripples)
    float chopSum = 0.0;
    // Align chop with primary wind direction
    float windDir = 0.85 * pos.x + 0.52 * pos.z;
    chopSum += 0.4 * sin(0.2 * windDir - 14.0 * t + smoothNoise(pos.xz * 0.12));
    chopSum += 0.2 * sin(0.35 * windDir - 18.0 * t + smoothNoise(pos.xz * 0.18));
    chopSum += 0.1 * sin(0.5 * (pos.x * 0.9 + pos.z * 0.5) - 22.0 * t);
    p.y += chopSum;
    
    return p;
}

void main() {
    vec3 tangent, binormal;
    vec3 worldPos = gerstnerWave(position.xyz, time, tangent, binormal);
    
    // Calculate normal in world space
    vec3 normalVec = normalize(cross(binormal, tangent));
    
    // Transform to view space
    vec4 viewPos = modelViewMatrix * vec4(worldPos, 1.0);
    fragPosition = viewPos.xyz;
    
    // Transform normal to view space
    fragNormal = mat3(modelViewMatrix) * normalVec;
    
    fragTexCoord = texcoord;
    waveHeight = worldPos.y;
    
    gl_Position = modelViewProjectionMatrix * vec4(worldPos, 1.0);
}