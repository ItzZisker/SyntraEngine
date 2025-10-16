#version 300 es
precision highp float;
precision highp int;

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

uniform float roughnessConstrant;
uniform bool roughness;
uniform bool parallax;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;
uniform sampler2D texture_roughness1;

uniform float opacity;
uniform float specularStrength;
uniform float shininess;
uniform float parallaxMinLayers;
uniform float parallaxMaxLayers;
uniform float height_scale;

#if HAS_SHADOWS
uniform sampler2D shadowMap;
uniform float shadowStrength;
uniform float shadowBiasMax;
uniform float shadowBiasMin;
uniform float shadowPCFScale;
uniform int shadowPCFRadius;
#endif

in vec3 vs_Normal;
in vec3 vs_FragPos;
in vec2 vs_TexCoords;
in mat3 vs_TBN;
#if HAS_SHADOWS
in vec4 vs_FragPosLightSpace;
#endif

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
    vec3 normal = normalize(vs_Normal);
    vec3 viewDir = normalize(cameraPos - vs_FragPos);
    vec2 texCoords;

    if (parallax) {
        texCoords = calculateParallax(vs_TexCoords, normalize(vs_TBN * -viewDir));
    } else {
        texCoords = vs_TexCoords;
    }

    vec4 texColor = texture(texture_diffuse1, texCoords);
    if (texColor.a < 0.1) discard;

    normal = texture(texture_normal1, texCoords).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(vs_TBN * normal);

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

    FragColor = vec4(result, opacity);
}

float sampleShininess(vec2 texCoords) {
    if (roughness) {
        vec3 texColor = texture(texture_roughness1, texCoords).rgb;
        float rc = max(roughnessConstrant, 0.0);
        float rough = (texColor.r - 0.5) * rc + 0.5;
        return mix(8.0, 32.0, 1.0 - rough);
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

float calculateSpec(vec3 normal, vec3 halfwayDir, vec2 texCoords) {
    float shininess = sampleShininess(texCoords);
    return (( 8.0 + shininess ) / ( 8.0 * 3.14159265 )) * pow(max(dot(normal, halfwayDir), 0.0), shininess);
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
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
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
    vec3 fragPos = vs_FragPos;
    vec3 ambient = light.ambient * texture(texture_diffuse1, texCoords).rgb;

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = texture(texture_diffuse1, texCoords).rgb * diff * light.diffuse;
    vec3 specular = texture(texture_specular1, texCoords).rgb * calculateSpec(normal, halfwayDir, texCoords) * light.specular;

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
    
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, texCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, texCoords));
    vec3 specular = light.specular * calculateSpec(normal, halfwayDir, texCoords) * vec3(texture(texture_specular1, texCoords));

#if HAS_SHADOWS
    return (ambient + (1.0 - shadowStrength * calculateShadow(light, normal, vs_FragPosLightSpace)) * (diffuse + specular));
#else
    return (ambient + diffuse + specular);
#endif
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 viewDir, vec2 texCoords) {
    vec3 fragPos = vs_FragPos;
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, texCoords));

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, texCoords));
    vec3 specular = light.specular * calculateSpec(normal, halfwayDir, texCoords) * vec3(texture(texture_specular1, texCoords));

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}