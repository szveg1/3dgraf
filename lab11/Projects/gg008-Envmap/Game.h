#pragma once
#include "App/App.h"
#include "Mesh/InputBinder.h"
#include "Mesh/Shaded.h"
#include "Cam/FirstPerson.h"
#include "Math/math.h"
#include <vector>

GG_SUBCLASS(Game, Egg::App)
	Egg::Mesh::InputBinderP inputBinder;	//< Input layout manager.
	Egg::Mesh::ShadedP shadedMesh;			//< Giraffe mesh.
	Egg::Mesh::ShadedP backgroundMesh;		//< Background quad mesh.

	Egg::Cam::FirstPersonP firstPersonCam;	//< Camera.
	com_ptr<ID3D11Buffer> perObjectConstantBuffer;	//< Constant buffer for transformation matrices.
	com_ptr<ID3D11Buffer> perFrameConstantBuffer;	//< Constant buffer for transformation matrices.
	com_ptr<ID3D11Texture2D> texture;				//< Giraffe pattern texture.

	//LABTODO: Particle class
	struct Particle {
		Egg::Math::float3 position;
		Egg::Math::float3 velocity;
		float lifespan;
		float age;
	public:
		void reborn() {
			using namespace Egg::Math;
			position =
				float3::random(-1, 1);
			velocity = position * 5;
			age = 0;
			lifespan = float1::random(2, 5);
		}
		Particle() { reborn(); }

		void move(float dt) {
			position += velocity * dt;
			age += dt;
			if (age > lifespan) reborn();
		}
	};


	//LABTODO: particle and billboard resources
	std::vector<Particle> particles;
	Egg::Mesh::Shaded::P fireBillboardSet;
	com_ptr<ID3D11Buffer> perSwapChainConstantBuffer;

	com_ptr<ID3D11BlendState> blendState;
	com_ptr<ID3D11DepthStencilState> depthStencilState;


public:
	Game(com_ptr<ID3D11Device> device);				//< Constructor.
	~Game(void);													//< Destructor.

	HRESULT createResources();		//< Event handler invoked on device creation. Create all resources here.
	HRESULT releaseResources();		//< Event handler invoked on device destruction. Release all resources here.

	void render(com_ptr<ID3D11DeviceContext> context);	//< Event handler for frame render. Draw here.
	void animate(float dt, float t);									//< Event handler for frame move.
	bool processMessage(HWND hWnd,										//< Event handler for Windows messages. Handle keypresses here.
		UINT uMsg, WPARAM wParam, LPARAM lParam);

GG_ENDCLASS
