#version 330 core

#define HAS_SHADOWS ${HAS_SHADOWS=0}

const float M_PI = 3.141592653589793;

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Encapsulate the various inputs used by the various functions in the shading equation
// We store values in this struct to simplify the integration of alternative implementations
// of the shading terms, outlined in the Readme.MD Appendix.
struct PBRInfo {
    // geometry properties
    float NdotL;        // cos angle between normal and light direction
    float NdotV;        // cos angle between normal and view direction
    float NdotH;        // cos angle between normal and half vector
    float LdotH;        // cos angle between light direction and half vector
    float VdotH;        // cos angle between view direction and half vector

    // Normal
    vec3 n;             // Sahding normal
    vec3 ng;            // Geometry normal
    vec3 t;             // Geometry tangent
    vec3 bit;             // Geometry bitangent
    vec3 v;             // vector from surface point to camera

    // material properties
    float perceptualRoughness;    // roughness value, as authored by the model creator (input to shader)
    vec3 reflectance0;            // full reflectance color (normal incidence angle)
    vec3 reflectance90;           // reflectance color at grazing angle
    float alphaRoughness;         // roughness mapped to a more linear change in the roughness (proposed by [2])
    vec3 baseDiffuseColor;        // color contribution from diffuse lighting
    vec3 baseSpecularColor;       // color contribution from specular lighting

    float ior;

    vec3 FssEss;
    float brdf_scale;
    float brdf_bias;
};

struct InputAttributes {
    vec2 uv[2];
};

struct MetallicRoughnessDataGPU {
    int occlusionTextureUV;
    int emissiveTextureUV;
    vec4 emissiveFactorAlphaCutoff;
    int baseColorTextureUV;
    vec4 baseColorFactor;
    int metallicRoughnessTextureUV;
    int normalTextureUV;
    vec4 metallicRoughnessNormalOcclusion; // packed metallicFactor, roughnessFactor, normalScale, occlusionStrength
    float ior;
};

uniform DirLight dirLight;
uniform vec3 cameraPos;

uniform float gamma;

uniform bool hasNormalMap;
uniform bool hasIBL;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;
uniform sampler2D texture_roughness1;
uniform sampler2D texture_metallic1;
uniform sampler2D texture_emissive1;
uniform sampler2D texture_ao1;

uniform vec4 metallicRoughnessNormalOcclusion;
uniform vec4 baseColor;
uniform vec4 emissiveColor;

uniform float ior;
uniform float opacity;
uniform float specularStrength;
uniform float shininess;

uniform float IBLRadianceLambertianFactor = 0.25;
uniform float IBLRadianceGGXFactor = 0.225;

#if HAS_SHADOWS
uniform sampler2D shadowMap;
uniform float shadowStrength;
uniform float shadowBiasMax;
uniform float shadowBiasMin;
uniform float shadowPCFScale;
uniform int shadowPCFRadius;
#endif

in VS_OUT {
    vec3 Normal;
    vec3 FragPos;
    vec2 TexCoord0;
    vec2 TexCoord1;
    vec4 Color;
    mat3 TBN;
#if HAS_SHADOWS
    vec4 FragPosLightSpace;
#endif
} fs_in;

out vec4 FragColor;

MetallicRoughnessDataGPU getMaterial() {
    MetallicRoughnessDataGPU res;
    res.occlusionTextureUV = 0;
    res.emissiveTextureUV = 0;
    res.emissiveFactorAlphaCutoff = emissiveColor;
    res.baseColorFactor = baseColor;
    res.metallicRoughnessTextureUV = 0;
    res.normalTextureUV = 0;
    res.metallicRoughnessNormalOcclusion = metallicRoughnessNormalOcclusion;
    res.ior = ior;
    return res;
}

float GetMetallicFactor(MetallicRoughnessDataGPU mat)  {
    return mat.metallicRoughnessNormalOcclusion.x;
}

float GetRoughnessFactor(MetallicRoughnessDataGPU mat)  {
    return mat.metallicRoughnessNormalOcclusion.y;
}

float GetNormalScale(MetallicRoughnessDataGPU mat)  {
    return mat.metallicRoughnessNormalOcclusion.z;
}

float GetOcclusionFactor(MetallicRoughnessDataGPU mat)  {
    return mat.metallicRoughnessNormalOcclusion.w;
}

vec2 GetNormalUV(InputAttributes tc, MetallicRoughnessDataGPU mat)  {
    return tc.uv[mat.normalTextureUV];
}

vec4 sampleAO(InputAttributes tc, MetallicRoughnessDataGPU mat) {
    return texture(texture_ao1, tc.uv[mat.occlusionTextureUV]);
}

