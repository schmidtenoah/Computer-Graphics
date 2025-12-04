#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 emission;
    float shininess;
    float alpha;
};

struct PointLight {
    vec3 posVS;
    vec3 color;
    vec3 falloff;  // x: constant, y: linear, z: quadratic
    bool enabled;
    float ambientFactor;
};

in vec3 v_fragPosVS;
in vec3 v_normalVS;
in vec2 v_texCoords;

uniform Material u_material;
uniform PointLight u_pointLight;
uniform vec3 u_camPosVS;
uniform bool u_useMaterial;
uniform bool u_useTexture;
uniform sampler2D u_texture;

out vec4 fragColor;

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, Material mat) {
    vec3 lightDir = normalize(light.posVS - fragPos);
    float distance = length(light.posVS - fragPos);

    // Attenuation
    float attenuation = 1.0 / (light.falloff.x + light.falloff.y * distance + light.falloff.z * distance * distance);

    // Ambient
    vec3 ambient = light.ambientFactor * mat.ambient * light.color;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * mat.diffuse * light.color;

    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), mat.shininess);
    vec3 specular = spec * mat.specular * light.color;

    return (ambient + diffuse + specular) * attenuation;
}

void main() {
    vec3 color;

    if (u_useMaterial) {
        vec3 norm = normalize(v_normalVS);
        vec3 viewDir = normalize(u_camPosVS - v_fragPosVS);

        vec3 result = u_material.emission;

        if (u_pointLight.enabled) {
            result += calculatePointLight(u_pointLight, norm, v_fragPosVS, viewDir, u_material);
        } else {
            // Fallback: simple ambient lighting
            result += u_material.ambient + u_material.diffuse * 0.5;
        }

        // Apply texture if enabled
        if (u_useTexture) {
            vec4 texColor = texture(u_texture, v_texCoords);
            result *= texColor.rgb;
        }

        color = result;
    } else {
        // Height-based coloring for surface without material
        float height = v_fragPosVS.y;
        float normalizedHeight = clamp(height * 2.0, 0.0, 1.0);

        vec3 lowColor = vec3(0.2, 0.4, 0.8);   // Blue
        vec3 highColor = vec3(0.8, 0.9, 0.3);  // Yellow

        color = mix(lowColor, highColor, normalizedHeight);

        // Apply texture if enabled
        if (u_useTexture) {
            vec4 texColor = texture(u_texture, v_texCoords);
            color *= texColor.rgb;
        }

        // Simple lighting
        vec3 norm = normalize(v_normalVS);
        vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
        float diff = max(dot(norm, lightDir), 0.0);
        color *= (0.3 + 0.7 * diff);
    }

    // Apply alpha from material
    float alpha = u_useMaterial ? u_material.alpha : 1.0;

    fragColor = vec4(color, alpha);
}