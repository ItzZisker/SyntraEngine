/* A Program to generate high quality BRDF lookup tables for the split-sum approximation in UE4s Physically based rendering
 * LUTs are stored in 16 bit or 32 bit floating point textures in either KTX or DDS format. 
 * Written by: Hector Medina-Fetterman
 * https://github.com/HectorMF/BRDFGenerator/blob/master/BRDFGenerator/BRDFGenerator.cpp
 */

#include "BRDFGenerator.hpp"
#include "Syngine/modules/Texture.hpp"
#include "Texture.hpp"

#include "stb_image.h"
#include "stb_image_write.h"

#include <stdlib.h>
#include <cmath>

using namespace syng;

float RadicalInverse_VdC(unsigned int bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10;
}

glm::vec2 Hammersley(unsigned int i, unsigned int N) {
	return glm::vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

glm::vec3 ImportanceSampleGGX(glm::vec2 Xi, float roughness, glm::vec3 N) {
	float a = roughness*roughness;

	float phi = 2.0 * M_PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

	// from spherical coordinates to cartesian coordinates
	glm::vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// from tangent-space vector to world-space sample vector
	glm::vec3 up = abs(N.z) < 0.999 ? glm::vec3(0.0, 0.0, 1.0) : glm::vec3(1.0, 0.0, 0.0);
	glm::vec3 tangent = normalize(cross(up, N));
	glm::vec3 bitangent = cross(N, tangent);

	glm::vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
	float a = roughness;
	float k = (a * a) / 2.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(float roughness, float NoV, float NoL) {
	float ggx2 = GeometrySchlickGGX(NoV, roughness);
	float ggx1 = GeometrySchlickGGX(NoL, roughness);

	return ggx1 * ggx2;
}

glm::vec2 IntegrateBRDF_RG(float NdotV, float roughness, unsigned int samples) {
	glm::vec3 V;
	V.x = sqrt(1.0 - NdotV * NdotV);
	V.y = 0.0;
	V.z = NdotV;

	float A = 0.0;
	float B = 0.0;

	glm::vec3 N = glm::vec3(0.0, 0.0, 1.0);

	for (unsigned int i = 0u; i < samples; ++i) {
		glm::vec2 Xi = Hammersley(i, samples);
		glm::vec3 H = ImportanceSampleGGX(Xi, roughness, N);
		glm::vec3 L = normalize(2.0f * dot(V, H) * H - V);

		float NoL = glm::max(L.z, 0.0f);
		float NoH = glm::max(H.z, 0.0f);
		float VoH = glm::max(dot(V, H), 0.0f);
		float NoV = glm::max(dot(N, V), 0.0f);

		if (NoL > 0.0) {
			float G = GeometrySmith(roughness, NoV, NoL);

			float G_Vis = (G * VoH) / (NoH * NoV);
			float Fc = pow(1.0 - VoH, 5.0);

			A += (1.0 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}

	return glm::vec2(A / float(samples), B / float(samples));
}

void syng::SG_generateBRDFLUT(TextureImage& img, int samples, int size) {
	if (stbi_is_hdr("BRDFLUT.hdr")) {
		int channels;
		uint8_t *data = (uint8_t*) loadRawImage_File("BRDFLUT.hdr", img.format, img.width, img.height, channels);
		img.bytes.resize(img.getSize());
		memcpy(img.bytes.data(), data, img.getSize());
		return;
	};

    img.format = ImageFormat::RGB32F;
    img.width = img.height = size;
    img.bytes.resize(img.getSize());

    // Each pixel has 3 floats → reinterpret as float*
    float* data = reinterpret_cast<float*>(img.bytes.data());

    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            float NoV = (x + 0.5f) / float(img.width);
            float roughness = (y + 0.5f) / float(img.height);

            glm::vec2 brdf = IntegrateBRDF_RG(NoV, roughness, samples);

            int idx = ((img.width - y - 1) * img.width + x) * 3; // 3 floats per pixel
            data[idx + 0] = brdf.x;
            data[idx + 1] = brdf.y;
            data[idx + 2] = 0;
        }
    }
	stbi_write_hdr("BRDFLUT.hdr", size, size, 3, (const float*) data);
}