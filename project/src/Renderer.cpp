//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	//refactor
	camera.ScreenHeight = m_Height;
	camera.ScreenWidth = m_Width;

	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();
	
	double fieldOfViewScalar{tan((TO_RADIANS * camera.FovAngle)/2.0)};

	Ray currentViewRayCast{};
	Ray currentLightRayCast{};
	HitRecord closestHit{};
	
	float aspectRatio{ m_Width/float(m_Height)};

	Sphere testSphere{ Vector3{0,0,100}, 50, 0 };

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			currentViewRayCast.Origin = camera.Origin;
			currentViewRayCast.max = INFINITY;
			currentViewRayCast.min = 0.f;

			//screen space to normalized device coordinates
			currentViewRayCast.direction.x = (2 * ((px + 0.5) / m_Width)) - 1;
			currentViewRayCast.direction.y = 1 - (2 * (py + 0.5) / m_Height);
			currentViewRayCast.direction.z = 1;

			currentViewRayCast.direction.x *= aspectRatio * fieldOfViewScalar * camera.DistanceFromViewerToCamera;
			currentViewRayCast.direction.y *= fieldOfViewScalar * camera.DistanceFromViewerToCamera;
			
			currentViewRayCast.direction = camera.cameraToWorld.TransformVector(currentViewRayCast.direction);

			currentViewRayCast.direction.Normalize();

			pScene->GetClosestHit(currentViewRayCast, closestHit);

			//Update Color in Buffer
			ColorRGB finalColor{};

			if (closestHit.didHit)
			{
				closestHit.Origin = closestHit.Origin + closestHit.normal * 0.0001f;
				bool lighted{ false };
				float currentRadiance{};
				float lambertCosine{ 0 };

				for (size_t lightIdx = 0; lightIdx < lights.size(); lightIdx++)
				{
					currentLightRayCast.Origin = lights[lightIdx].Origin;
					currentLightRayCast.direction = -LightUtils::GetDirectionToLight(lights[lightIdx], closestHit.Origin);//returns outgoing so have to make it INCOMING to get correct shadow effect on trinagle mesh culling
					currentLightRayCast.max = currentLightRayCast.direction.Magnitude();
					currentLightRayCast.direction.Normalize();
				
					//percentage of light projected in normal direction
					lambertCosine = Vector3::Dot(closestHit.normal, -currentLightRayCast.direction);

					if (lambertCosine >= 0 && !pScene->DoesHit(currentLightRayCast))
					{
						finalColor +=
							LightUtils::GetRadiance(lights[lightIdx], currentLightRayCast.max) *
							materials[closestHit.materialIndex]->Shade(closestHit, -currentLightRayCast.direction, -currentViewRayCast.direction) *
							lambertCosine;
					
					}
					
				}

			}

			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
