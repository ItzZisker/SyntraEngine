#version 330 core

#define HAS_SHADOWS ${HAS_SHADOWS=0}
#define NR_POINT_LIGHTS ${NR_POINT_LIGHTS=1}
#define NR_SPOT_LIGHTS ${NR_SPOT_LIGHTS=1}

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

#if NR_POINT_LIGHTS
uniform PointLight pointLights[NR_POINT_LIGHTS];
#endif
#if NR_SPOT_LIGHTS
uniform SpotLight spotLights[NR_SPOT_LIGHTS];
#endif
uniform DirLight dirLight;

uniform vec3 cameraPos;

uniform float roughnessConstrant = 4.0;
uniform bool roughness = false;
uniform bool parallax = false;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;
uniform sampler2D texture_roughness1;

uniform float opacity = 1.0;
uniform float specularStrength = 1.0;
uniform float shininess = 32.0;
uniform float parallaxMinLayers = 8.0;
uniform float parallaxMaxLayers = 32.0;
uniform float height_scale = 0.05;

#if HAS_SHADOWS
uniform sampler2D shadowMap;
uniform float shadowStrength = 0.5;
uniform float shadowBiasMax = 0.05;
uniform float shadowBiasMin = 0.005;
uniform float shadowPCFScale = 1.0;
uniform int shadowPCFRadius = 10;
#endif

in VS_OUT {
    vec3 Normal;
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
#if HAS_SHADOWS
    vec4 FragPosLightSpace;
#endif
} fs_in;

out vec4 FragColor;

#if HAS_SHADOWS
float calculateShadow(DirLight light, vec4 fragPosLightSpace);
#endif

float sampleShininess(vec2 texCoords);

vec2 calculateParallax(vec2 texCoords, vec3 viewDir);
vec3 calculateDirectionalLight(DirLight light, vec3 normal, vec3 viewDir, vec2 texCoords);
vec3 calculatePointLight(PointLight light, vec3 normal, vec3 viewDir, vec2 texCoords);
vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec2 texCoords);

void main() {
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(cameraPos - fs_in.FragPos);
    vec2 texCoords;

    if (parallax) {
        texCoords = calculateParallax(fs_in.TexCoords, normalize(fs_in.TBN * -viewDir));

        // if (texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0) {
        //     discard;
        // }
    } else {
        texCoords = fs_in.TexCoords;
    }

    vec4 texColor = texture(texture_diffuse1, texCoords);
    if (texColor.a < 0.1) discard;

    normal = texture(texture_normal1, texCoords).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(fs_in.TBN * normal);

    vec3 result = calculateDirectionalLight(dirLight, normal, viewDir, texCoords);

#if NR_POINT_LIGHTS
    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += calculatePointLight(pointLights[i], normal, viewDir, texCoords);
    }
#endif
#if NR_SPOT_LIGHTS
    for (int i = 0; i < NR_SPOT_LIGHTS; i++) {
        result += calculateSpotLight(spotLights[i], normal, viewDir, texCoords);
    }
#endif

    // if (parallax) {
    //     FragColor = vec4(100.0, 0.0, 0.0, opacity);
    // } else {
        FragColor = vec4(result, opacity);
    // }
}

float sampleShininess(vec2 texCoords) {
    if (roughness) { 
        vec3 texColor = texture(texture_roughness1, texCoords).rgb;
        //float roughnessValue = dot(texColor, vec3(0.299, 0.587, 0.114));
        return mix(8.0, 32.0, 1.0 - ((texColor.r - 0.5) * max(roughnessConstrant, 0)) + 0.5);
    } else {
        return shininess;
    }
}

vec2 calculateParallax(vec2 texCoords, vec3 viewDir) { 
    float numLayers = mix(parallaxMaxLayers, parallaxMinLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));  
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;
    vec2 P = viewDir.xy * height_scale; 
    vec2 deltaTexCoords = P / numLayers;

    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(texture_height1, currentTexCoords).r;
  
    while(currentLayerDepth < currentDepthMapValue) {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(texture_height1, currentTexCoords).r;  
        currentLayerDepth += layerDepth;  
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(texture_height1, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

#if HAS_SHADOWS
float calculateShadow(DirLight light, vec3 normal, vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    vec3 lightDir = normalize(-light.direction);
    float bias = max(shadowBiasMax * (1.0 - dot(normal, lightDir)), shadowBiasMin);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    vec2 offsetSize = texelSize * shadowPCFScale;

    int samples = 0;
    for (int x = -shadowPCFRadius; x <= shadowPCFRadius; ++x) {
        for (int y = -shadowPCFRadius; y <= shadowPCFRadius; ++y) {
            vec2 offset = vec2(x, y) * offsetSize;
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            if (projCoords.z - bias > pcfDepth)
                shadow += 1.0;
            samples += 1;
        }
    }
    shadow /= float(samples);

    return shadow;
}
#endif

vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec2 texCoords) {
    vec3 fragPos = fs_in.FragPos;
    vec3 ambient = light.ambient * texture(texture_diffuse1, texCoords).rgb;

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = texture(texture_diffuse1, texCoords).rgb * diff * light.diffuse;

    float spec = pow(max(dot(normal, halfwayDir), 0.0), sampleShininess(texCoords));
    vec3 specular = texture(texture_specular1, texCoords).rgb * spec * light.specular;

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

vec3 calculateDirectionalLight(DirLight light, vec3 normal, vec3 viewDir, vec2 texCoords) {
    vec3 lightDir = normalize(-light.direction);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float diff = max(dot(normal, lightDir), 0.0);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), sampleShininess(texCoords));
    
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, texCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, texCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, texCoords));

#if HAS_SHADOWS
    return (ambient + (1.0 - shadowStrength * calculateShadow(light, normal, fs_in.FragPosLightSpace)) * (diffuse + specular));
#else
    return (ambient + diffuse + specular);
#endif
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 viewDir, vec2 texCoords) {
    vec3 fragPos = fs_in.FragPos;
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float diff = max(dot(normal, lightDir), 0.0);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), sampleShininess(texCoords));

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, texCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, texCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, texCoords));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}