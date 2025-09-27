#include "Scene.h"
#include "Utils.h"
#include "Material.h"

namespace dae {

#pragma region Base Scene
	//Initialize Scene with Default Solid Color Material (RED)
	Scene::Scene() :
		m_Materials({ new Material_SolidColor({1,0,0}) })
	{
		m_SphereGeometries.reserve(32);
		m_PlaneGeometries.reserve(32);
		m_TriangleMeshGeometries.reserve(32);
		m_Lights.reserve(32);
	}

	Scene::~Scene()
	{
		for (auto& pMaterial : m_Materials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}

		m_Materials.clear();
	}

	void dae::Scene::GetClosestHit(const Ray& ray, HitRecord& closestHit) const
	{
		const std::vector<dae::Plane>& planes = GetPlaneGeometries();
		const std::vector<dae::Sphere>& spheres = GetSphereGeometries();

		HitRecord currentHit{};
		closestHit.t = INFINITY;
		closestHit.didHit = false;

		for (size_t sphereIdx = 0; sphereIdx < spheres.size(); sphereIdx++)
		{
			if (GeometryUtils::HitTest_Sphere(spheres[sphereIdx], ray, currentHit) && 
				currentHit.t < closestHit.t)
			{
				closestHit = currentHit;
			}
		}

		for (size_t planeIdx = 0; planeIdx < planes.size(); planeIdx++)
		{
			if (GeometryUtils::HitTest_Plane(planes[planeIdx], ray, currentHit) &&
				currentHit.t < closestHit.t)
			{
				closestHit = currentHit;
			}
		}

		for (size_t i = 0; i < m_Triangles.size(); i++)
		{
			if (GeometryUtils::HitTest_Triangle(m_Triangles[i], ray, currentHit) &&
				currentHit.t < closestHit.t)
			{
				closestHit = currentHit;
			}
		}

		for (size_t i = 0; i < m_TriangleMeshGeometries.size(); i++)
		{
			if (GeometryUtils::HitTest_TriangleMesh(m_TriangleMeshGeometries[i], ray, currentHit) &&
				currentHit.t < closestHit.t)
			{
				closestHit = currentHit;
			}
		}
	}

	bool Scene::DoesHit(const Ray& ray) const
	{
		const std::vector<dae::Plane>& planes = GetPlaneGeometries();
		const std::vector<dae::Sphere>& spheres = GetSphereGeometries();

		for (size_t sphereIdx = 0; sphereIdx < spheres.size(); sphereIdx++)
		{
			if (GeometryUtils::HitTest_Sphere(spheres[sphereIdx], ray))
			{
				return true;
			}
		}

		for (size_t planeIdx = 0; planeIdx < planes.size(); planeIdx++)
		{
			if (GeometryUtils::HitTest_Plane(planes[planeIdx], ray))
			{
				return true;
			}
		}

		for (size_t i = 0; i < m_Triangles.size(); i++)
		{
			if (GeometryUtils::HitTest_Triangle(m_Triangles[i], ray))
			{
				return true;
			}
		}

		for (size_t i = 0; i < m_TriangleMeshGeometries.size(); i++)
		{
			if (GeometryUtils::HitTest_TriangleMesh(m_TriangleMeshGeometries[i], ray))
			{
				return true;
			}
		}

		

		return false;
	}

#pragma region Scene Helpers
	Sphere* Scene::AddSphere(const Vector3& Origin, float radius, unsigned char materialIndex)
	{
		Sphere s;
		s.Origin = Origin;
		s.radius = radius;
		s.materialIndex = materialIndex;

		m_SphereGeometries.emplace_back(s);
		return &m_SphereGeometries.back();
	}

	Plane* Scene::AddPlane(const Vector3& Origin, const Vector3& normal, unsigned char materialIndex)
	{
		Plane p;
		p.Origin = Origin;
		p.normal = normal;
		p.materialIndex = materialIndex;

		m_PlaneGeometries.emplace_back(p);
		return &m_PlaneGeometries.back();
	}

	TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex)
	{
		TriangleMesh m{};
		m.cullMode = cullMode;
		m.materialIndex = materialIndex;

		m_TriangleMeshGeometries.emplace_back(m);
		return &m_TriangleMeshGeometries.back();
	}

	Light* Scene::AddPointLight(const Vector3& Origin, float intensity, const ColorRGB& color)
	{
		Light l;
		l.Origin = Origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Point;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
	{
		Light l;
		l.direction = direction;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Directional;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	unsigned char Scene::AddMaterial(Material* pMaterial)
	{
		m_Materials.push_back(pMaterial);
		return static_cast<unsigned char>(m_Materials.size() - 1);
	}
#pragma endregion
#pragma endregion

#pragma region SCENE W1
	void Scene_W1::Initialize()
	{
		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Spheres
		AddSphere({ -25.f, 0.f, 100.f }, 50.f, matId_Solid_Red);
		AddSphere({ 25.f, 0.f, 100.f }, 50.f, matId_Solid_Blue);

		//Plane
		AddPlane({ -75.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 75.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, -75.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 75.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 125.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);
	}
#pragma endregion
	void Scene_W2::Initialize()
	{
		m_Camera.Origin = {0.f, 3.f, -9.f};
		m_Camera.FovAngle = 45.f;

		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });


		//Plane
		AddPlane({-5.f,  0.f,   0.f }, {  1.f,  0.f,   0.f }, matId_Solid_Green);
		AddPlane({ 5.f,  0.f,   0.f }, { -1.f,  0.f,   0.f }, matId_Solid_Green);
		AddPlane({ 0.f,  0.f,   0.f }, {  0.f,  1.f,   0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f,  10.f,  0.f }, {  0.f, -1.f,   0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f,  0.f,  10.f }, {  0.f,  0.f,  -1.f }, matId_Solid_Magenta);

		//Spheres
		AddSphere({ -1.75f,  1.f,  0.f }, 0.75f, matId_Solid_Red);
		AddSphere({  0.f,    1.f,  0.f }, 0.75f, matId_Solid_Blue);
		AddSphere({  1.75f,  1.f,  0.f }, 0.75f, matId_Solid_Red);
		AddSphere({ -1.75,   3.f,  0.f }, 0.75f, matId_Solid_Blue);
		AddSphere({  0.f,    3.f,  0.f }, 0.75f, matId_Solid_Red);
		AddSphere({  1.75f,  3.f,  0.f }, 0.75f, matId_Solid_Blue);

		AddPointLight({ 0.f, 5.f, -5.f }, 70.f, colors::White);
	}
	void Scene_W3::Initialize()
	{
		m_Camera.Origin = { 0.f, 3.f, -9.f };
		m_Camera.FovAngle = 45.f;

		const unsigned char matCT_Grey_RoughMetal    = AddMaterial(new Material_CookTorrence({ 0.972f		, 0.960f	, 0.915f}	, true	, 1.f));
		const unsigned char matCT_Grey_MediumMetal   = AddMaterial(new Material_CookTorrence({ 0.972f		, 0.960f	, 0.915f}	, true	, 0.6f));
		const unsigned char matCT_Grey_SmoothMetal   = AddMaterial(new Material_CookTorrence({ 0.972f		, 0.960f	, 0.915f}	, true	, 0.1f));
		const unsigned char matCT_Grey_RoughPlastic  = AddMaterial(new Material_CookTorrence({ 0.75f		, 0.75f		, 0.75f}	, false	, 1.f));
		const unsigned char matCT_Grey_MediumPlastic = AddMaterial(new Material_CookTorrence({ 0.75f		, 0.75f		, 0.75f}	, false	, 0.6f));
		const unsigned char matCT_Grey_SmoothPlastic = AddMaterial(new Material_CookTorrence({ 0.75f		, 0.75f		, 0.75f}	, false	, 0.1f));

		const unsigned char matLambert_Grey_Blue	 = AddMaterial(new Material_Lambert( {0.49f, 0.57f, 0.57f}, 1.f));

		AddPlane({  0.f,   0.f,   10.f }, {  0.f,   0.f,   -1.f }, matLambert_Grey_Blue);
		AddPlane({  0.f,   0.f,    0.f }, {  0.f,   1.f,    0.f }, matLambert_Grey_Blue);
		AddPlane({  0.f,  10.f,    0.f }, {  0.f,  -1.f,    0.f }, matLambert_Grey_Blue);
		AddPlane({  5.f,   0.f,    0.f }, { -1.f,   0.f,    0.f }, matLambert_Grey_Blue);
		AddPlane({ -5.f,   0.f,    0.f }, {  1.f,   0.f,    0.f }, matLambert_Grey_Blue);

		AddSphere({ -1.75f,		1.f,  0.f }, 0.75f,		matCT_Grey_RoughMetal   );
		AddSphere({  0.f,		1.f,  0.f }, 0.75f,		matCT_Grey_MediumMetal  );
		AddSphere({  1.75f,		1.f,  0.f }, 0.75f,		matCT_Grey_SmoothMetal  );
		AddSphere({ -1.75f,		3.f,  0.f }, 0.75f,		matCT_Grey_RoughPlastic );
		AddSphere({  0.f,		3.f,  0.f }, 0.75f,		matCT_Grey_MediumPlastic);
		AddSphere({  1.75f,		3.f,  0.f }, 0.75f,		matCT_Grey_SmoothPlastic);

		AddPointLight({  0.f,  5.f,   5.f },  50.f, ColorRGB{1.f,		 0.61f,		0.45f});
		AddPointLight({ -2.5f, 5.f,  -5.f },  70.f, ColorRGB{1.f,		 0.8f,		0.45f});
		AddPointLight({  2.5f, 2.5f, -5.f },  50.f, ColorRGB{0.34f,		 0.47f,		0.68f});
	}
	void Scene_W4::Initialize()
	{
		m_Camera.Origin = { 0.f, 1.f, -5.f };
		m_Camera.FovAngle = 45.f;
		////m_Camera.Origin = { 0.f, 1.f, 4.f };
		////m_Camera.TotalYaw = M_PI;

		sceneName = "Reference Scene";
		m_Camera.Origin = { 0, 3, -9 };
		m_Camera.FovAngle = 45.f;

		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, true, .6f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, true, .1f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, true, .05f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, false, .6f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, false, .1f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, false, .05f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, .57f, .57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matLambert_GrayBlue);  // BACK
		AddPlane({ 0.f, -1.f, 0.f }, { 0.f, 1.f, 0.f }, matLambert_GrayBlue);   // BOTTOM
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matLambert_GrayBlue);   // TOP
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matLambert_GrayBlue);   // RIGHT
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matLambert_GrayBlue);   // LEFT

		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);

		AddPointLight( Vector3{  0.0f , 5.0f  ,  5.0f }, 50.f, ColorRGB{  1.00f , 0.61f  , 0.45f });
		AddPointLight( Vector3{ -2.5f , 5.0f  , -5.0f }, 70.f, ColorRGB{  1.00f , 0.80f  , 0.45f });
		AddPointLight( Vector3{  2.5f , 2.5f  , -5.0f }, 50.f, ColorRGB{  0.34f , 0.47f  , 0.68f });

		// CW Winding Order!
		const Triangle baseTriangle = { Vector3{-.75f, 1.5f, 0.f}, Vector3{.75f, 0.f, 0.f}, Vector3{-.75f, 0.f, 0.f} };
		m_Meshes.resize(3);
		m_Meshes[0] = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		m_Meshes[0]->AppendTriangle(baseTriangle, true);
		m_Meshes[0]->Translate(Vector3{ -1.75f, 4.5f, 0.f });
		m_Meshes[0]->UpdateTransforms();

		m_Meshes[1] = AddTriangleMesh(TriangleCullMode::FrontFaceCulling, matLambert_White);
		m_Meshes[1]->AppendTriangle(baseTriangle, true);
		m_Meshes[1]->Translate(Vector3{ 0.f, 4.5f, 0.f });
		m_Meshes[1]->UpdateTransforms();

		m_Meshes[2] = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		m_Meshes[2]->AppendTriangle(baseTriangle, true);
		m_Meshes[2]->Translate(Vector3{ 1.75f, 4.5f, 0.f });
		m_Meshes[2]->UpdateTransforms();
	}

	void Scene_W4_BunnyScene::Initialize()
	{

		sceneName = "Bunny Scene";
		m_Camera.Origin = { 0.f, 1.f, -5.f };
		m_Camera.FovAngle = 45.f;

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert{ {0.49f, 0.57f, 0.57f}, 1.f });
		const auto matLambert_White = AddMaterial(new Material_Lambert{ colors::White, 1.f });

		AddPlane(Vector3{ 0.f ,  0.f , 10.f }, Vector3{ 0.f ,  0.f ,  -1.f }, matLambert_GrayBlue);
		AddPlane(Vector3{ 0.f ,  0.f ,  0.f }, Vector3{ 0.f ,  1.f ,   0.f }, matLambert_GrayBlue);
		AddPlane(Vector3{ 0.f , 10.f ,  0.f }, Vector3{ 0.f , -1.f ,   0.f }, matLambert_GrayBlue);
		AddPlane(Vector3{ 5.f ,  0.f ,  0.f }, Vector3{ -1.f ,  0.f ,   0.f }, matLambert_GrayBlue);
		AddPlane(Vector3{ -5.f ,  0.f ,  0.f }, Vector3{ 1.f ,  0.f ,   0.f }, matLambert_GrayBlue);

		p_Mesh = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		Utils::ParseOBJ("Resources/lowpoly_bunny.obj",
			p_Mesh->positions,
			p_Mesh->normals,
			p_Mesh->indices);

		//p_Mesh->Scale({ 2.f, 2.f, 2.f });
		//p_Mesh->Translate({ 0.f, 1.f, 0.f });

		p_Mesh->UpdateTransforms();



		AddPointLight(Vector3{ 0.0f , 5.0f  ,  5.0f }, 50.f, ColorRGB{ 1.00f , 0.61f  , 0.45f });
		AddPointLight(Vector3{ -2.5f , 5.0f  , -5.0f }, 70.f, ColorRGB{ 1.00f , 0.80f  , 0.45f });
		AddPointLight(Vector3{ 2.5f , 2.5f  , -5.0f }, 50.f, ColorRGB{ 0.34f , 0.47f  , 0.68f });




		////m_Camera.Origin = { 0.f, 1.f, 4.f };
		////m_Camera.TotalYaw = M_PI;
		//m_Camera.FovAngle = 45.f;
		//
		//const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert{ {0.49f, 0.57f, 0.57f}, 1.f });
		//const auto matLambert_White		= AddMaterial(new Material_Lambert{colors::White, 1.f});
		//
		//AddPlane(Vector3{  0.f ,  0.f , 10.f }, Vector3{  0.f ,  0.f ,  -1.f }, matLambert_GrayBlue);
		//AddPlane(Vector3{  0.f ,  0.f ,  0.f }, Vector3{  0.f ,  1.f ,   0.f }, matLambert_GrayBlue);
		//AddPlane(Vector3{  0.f , 10.f ,  0.f }, Vector3{  0.f , -1.f ,   0.f }, matLambert_GrayBlue);
		//AddPlane(Vector3{  5.f ,  0.f ,  0.f }, Vector3{ -1.f ,  0.f ,   0.f }, matLambert_GrayBlue);
		//AddPlane(Vector3{ -5.f ,  0.f ,  0.f }, Vector3{  1.f ,  0.f ,   0.f }, matLambert_GrayBlue);

		//TEST TRIANGLE
		//auto triangle = Triangle{
		//	Vector3{-0.75f, 0.5f, 0.f},
		//	Vector3{-0.75f, 2.0f, 0.f},
		//	Vector3{ 0.75f, 0.5f, 0.f}
		//};
		//
		//triangle.cullMode = TriangleCullMode::BackFaceCulling; //does work but shadows are inverted
		//triangle.materialIndex = matLambert_White;
		//
		//m_Triangles.emplace_back(triangle);


		//TEST TRIANGLE MESH
		//auto triangleMesh = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		//triangleMesh->positions = {
		//
		//	Vector3{ -.75f, -1.00f, 0.00f },
		//	Vector3{ -.75f,  1.00f, 0.00f },
		//	Vector3{ 0.75f,  1.00f, 1.00f },
		//	Vector3{ 0.75f, -1.00f, 0.00f }
		//
		//};
		//triangleMesh->indices = {
		//	0,1,2,
		//	0,2,3
		//
		//};
		//
		//triangleMesh->CalculateNormals();
		//
		//triangleMesh->Translate({ 0,1.5f, 0 });
		//triangleMesh->RotateY(45);
		//
		//triangleMesh->UpdateTransforms();

		//TEST CUBE
		//p_Mesh = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		//Utils::ParseOBJ("Resources/lowpoly_bunny.obj", 
		//	p_Mesh->positions,
		//	p_Mesh->normals,
		//	p_Mesh->indices);
		//
		//p_Mesh->Scale({ 0.7f, 0.7f, 0.7f });
		//p_Mesh->Translate({ 0.f, 1.f, 0.f});
		//
		//p_Mesh->UpdateTransforms();
		
	}
}
