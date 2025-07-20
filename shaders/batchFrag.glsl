#version 330 core

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define HAS_SHADOWS ${HAS_SHADOWS=0}

#define NR_POINT_LIGHTS ${NR_POINT_LIGHTS=1}
#define NR_SPOT_LIGHTS ${NR_SPOT_LIGHTS=1}

#if NR_POINT_LIGHTS
uniform PointLight pointLights[NR_POINT_LIGHTS];
#endif
#if NR_SPOT_LIGHTS
uniform SpotLight spotLights[NR_SPOT_LIGHTS];
#endif
uniform DirLight dirLight;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform float shininess;

#if HAS_SHADOWS
in vec4 fragPosLightSpace;

uniform sampler2D shadowMap;

uniform float shadowBiasMax = 0.05;
uniform float shadowBiasMin = 0.005;
#endif

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;

out vec4 FragColor;

uniform vec3 cameraPos;
uniform float opacity = 1.0;

float specularStrength = 1.0;

#if HAS_SHADOWS
float calculateShadow(DirLight light, vec4 fragPosLightSpace);
#endif

vec3 calculateDirectionalLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calculatePointLight(PointLight light, vec3 normal, vec3 viewDir);
vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 viewDir);

void main() {
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(cameraPos - fragPos);
    vec3 result = calculateDirectionalLight(dirLight, norm, viewDir);

#if NR_POINT_LIGHTS
    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += calculatePointLight(pointLights[i], norm, viewDir);
    }
#endif
#if NR_SPOT_LIGHTS
    for (int i = 0; i < NR_SPOT_LIGHTS; i++) {
        result += calculateSpotLight(spotLights[i], norm, viewDir);
    }
#endif

    FragColor = vec4(result, opacity);
}

#if HAS_SHADOWS
float calculateShadow(DirLight light, vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    vec3 lightDir = normalize(-light.direction);
    float bias = max(shadowBiasMax * (1.0 - dot(normal, lightDir)), shadowBiasMin);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            if (projCoords.z - bias > pcfDepth)
                shadow += 1.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}
#endif

vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 viewDir) {
    vec3 ambient = light.ambient * texture(texture_diffuse1, texCoord).rgb;

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = texture(texture_diffuse1, texCoord).rgb * diff * light.diffuse;

    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = texture(texture_specular1, texCoord).rgb * spec * light.specular;

    // spotlight (soft edges)
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = (light.cutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    diffuse *= intensity;
    specular *= intensity;

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 calculateDirectionalLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float diff = max(dot(normal, lightDir), 0.0);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, texCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, texCoord));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, texCoord));

#if HAS_SHADOWS
    return (ambient + (1.0 - 0.5 * calculateShadow(light, fragPosLightSpace)) * (diffuse + specular));
#else
    return (ambient + diffuse + specular);
#endif
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float diff = max(dot(normal, lightDir), 0.0);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, texCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, texCoord));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, texCoord));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}