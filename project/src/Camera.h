#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Maths.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			Origin{ _origin },
			FovAngle{ _fovAngle } {}


		Vector3 Origin{};
		float FovAngle{ 90.f };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		Matrix cameraToWorld{};

		float DistanceFromViewerToCamera{ 1 };

		float MaxTranslationPerSecond{ 100 };//m/s
		float MaxRotationPerSecond{ 90.f * TO_RADIANS};//radians/s

		int ScreenWidth{};
		int ScreenHeight{};

		float TotalPitch{ 0.f };//radians
		float TotalYaw{ 0.f };//radians

	private:

		
		bool m_CanMove{ false };

	private:

		void SetMovement(bool canMove) {

			if (m_CanMove && !canMove)
			{
				m_CanMove = canMove;

				SDL_SetRelativeMouseMode(SDL_FALSE);

			}
			else if (!m_CanMove && canMove)
			{
				m_CanMove = canMove;

				SDL_SetRelativeMouseMode(SDL_TRUE);
			}
		}

	public:

		Matrix CalculateCameraToWorld()
		{

			cameraToWorld = Matrix{};
			cameraToWorld *= Matrix::CreateTranslation(Origin);
			cameraToWorld *= Matrix::CreateRotationY(TotalYaw);
			cameraToWorld *= Matrix::CreateRotationX(TotalPitch);

			return cameraToWorld;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			Vector3 localSpaceCameraWorldDelta{};
			float deltaRotationX{}; 
			float deltaRotationY{}; 

			//this is also unreal's check order
			if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) 
			{
				SetMovement(true);
				localSpaceCameraWorldDelta.z = -mouseY;
			}
			else if (mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE))
			{
				SetMovement(true);

				localSpaceCameraWorldDelta.x = mouseX;
				localSpaceCameraWorldDelta.y = -mouseY;
			}
			else if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				SetMovement(true);

				deltaRotationX = (float)mouseY * TO_RADIANS;
				deltaRotationY = (float)mouseX * TO_RADIANS;
			}
			else
			{
				SetMovement(false);
			}

			if (m_CanMove)
			{
				//Keyboard Input
				const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

				if (pKeyboardState[SDL_SCANCODE_W])
				{
					localSpaceCameraWorldDelta.z += MaxTranslationPerSecond;
				}
				if (pKeyboardState[SDL_SCANCODE_S])
				{
					localSpaceCameraWorldDelta.z -= MaxTranslationPerSecond;
				}
				if (pKeyboardState[SDL_SCANCODE_A])
				{
					localSpaceCameraWorldDelta.x -= MaxTranslationPerSecond;
				}
				if (pKeyboardState[SDL_SCANCODE_D])
				{
					localSpaceCameraWorldDelta.x += MaxTranslationPerSecond;
				}
			}

			localSpaceCameraWorldDelta *= deltaTime;

			TotalPitch += std::clamp(deltaRotationX, -MaxRotationPerSecond, MaxRotationPerSecond) * deltaTime;
			TotalYaw   += std::clamp(deltaRotationY, -MaxRotationPerSecond, MaxRotationPerSecond) * deltaTime;

			//Keep rotation within range to avoid overflow or underflow
			TotalPitch = 
				TotalPitch >  PI_2 ? TotalPitch - PI_2 :
				TotalPitch < -PI_2 ? TotalPitch + PI_2 :
				TotalPitch;

			TotalYaw =
				TotalYaw >  PI_2 ? TotalYaw - PI_2 :
				TotalYaw < -PI_2 ? TotalYaw + PI_2 :
				TotalYaw;
			
			Matrix rotation = Matrix::CreateRotationX(TotalPitch);
			rotation *= Matrix::CreateRotationY(TotalYaw);

			right = cameraToWorld.GetAxisX();
			up = cameraToWorld.GetAxisY();
			forward = cameraToWorld.GetAxisZ();

			Origin += rotation.TransformVector(localSpaceCameraWorldDelta); //local space movement to world space

			cameraToWorld = Matrix::CreateTranslation(Origin);
			cameraToWorld *= rotation;

			

		}
	};
}
