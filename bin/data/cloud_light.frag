OF_GLSL_SHADER_HEADER

// base frag shader for lighting code (Modified from class code to work for cloud meshes).

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

out vec4 fragColor;

void main()
{

    // Ambient
    float ambientStrength = 0.1;
    float ambient = ambientStrength;

    // Diffuse
    vec3 N = normalize(Normal); // interpolation might give something that isn't unit length
    vec3 L = normalize(lightPos - FragPos);
    float diffuse = max(dot(N, L), 0.0);
    

    // Specular
    float specularStrength = 0.5;
    vec3 V = normalize(viewPos - FragPos);
    vec3 R = reflect(-L, N);

	//vec3 R = normalize(-L + 2.0*N*dot(L,N));

    float spec = pow(max(dot(V, R), 0.0), 32);
    float specular = specularStrength * spec;

	//specular = 0;
    //vec3 result = (ambient + diffuse + specular) * objectColor;

    vec3 result = (ambient + diffuse + specular) * objectColor * lightColor; // Added lightColor to factor into result.
    fragColor = vec4(result.rgb, 1.0);

  //  fragColor.rgb = vec3(N.x*0.5+0.5, 0.3, N.z*0.5+0.5);
 //   fragColor = vec4(0.7,0.3,0.1,1.0); // test with constant color
}