vec4 SampleEmissive(InputAttributes tc, MetallicRoughnessDataGPU mat) {
    return texture(texture_emissive1, tc.uv[mat.emissiveTextureUV]) * vec4(mat.emissiveFactorAlphaCutoff.xyz, 1.0f);
}

vec4 sampleAlbedo(InputAttributes tc, MetallicRoughnessDataGPU mat) {
    return texture(texture_diffuse1, tc.uv[mat.baseColorTextureUV]) * mat.baseColorFactor;
}

vec4 sampleMetallicRoughness(InputAttributes tc, MetallicRoughnessDataGPU mat) {
    return texture(texture_roughness1, tc.uv[mat.metallicRoughnessTextureUV]);
}

vec3 sampleNormal(InputAttributes tc, MetallicRoughnessDataGPU mat) {
    return texture(texture_normal1, tc.uv[mat.normalTextureUV]).rgb;
}

// Standard Schlick approximation without environment contribution for low-end hardware with limited textures
vec3 CalcSchlickFresnelMinimal(vec3 F0, float NdotV) {
    return F0 + (1.0 - F0) * pow(1.0 - NdotV, 5.0);
}

// Source: OGLDev
// vec3 CalcSchlickFresnel(inout PBRInfo pbrInputs) {
//     EnvironmentMapDataGPU envMap = getEnvironment(getEnvironmentId());

//     vec2 brdfSamplePoint = vec2(pbrInputs.NdotV, pbrInputs.perceptualRoughness);
//     brdfSamplePoint = clamp(brdfSamplePoint, vec2(0.0, 0.0), vec2(1.0, 1.0));
//     vec2 brdf = sampleBRDF_LUT(brdfSamplePoint, envMap).rg;
//     pbrInputs.brdf_scale = brdf.x;
//     pbrInputs.brdf_bias = brdf.y;

//     // see https://bruop.github.io/ibl/#single_scattering_results at Single Scattering Results
//     // Roughness dependent fresnel, from Carmelo Fdez-Aguera
//     vec3 Fr = max(vec3(1.0 - pbrInputs.perceptualRoughness), pbrInputs.reflectance0) - pbrInputs.reflectance0;
//     vec3 k_S = pbrInputs.reflectance0 + Fr * pow(1.0 - pbrInputs.NdotV, 5.0);
//     vec3 FssEss = k_S * pbrInputs.brdf_scale + pbrInputs.brdf_bias;

//     return FssEss;
// }

vec3 diffuseBurley(PBRInfo pbrInputs) 
{
    float f90 = 2.0 * pbrInputs.LdotH * pbrInputs.LdotH * pbrInputs.alphaRoughness - 0.5;

    return (pbrInputs.baseDiffuseColor / M_PI) * (1.0 + f90 * pow((1.0 - pbrInputs.NdotL), 5.0)) * 
                                                 (1.0 + f90 * pow((1.0 - pbrInputs.NdotV), 5.0));
}

// Source: OGLDev
PBRInfo CalculatePBRInputsMetallicRoughness(MetallicRoughnessDataGPU mat, vec4 albedo, vec3 normal, vec4 mrSample) {
    PBRInfo pbrInputs;
    pbrInputs.ior = mat.ior;
  
    // Roughness is stored in the 'g' channel, MetallicFactor is stored in the 'b' channel.
    // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
    
    float MetallicFactor = GetMetallicFactor(mat) * mrSample.b;
    MetallicFactor = clamp(MetallicFactor, 0.0, 1.0);

    float PerceptualRoughness = GetRoughnessFactor(mat);
    PerceptualRoughness = mrSample.g * PerceptualRoughness;
    const float c_MinRoughness = 0.04;
    PerceptualRoughness = clamp(PerceptualRoughness, c_MinRoughness, 1.0);

    // Roughness is authored as perceptual roughness; as is convention,
    // convert to material roughness by squaring the perceptual roughness [2].
    float alphaRoughness = PerceptualRoughness * PerceptualRoughness;

    vec3 f0 = vec3(0.04);
    vec3 baseDiffuseColor = mix(albedo.rgb, vec3(0), MetallicFactor); 
    vec3 baseSpecularColor = mix(f0, albedo.rgb, MetallicFactor);

    float reflectance = max(max(baseSpecularColor.r, baseSpecularColor.g), baseSpecularColor.b);

    float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
    vec3 specularEnvironmentR0 = baseSpecularColor.rgb;
    vec3 specularEnvironmentR90 = vec3(reflectance90);

    vec3 v = normalize(cameraPos - fs_in.FragPos);

    pbrInputs.NdotV = clamp(abs(dot(normal, v)), 0.001, 1.0);
    pbrInputs.perceptualRoughness = PerceptualRoughness;
    pbrInputs.reflectance0 = specularEnvironmentR0;
    pbrInputs.reflectance90 = specularEnvironmentR90;
    pbrInputs.alphaRoughness = alphaRoughness;
    pbrInputs.baseDiffuseColor = baseDiffuseColor;
    pbrInputs.baseSpecularColor = baseSpecularColor;
    pbrInputs.n = normal;
    pbrInputs.v = v;

    if (hasIBL) {
        // Disabled for now, we have to detect amount of bindable textures on hardware GPU, then decide to whether bind the environment map or not
        //pbrInputs.FssEss = CalcSchlickFresnel(pbrInputs);
    } else {
        pbrInputs.FssEss = CalcSchlickFresnelMinimal(pbrInputs.reflectance0, pbrInputs.NdotV);
    }
    return pbrInputs;
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
vec3 specularReflection(PBRInfo pbrInputs)  {
    return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance0) * 
           pow(clamp(1.0 - pbrInputs.VdotH, 0.0, 1.0), 5.0);
}

