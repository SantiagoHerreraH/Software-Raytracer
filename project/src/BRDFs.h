#pragma once
#include "Maths.h"

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{;
			return (cd * kd)/M_PI;
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			return (kd * cd)/M_PI;
		}

		/**
		 * \brief todo
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(
			float ks, float exp,
			const Vector3& incomingNormalizedLightDirection, const Vector3& viewDirection, const Vector3& surfaceNormal)
		{

			Vector3 reflectedLightDirection{ 
				incomingNormalizedLightDirection  -
				((2.f * Vector3::Dot(incomingNormalizedLightDirection, surfaceNormal)) * surfaceNormal)};

			float phongSpecularReflection{ ks * std::pow(Vector3::Dot(reflectedLightDirection, viewDirection), exp) };

			return ColorRGB{ phongSpecularReflection , phongSpecularReflection , phongSpecularReflection };
		}

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& normalizedHalfVector, const Vector3& outgoingViewDirection, const ColorRGB& percentageOfReflection)
		{
			ColorRGB fresnelReflection{};
			float calculation{ std::powf(1. - Vector3::Dot(normalizedHalfVector, outgoingViewDirection), 5. )};
			fresnelReflection.r = percentageOfReflection.r + (1.f - percentageOfReflection.r ) * calculation; 
			fresnelReflection.g = percentageOfReflection.g + (1.f - percentageOfReflection.g ) * calculation; 
			fresnelReflection.b = percentageOfReflection.b + (1.f - percentageOfReflection.b ) * calculation;
			return fresnelReflection;
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& surfaceNormal, const Vector3& normalizedHalfVector, float roughness)
		{
			float alpha{ std::powf(roughness, 4.f) };
			float calculation{std::powf(Vector3::Dot(surfaceNormal, normalizedHalfVector), 2.f) * (alpha - 1.f) + 1.f};

			return alpha  / (M_PI * std::powf(calculation, 2.f));
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(const Vector3& surfaceNormal, const Vector3& viewVector, float roughness)
		{
			float kDirect{ (std::powf(std::powf(roughness, 2) + 1, 2)) / 8.f};
			float dot{ std::clamp(Vector3::Dot(surfaceNormal, viewVector), 0.f, 1.f )};

			return dot / (dot * (1 - kDirect) + kDirect);
		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& surfaceNormal, const Vector3& viewVector, const Vector3& lightVector, float roughness)
		{
			return 
				BRDF::GeometryFunction_SchlickGGX(surfaceNormal, viewVector , roughness) *
				BRDF::GeometryFunction_SchlickGGX(surfaceNormal, lightVector, roughness);
		}

	}
}