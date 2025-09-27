#pragma once
#include <fstream>
#include "Maths.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			Vector3 deltaVector{ sphere.Origin - ray.Origin };

			float deltaVectorMagnitude = deltaVector.Magnitude();

			//if ((deltaVectorMagnitude - sphere.radius) > ray.max) {
			//	return false;
			//}

			float projectedVectorLength{ Vector3::Dot(ray.direction, deltaVector) };
			float orthogonalVectorLength{ sqrtf(powf(deltaVectorMagnitude, 2) - powf(projectedVectorLength, 2)) };

			float distanceFromMidPointToHitPoints{ sqrtf( powf(sphere.radius, 2) - powf(orthogonalVectorLength, 2))};

			//You just need one for now, I guess
			float distanceToHitPoint{ (projectedVectorLength - distanceFromMidPointToHitPoints) };
			Vector3 possibleHitPoint{ ray.Origin + (ray.direction * distanceToHitPoint) };
			
			Vector3 hitNormal{ possibleHitPoint - sphere.Origin };

			//bool didHit{
			//	distanceToHitPoint <= ray.max &&
			//	distanceToHitPoint >= ray.min &&
			//	deltaVectorMagnitude > sphere.radius &&
			//	dae::AreEqual(hitNormal.Magnitude(), sphere.radius, 0.01f) };

			bool didHit{
				distanceToHitPoint <= ray.max &&
				distanceToHitPoint >= ray.min &&
				dae::AreEqual(hitNormal.Magnitude(), sphere.radius, 0.01f) };

			if (!ignoreHitRecord)
			{
				hitRecord.didHit = didHit;
				hitRecord.materialIndex = sphere.materialIndex;
				hitRecord.normal = hitNormal.Normalized();
				hitRecord.Origin = possibleHitPoint;
				hitRecord.t = distanceToHitPoint;
			}

			return didHit;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{

			float distanceToPlane
			{ 
				Vector3::Dot((plane.Origin - ray.Origin), plane.normal)/ 
				Vector3::Dot(ray.direction, plane.normal)
			};

			if (distanceToPlane < ray.min || distanceToPlane > ray.max)
			{
				return false;
			}

			Vector3 possibleHitPoint{ ray.Origin + (ray.direction * distanceToPlane)};

			bool didHit{dae::AreEqual(Vector3::Dot((possibleHitPoint - plane.Origin), plane.normal), 0, 0.01)};

			if (!ignoreHitRecord)
			{
				hitRecord.didHit = didHit;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.normal = plane.normal; // -ray.direction;
				hitRecord.Origin = possibleHitPoint;
				hitRecord.t = distanceToPlane;
			}

			return didHit;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest

		
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			
			const Vector3 a{ triangle.v1 - triangle.v0 };
			const Vector3 b{ triangle.v2 - triangle.v0 };
			Vector3 normal{ Vector3::Cross(a, b)};
			normal.Normalize();

			if (triangle.cullMode == TriangleCullMode::BackFaceCulling && 
				Vector3::Dot(ray.direction, normal) > 0)
			{
				if (!ignoreHitRecord)
				{
					hitRecord.didHit = false;
				}
				return false;
			}
			else if (
				triangle.cullMode == TriangleCullMode::FrontFaceCulling &&
				Vector3::Dot(ray.direction, normal) < 0)
			{
				if (!ignoreHitRecord)
				{
					hitRecord.didHit = false;
				}
				return false;
			}
		
			if (dae::AreEqual(Vector3::Dot(normal, ray.direction), 0, 0.001f )) {
				return false;
			}
		
			Vector3 l = triangle.v0 - ray.Origin;
		
			float t = Vector3::Dot(l, normal) / Vector3::Dot(ray.direction, normal);
		
			if (t < ray.min || t > ray.max)
			{
				if (!ignoreHitRecord)
				{
					hitRecord.didHit = false;
				}
				return false;
			}
		
			Vector3 hitPoint = ray.Origin + ray.direction * t;
		
			Vector3 e{ triangle.v1 - triangle.v0 };
			Vector3 p{ hitPoint - triangle.v0 };
		
			if (Vector3::Dot(Vector3::Cross(e, p), normal) < 0)
			{
				if (!ignoreHitRecord)
				{
					hitRecord.didHit = false;
				}
				return false;
			}
		
			e =  triangle.v2 - triangle.v1 ;
			p =  hitPoint - triangle.v1 ;
		
			if (Vector3::Dot(Vector3::Cross(e, p), normal) < 0)
			{
				if (!ignoreHitRecord)
				{
					hitRecord.didHit = false;
				}
				return false;
			}
		
			e =  triangle.v0 - triangle.v2 ;
			p =  hitPoint - triangle.v2 ;
		
			if (Vector3::Dot(Vector3::Cross(e, p), normal) < 0)
			{
				if (!ignoreHitRecord)
				{
					hitRecord.didHit = false;
				}
				return false;
			}
		
			
		
			if (!ignoreHitRecord)
			{
				hitRecord.didHit = true;
				hitRecord.t = t;
				hitRecord.Origin = hitPoint;
				hitRecord.normal = triangle.normal;
				hitRecord.materialIndex = triangle.materialIndex;
			}
		
			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			Triangle currentTriangle{};
			currentTriangle.cullMode = mesh.cullMode;
			currentTriangle.materialIndex = mesh.materialIndex;
		
			bool hitAnyTriangle = false;
			float closestT = INFINITY;
			HitRecord tempHitRecord;
		
			int currentNormalIdx = 0;
			for (size_t i = 0; i < mesh.indices.size(); i += 3)
			{
				
				currentTriangle.normal = mesh.transformedNormals[currentNormalIdx];
				currentTriangle.v0 = mesh.transformedPositions[mesh.indices[i]];
				currentTriangle.v1 = mesh.transformedPositions[mesh.indices[i + 1]];
				currentTriangle.v2 = mesh.transformedPositions[mesh.indices[i + 2]];
				++currentNormalIdx;
		
				// Use a temporary hit record to track individual triangle intersections
				if (HitTest_Triangle(currentTriangle, ray, tempHitRecord, ignoreHitRecord))
				{
					hitAnyTriangle = true;
		
					if (tempHitRecord.t < closestT)
					{
						closestT = tempHitRecord.t;
		
						// Update the main hitRecord only if ignoreHitRecord is false
						if (!ignoreHitRecord)
						{
							hitRecord = tempHitRecord;
						}
					}
				}
			}
		
			return hitAnyTriangle;
		}

		//inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		//{
		//	HitRecord closestHit{};
		//	closestHit.didHit = false;
		//	closestHit.t = FLT_MAX;
		//	HitRecord temp{};
		//	for (int idx{}; idx < mesh.indices.size() / 3; ++idx)
		//	{
		//		Triangle curTriangle{ mesh.transformedPositions[mesh.indices[(idx * 3)]], mesh.transformedPositions[mesh.indices[(idx * 3) + 1]],
		//			mesh.transformedPositions[mesh.indices[(idx * 3) + 2]], mesh.normals[idx] };
		//		curTriangle.cullMode = mesh.cullMode;
		//		curTriangle.materialIndex = mesh.materialIndex;
		//		curTriangle.normal = mesh.transformedNormals[idx];
		//		if (HitTest_Triangle(curTriangle, ray, temp) == true)
		//		{
		//			if (ignoreHitRecord == true)
		//			{
		//				//hitRecord = temp;
		//				return true;
		//			}
		//			else
		//			{
		//				if (temp.t < closestHit.t)
		//				{
		//					closestHit = temp;
		//				}
		//			}
		//		}
		//	}

		//	if (ignoreHitRecord == true)
		//	{
		//		return false;
		//	}
		//	else
		//	{
		//		hitRecord = closestHit;
		//		return hitRecord.didHit;
		//	}
		//}


		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			Triangle currentTriangle{};
			currentTriangle.cullMode = mesh.cullMode;
			currentTriangle.materialIndex = mesh.materialIndex;
			HitRecord tempHitRecord;

			for (size_t i = 0; i < mesh.indices.size(); i += 3)
			{
				currentTriangle.normal = mesh.transformedNormals[mesh.indices[i]];
				currentTriangle.v0 = mesh.transformedPositions[mesh.indices[i]];
				currentTriangle.v1 = mesh.transformedPositions[mesh.indices[i + 1]];
				currentTriangle.v2 = mesh.transformedPositions[mesh.indices[i + 2]];

				// Use a temporary hit record to track individual triangle intersections
				if (HitTest_Triangle(currentTriangle, ray, tempHitRecord, true))
				{
					return true;
				}
			}

			return false;
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 Origin)
		{
			Vector3 direction{};

			switch (light.type)
			{
			case LightType::Point:
				direction = light.Origin - Origin;
				break;
			case LightType::Directional:
				direction = -light.direction;
				break;
			default:
				throw std::runtime_error("GetDirectionToLight - Invalid Light Type");
				break;
			}
			return direction;
		}

		inline ColorRGB GetRadiance(const Light& light, float distanceFromLightToTarget)
		{
			float radiance{};

			switch (light.type)
			{
			case LightType::Point:
				radiance = light.intensity / (std::powf(distanceFromLightToTarget, 2)); //total power distributed over total sphere area
				break;
			case LightType::Directional:
				radiance = light.intensity;
				break;
			default:
				throw std::runtime_error("GetRadiance - Invalid Light Type");
				break;
			}
			return light.color * radiance;
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof())
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if (std::isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (std::isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}