// This calculates the specular geometric attenuation (aka G()),
// where rougher material will reflect less light back to the viewer.
// This implementation is based on [1] Equation 4, and we adopt their modifications to
// alphaRoughness as input as originally proposed in [2].
float geometricOcclusion(PBRInfo pbrInputs)  {
    float NdotL = pbrInputs.NdotL;
    float NdotV = pbrInputs.NdotV;
    float rSqr = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;

    float attenuationL = 2.0 * NdotL / (NdotL + sqrt(rSqr + (1.0 - rSqr) * (NdotL * NdotL)));
    float attenuationV = 2.0 * NdotV / (NdotV + sqrt(rSqr + (1.0 - rSqr) * (NdotV * NdotV)));
    return attenuationL * attenuationV;
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float microfacetDistribution(PBRInfo pbrInputs)  {
    float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
    float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
    return roughnessSq / (M_PI * f * f);
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

// Source: OGLDev
vec3 calculatePBRLightContribution(inout PBRInfo pbrInputs, DirLight light)  {
    vec3 n = pbrInputs.n;
    vec3 v = pbrInputs.v;
    vec3 l = normalize(-light.direction);  // Vector from surface point to light
    vec3 h = normalize(l+v);             // Half vector between both l and v

    float NdotV = pbrInputs.NdotV;
    float NdotL = clamp(dot(n, l), 0.001, 1.0);
    float NdotH = clamp(dot(n, h), 0.0, 1.0);
    float LdotH = clamp(dot(l, h), 0.0, 1.0);
    float VdotH = clamp(dot(v, h), 0.0, 1.0);

    vec3 color = vec3(0);

    if (NdotL > 0.0 || NdotV > 0.0) {
        pbrInputs.NdotL = NdotL;
        pbrInputs.NdotH = NdotH;
        pbrInputs.LdotH = LdotH;
        pbrInputs.VdotH = VdotH;

        // Calculate the shading terms for the microfacet specular shading model
        vec3 F = specularReflection(pbrInputs);
        float G = geometricOcclusion(pbrInputs);
        float D = microfacetDistribution(pbrInputs);

        // Calculation of analytical lighting contribution
        vec3 diffuseContrib = (1.0 - F) * diffuseBurley(pbrInputs);
        vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
        // Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)
        color = NdotL * light.diffuse * (diffuseContrib + specContrib);
#if HAS_SHADOWS
        color *= mix(1.0, 1.0 - clamp(shadowStrength, 0.0, 1.0), calculateShadow(light, n, fs_in.FragPosLightSpace));
#endif
    }

    return color;
}

// Source: ogldev
// Calculation of the lighting contribution from an optional Image Based Light source.
// Precomputed Environment Maps are required uniform inputs and are computed as outlined in [1].
// See our README.md on Environment Maps [3] for additional discussion.
//vec3 getIBLRadianceGGX(vec3 n, vec3 v, float roughness, vec3 F0) 
// {
//     EnvironmentMapDataGPU envMap = getEnvironment(getEnvironmentId());       
//     float mipCount = float(textureQueryLevels(envMap.envMapTextureSampler));
//     float lod = roughness * (mipCount - 1);
//     vec3 reflection = normalize(reflect(-v, n));
    
//     // HDR envmaps are already linear
//     vec3 specularLight = sampleEnvMapLod(reflection.xyz, lod, envMap).rgb;

//     float NdotV = ClampedDot(n, v);
//     vec2 brdfSamplePoint = clamp(vec2(NdotV, roughness), vec2(0.0, 0.0), vec2(1.0, 1.0));
//     vec3 f_ab = sampleBRDF_LUT(brdfSamplePoint, envMap).rgb;

//     // see https://bruop.github.io/ibl/#single_scattering_results at Single Scattering Results
//     // Roughness dependent fresnel, from Fdez-Aguera
//     vec3 Fr = max(vec3(1.0 - roughness), F0) - F0;
//     vec3 k_S = F0 + Fr * pow(1.0 - NdotV, 5.0);
//     vec3 FssEss = k_S * f_ab.x + f_ab.y;

//     return specularLight * FssEss;
// }

// Non-IBL Radiance GGX
vec3 getIBLRadianceGGXMinimal(vec3 n, vec3 v, float roughness, vec3 F0)
{
    // Compute simple Fresnel (view-dependent shine)
    float NdotV = max(dot(n, v), 0.0);
    vec3 Fr = F0 + (1.0 - F0) * pow(1.0 - NdotV, 5.0);

    // Optional: modulate intensity by roughness (rough = less reflection)
    float intensity = 1.0 - roughness;

    // Optional: fake constant "ambient reflection"
    vec3 fakeReflection = dirLight.ambient * IBLRadianceGGXFactor; // small constant reflection
    return fakeReflection * Fr * intensity;
}

// vec3 getIBLRadianceLambertian(PBRInfo pbrInputs) 
// {
//     EnvironmentMapDataGPU envMap = getEnvironment(getEnvironmentId());

//     vec3 Irradiance = texture(envMap.envMapTextureIrradianceSampler, pbrInputs.n).rgb;

//     // Multiple scattering, from Fdez-Aguera
//     float Ems = (1.0 - (pbrInputs.brdf_scale + pbrInputs.brdf_bias));
//     vec3 F_avg = (pbrInputs.reflectance0 + (1.0 - pbrInputs.reflectance0) / 21.0);
//     vec3 FmsEms = F_avg * Ems * pbrInputs.FssEss / (1.0 - F_avg * Ems);
//     // we use +FmsEms as indicated by the formula in the blog post 
//     // (might be a typo in the implementation)
//     vec3 k_D = pbrInputs.baseDiffuseColor * (1.0 - pbrInputs.FssEss + FmsEms); 

//     return (FmsEms + k_D) * Irradiance;
// }

// Non-IBL Radiance Lambertian
vec3 getIBLRadianceLambertianMinimal(PBRInfo pbrInputs)
{
    // Approximate ambient light color
    vec3 ambientLight = dirLight.ambient * IBLRadianceLambertianFactor;

    // Basic diffuse term (Lambert)
    vec3 k_D = pbrInputs.baseDiffuseColor * (1.0 - pbrInputs.FssEss);
    return ambientLight * k_D;
}

void main() {
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(cameraPos - fs_in.FragPos);

    InputAttributes tc;
    tc.uv[0] = fs_in.TexCoord0;
    tc.uv[1] = fs_in.TexCoord1;
    MetallicRoughnessDataGPU mat = getMaterial();

    vec4 AlbedoColor = sampleAlbedo(tc, mat);// * fs_in.Color;
    if (texture(texture_diffuse1, tc.uv[0]).a < 0.1) discard;
    if (hasNormalMap) {
        normal = sampleNormal(tc, mat);
        normal = normal * 2.0 - 1.0;
        normal = normalize(fs_in.TBN * normal);
    }

    vec4 mrSample = sampleMetallicRoughness(tc, mat);
    vec4 EmissiveColor = SampleEmissive(tc, mat);
    vec4 AmbientOcclusion = sampleAO(tc, mat);

    if (!gl_FrontFacing) normal *= -1.0;

    float Occlusion = AmbientOcclusion.r < 0.01 ? 1.0 : AmbientOcclusion.r;
    float OcclusionStrength = GetOcclusionFactor(mat);

    PBRInfo pbrInputs = CalculatePBRInputsMetallicRoughness(mat, AlbedoColor, normal, mrSample);
    
    vec3 SpecularColor = getIBLRadianceGGXMinimal(pbrInputs.n, pbrInputs.v, pbrInputs.perceptualRoughness, pbrInputs.reflectance0);
    vec3 DiffuseColor = getIBLRadianceLambertianMinimal(pbrInputs);

    DiffuseColor = mix(DiffuseColor, DiffuseColor * Occlusion, OcclusionStrength);
    SpecularColor = mix(SpecularColor, SpecularColor * Occlusion, OcclusionStrength);

    vec3 LightContribution = calculatePBRLightContribution(pbrInputs, dirLight);

    vec3 Color = SpecularColor + DiffuseColor + LightContribution;
    if (AmbientOcclusion.r >= 0.1) {
        Color *= AmbientOcclusion.r;
    }
    Color += EmissiveColor.rgb;

    Color.xyz = pow(Color.xyz, vec3(1.0/gamma));
    FragColor = vec4(Color.xyz, opacity);
}