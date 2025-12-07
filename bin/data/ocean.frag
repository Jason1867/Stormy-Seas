OF_GLSL_SHADER_HEADER

// Fragment shader for stormy ocean with lighting and foam

uniform vec3 lightPosition;      // Light position in view space
uniform vec3 lightColor;
uniform vec3 waterColorDeep;
uniform vec3 waterColorShallow;
uniform vec3 foamColor;
uniform float fogDensity;
uniform vec3 fogColor;
uniform float time;

varying vec3 fragPosition;
varying vec3 fragNormal;
varying vec2 fragTexCoord;
varying float waveHeight;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    for (int i = 0; i < 4; i++) {
        value += amplitude * noise(p * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    
    return value;
}

void main() {
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(-fragPosition);
    vec3 L = normalize(lightPosition - fragPosition);
    vec3 H = normalize(L + V);
    
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    
    // Lighting components
    vec3 ambient = 0.15 * lightColor;
    vec3 diffuse = 1.0 * NdotL * lightColor;
    float shininess = 150.0;
    vec3 specular = 1.2 * pow(NdotH, shininess) * lightColor;
    
    // Fresnel
    float fresnel = pow(1.0 - max(dot(N, V), 0.0), 4.0);
    fresnel = mix(0.02, 0.6, fresnel);
    
    // Water color
    float depthFactor = clamp((waveHeight + 25.0) / 50.0, 0.0, 1.0);
    vec3 waterColor = mix(waterColorDeep, waterColorShallow, depthFactor);
    
    // Foam
    float foamAmount = 0.0;
    if (waveHeight > 18.0) {
        float crestFoam = (waveHeight - 18.0) / 20.0;
        vec2 foamCoord = fragPosition.xz * 0.1 + time * 0.1;
        float foamNoise = fbm(foamCoord);
        foamAmount = crestFoam * foamNoise * 0.8;
    }
    
    float steepness = length(vec2(dFdx(waveHeight), dFdy(waveHeight))) * 0.5;
    if (steepness > 0.35) {
        foamAmount = max(foamAmount, (steepness - 0.35) * 1.5);
    }
    
    foamAmount = clamp(foamAmount, 0.0, 0.7);
    
    // Combine
    vec3 litColor = waterColor * (ambient + diffuse) + specular;
    
    vec3 skyColor = vec3(0.4, 0.45, 0.5);
    litColor = mix(litColor, skyColor, fresnel * 0.3);
    
    vec3 finalColor = mix(litColor, foamColor, foamAmount);
    
    // Dense dark mist (linear distance fog)
    float dist = length(fragPosition);
    float fogFactor = clamp(dist * fogDensity * 0.01, 0.0, 0.85);
    finalColor = mix(finalColor, fogColor, fogFactor);
    
    gl_FragColor = vec4(finalColor, 1.0);
}
