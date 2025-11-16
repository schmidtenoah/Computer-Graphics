#version 430

out vec4 fragColor;

#define HEIGHT_LOWEST       -5.0
#define HEIGHT_VERY_LOW     -1.0
#define HEIGHT_LOW          -0.25
#define HEIGHT_MEDIUM       0.001
#define HEIGHT_HIGH         0.25
#define HEIGHT_VERY_HIGH    0.6
#define HEIGHT_HIGHEST      2.0

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 emission;
    float shininess;
};

struct HeightData {
    float heightY;
    vec3 color;
    Material mat;
}; 

const HeightData HEIGHTDATA_LOWEST = HeightData(
    HEIGHT_LOWEST,
    vec3(0.0, 0.0, 1.0),
    Material(
        vec3(0.05, 0.05, 0.1),
        vec3(0.1, 0.1, 0.3),
        vec3(0.1, 0.1, 0.2),
        vec3(0.0),
        8.0
    )
);

const HeightData HEIGHTDATA_VERY_LOW = HeightData(
    HEIGHT_VERY_LOW,
    vec3(0.1, 0.0, 0.7),
    Material(
        vec3(0.05, 0.05, 0.1),
        vec3(0.1, 0.1, 0.5),
        vec3(0.05, 0.05, 0.2),
        vec3(0.0),
        12.0
    )
);

const HeightData HEIGHTDATA_LOW = HeightData(
    HEIGHT_LOW,
    vec3(0.0, 0.0, 0.5),
    Material(
        vec3(0.05, 0.05, 0.1),
        vec3(0.1, 0.1, 0.4),
        vec3(0.05, 0.05, 0.1),
        vec3(0.0),
        16.0
    )
);

const HeightData HEIGHTDATA_MEDIUM = HeightData(
    HEIGHT_MEDIUM,
    vec3(0.0, 0.5, 0.0),
    Material(
        vec3(0.05, 0.1, 0.05),
        vec3(0.1, 0.4, 0.1),
        vec3(0.05, 0.2, 0.05),
        vec3(0.0),
        20.0
    )
);

const HeightData HEIGHTDATA_HIGH = HeightData(
    HEIGHT_HIGH,
    vec3(0.5, 0.5, 0.0),
    Material(
        vec3(0.1, 0.1, 0.05),
        vec3(0.6, 0.6, 0.2),
        vec3(0.3, 0.3, 0.1),
        vec3(0.0),
        32.0
    )
);

const HeightData HEIGHTDATA_VERY_HIGH = HeightData(
    HEIGHT_VERY_HIGH,
    vec3(0.5, 0.2, 0.3),
    Material(
        vec3(0.1, 0.05, 0.05),
        vec3(0.6, 0.3, 0.3),
        vec3(0.4, 0.2, 0.2),
        vec3(0.2, 0.1, 0.05),
        48.0
    )
);

const HeightData HEIGHTDATA_HIGHEST = HeightData(
    HEIGHT_HIGHEST,
    vec3(0.5, 0.1, 0.1),
    Material(
        vec3(0.1, 0.05, 0.05),
        vec3(0.6, 0.2, 0.2),
        vec3(0.4, 0.1, 0.1),
        vec3(0.5, 0.2, 0.1),
        64.0
    )
);

in VS_OUT {
    vec2 TexCoords;
    vec3 PositionWS;
    vec3 NormalVS;
    vec3 PositionVS;
} fs_in;

struct PointLight {
    vec3 posVS;
    vec3 color;
    vec3 falloff;  // x: constant, y: linear, z: quadratic
    bool enabled;
    float ambientFactor;
};

uniform sampler2D u_texture;
uniform bool u_useTexture = false;
uniform vec3 u_camPosVS;
uniform PointLight u_pointLight;

/**
 * Computes Phong lighting contribution for a given light direction and view direction.
 * @param N            Surface normal (normalized)
 * @param L            Light direction vector (normalized)
 * @param V            View direction vector (normalized)
 * @param lightColor   Color of the light source
 * @param matDiffuse   Diffuse color of the material
 * @param matSpecular  Specular color of the material
 * @param shininess    Shininess exponent for specular highlight size
 *
 * @return vec3        Combined diffuse + specular lighting color
 */
vec3 phongLight(vec3 N, vec3 L, vec3 V, vec3 lightColor, vec3 matDiffuse, vec3 matSpecular, float shininess) {
    float NdotL = max(dot(N, L), 0.0);

    // Diffuse
    vec3 diffuse = NdotL * matDiffuse * lightColor;

    // Specular
    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(R, V), 0.0), shininess);
    vec3 specular = spec * matSpecular * lightColor;

    vec3 result = diffuse + specular;
    return clamp(result, 0.0, 1.0);
}

/**
 * Calculates the light contribution of the pointl light.
 * @param N         normal of the material
 * @param V         normal to the camera (view)
 * @param fragPos   view position of the fragment
 * @param m         normal the material properties (diff, spec, ...)
 +
 * @returns the point light contribution 
 */
vec3 pointLightContribution(vec3 N, vec3 V, vec3 fragPos, Material m) {
    if (!u_pointLight.enabled) return vec3(0.0);

    vec3 L = normalize(u_pointLight.posVS - fragPos);
    float dist = length(u_pointLight.posVS - fragPos);
    float att = 1.0 / (u_pointLight.falloff.x + u_pointLight.falloff.y * dist + u_pointLight.falloff.z * dist * dist);

    vec3 ambient  = m.ambient * u_pointLight.color * u_pointLight.ambientFactor;
    vec3 lighting = phongLight(N, L, V, u_pointLight.color, m.diffuse.rgb, m.specular, m.shininess) * att;

    return ambient + lighting + (m.emission * 0.3);
}

/**
 * Extracts the HeightData related to the fragment world position.
 * @param height The height of the fragment in world space.
 * @returns the HeightData related to the fragment world position.
 */
HeightData getHeightData(float height) {
    if (height < HEIGHT_VERY_LOW)
        return HEIGHTDATA_LOWEST;
    else if (height < HEIGHT_LOW)
        return HEIGHTDATA_VERY_LOW;
    else if (height < HEIGHT_MEDIUM)
        return HEIGHTDATA_LOW;
    else if (height < HEIGHT_HIGH)
        return HEIGHTDATA_MEDIUM;
    else if (height < HEIGHT_VERY_HIGH)
        return HEIGHTDATA_HIGH;
    else if (height < HEIGHT_HIGHEST)
        return HEIGHTDATA_VERY_HIGH;
    else
        return HEIGHTDATA_HIGHEST;
}

/**
 * Model Fragment Shader Main.
 * Extracts the height data based on the fragment world y position, 
 * then applies shading.
 */
void main(void) {
    HeightData data = getHeightData(fs_in.PositionWS.y);

    Material mat = data.mat;

    if (u_useTexture) {
        mat.diffuse = texture(u_texture, fs_in.TexCoords).rgb;
    } else {
        mat.diffuse = data.color;
    }

   if (u_pointLight.enabled) {
        vec3 N = normalize(fs_in.NormalVS);
        vec3 V = normalize(u_camPosVS - fs_in.PositionVS);
        vec3 phongColor = pointLightContribution(N, V, fs_in.PositionVS, mat);
        fragColor = vec4(phongColor, 1.0);
    } else {
        fragColor = vec4(mat.diffuse, 1.0);
    }
}
