#include "ColoredTriangleDemo.h"
#include "Game.h"
#include "GameException.h"
#include "Utility.h"
#include "VertexDeclarations.h"
#include "ColorHelper.h"
#include <boost/algorithm/string.hpp>
#include <WICTextureLoader.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <Mouse.h>
#include <Keyboard.h>

#define OBJL_CONSOLE_OUTPUT
namespace Rendering
{
	RTTI_DEFINITIONS(ColoredTriangleDemo)

	ColoredTriangleDemo::ColoredTriangleDemo(Game& game, Camera& camera)
	: DrawableGameComponent(game, camera), mVertexShader(nullptr), mInputLayout(nullptr), mPixelShader(nullptr), pMouse(nullptr)
	{
	}

	ColoredTriangleDemo::~ColoredTriangleDemo()
	{
		for (int i = 0; i < v_vertexBuffer.size(); ++i){
			ReleaseObject(v_vertexBuffer[i]);
		}
		for (int i = 0; i < v_indexBuffer.size(); ++i){
			ReleaseObject(v_indexBuffer[i]);
		}
		for (int j = 0; j < v_texture2dSRV.size(); ++j){
			ReleaseObject(v_objMaterialCBuffer[j]);
		}
		
		ReleaseObject(mPixelShader)
		ReleaseObject(mInputLayout)
		ReleaseObject(mConstantBuffer)
		ReleaseObject(textureSamplerStateObject)
	}

	//Create mvp constant buffer
	ID3D11Buffer* ColoredTriangleDemo::create_mvp_cbuffer(){
		ID3D11Buffer* mConstantBuffer;

		//Create World Matrix
		XMMATRIX world = XMMatrixIdentity();

		//Create View matrices
		XMVECTOR eyepos = XMVectorSet(2.f, 2.f, -2.f, 0.f);
		XMVECTOR lookat = XMVectorSet(0.f, 0.f, 0.f, 0.f);
		XMVECTOR up = XMVectorSet(0.f, 1.f, 0.f, 0.f);
		XMMATRIX view = XMMatrixLookAtLH(eyepos, lookat, up);

		//Create Projection matrix
		float fovDegrees = 90.f;
		float fovRadians = (float)(fovDegrees / 360.f) * 3.14f;
		float aspectRatio = (float)(1024.f / 768.f);
		float nearz = 0.1f;
		float farz = 1000.f;
		XMMATRIX projection = XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearz, farz);

		//Create View Projection Matrix
		XMMATRIX viewProjections = world * view * projection;
		viewProjections = XMMatrixTranspose(viewProjections);

		//Create Constant buffer desc
		D3D11_BUFFER_DESC constantDesc;
		ZeroMemory(&constantDesc, sizeof(constantDesc));
		constantDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantDesc.ByteWidth = sizeof(viewProjections);
		constantDesc.Usage = D3D11_USAGE_DYNAMIC;
		constantDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		//Create subresource data for CB
		D3D11_SUBRESOURCE_DATA cbData;
		ZeroMemory(&cbData, sizeof(cbData));
		cbData.pSysMem = &viewProjections;

		HRESULT cb = mGame->Direct3DDevice()->CreateBuffer(&constantDesc, &cbData, &mConstantBuffer);
		if (FAILED(cb)){
			std::cout << "Constant Buffer Creation Failed " << cb << std::endl;
			exit;
		}
		return mConstantBuffer;
	}

	//Create shader Sampler
	ID3D11SamplerState* ColoredTriangleDemo::create_shader_sampler(){
		ID3D11SamplerState* textureSamplerStateObject;

		//Create SSO
		D3D11_SAMPLER_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		textureDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		textureDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		textureDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		textureDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		textureDesc.MinLOD = 0;
		textureDesc.MaxLOD = D3D11_FLOAT32_MAX;

		HRESULT sso = mGame->Direct3DDevice()->CreateSamplerState(&textureDesc, &textureSamplerStateObject);
		if (FAILED(sso)){
			std::cout << "Sampler State Creation Failed " << sso << std::endl;
			exit;
		}
		return textureSamplerStateObject;
	}

	//Mouse and keyboard controls to update camera location
	//This routine updates the mvp matrix by reading in mouse and keyboard movements
	void ColoredTriangleDemo::update_camera_loc(){
	
		//Get key presses. Q means +z, E means -Z. Sensitivity is 1/50. R is for reset
		float x = pKeyboard->IsKeyDown(DIKEYBOARD_D) ? (this->mouse_loc.x + 0.02f) :
				  pKeyboard->IsKeyDown(DIKEYBOARD_A) ? (this->mouse_loc.x - 0.02f) : 
				  pKeyboard->IsKeyDown(DIKEYBOARD_R) ? (0.f)					   : this->mouse_loc.x;
		float y = pKeyboard->IsKeyDown(DIKEYBOARD_S) ? (this->mouse_loc.y - 0.02f) :
				  pKeyboard->IsKeyDown(DIKEYBOARD_W) ? (this->mouse_loc.y + 0.02f) : 
				  pKeyboard->IsKeyDown(DIKEYBOARD_R) ? (0.f)					   : this->mouse_loc.y;
		float z = pKeyboard->IsKeyDown(DIKEYBOARD_Q) ? (this->mouse_loc.z + 0.02f) :
				  pKeyboard->IsKeyDown(DIKEYBOARD_E) ? (this->mouse_loc.z - 0.02f) : 
				  pKeyboard->IsKeyDown(DIKEYBOARD_R) ? (-2.f)					   : this->mouse_loc.z;

		//Save mouse coordinates
		this->mouse_loc.x = x;
		this->mouse_loc.y = y;
		this->mouse_loc.z = z;

		//Create World Matrix
		XMMATRIX world = XMMatrixIdentity();
		//Create View matrices
		XMVECTOR eyepos = XMVectorSet(x, y, z, 0.f);
		XMVECTOR lookat = XMVectorSet(0.f, 0.f, 0.f, 0.f);
		XMVECTOR up = XMVectorSet(0.f, 1.f, 0.f, 0.f);
		XMMATRIX view = XMMatrixLookAtLH(eyepos, lookat, up);

		//Create Projection matrix
		float fovDegrees = 90.f;
		float fovRadians = (float)(fovDegrees / 360.f) * 3.14f;
		float aspectRatio = (float)(1024.f / 768.f);
		float nearz = 0.1f;
		float farz = 1000.f;
		XMMATRIX projection = XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearz, farz);

		//Create View Projection Matrix
		XMMATRIX viewProjections = world * view * projection;
		viewProjections = XMMatrixTranspose(viewProjections);

		//Create Mapped subresource
		D3D11_MAPPED_SUBRESOURCE newData;
		ZeroMemory(&newData, sizeof(D3D11_MAPPED_SUBRESOURCE));
		newData.RowPitch = sizeof(viewProjections);
		newData.DepthPitch = 1;

		//Update subresource data
		mGame->Direct3DDeviceContext()->Map(mConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &newData);
		memcpy(newData.pData, &viewProjections, sizeof(viewProjections));
		mGame->Direct3DDeviceContext()->Unmap(mConstantBuffer, 0);

	}//End: Update camera location

	//Load mesh from file using obj wavefront object format
	//Function implementations
	objl::Loader ColoredTriangleDemo::load_mesh_from_file(std::string fileName){
		//Initialize loader class
		objl::Loader meshLoader;

		//Load obj file
		bool isLoaded = meshLoader.LoadFile(fileName);
		if (isLoaded == FALSE){
			std::cout << "Error Mesh cannot be loaded" << std::endl;
			exit;
		}
		return meshLoader;
	}//End: load_mesh_from_file()

	//Load vertex Buffer from obj file
	std::vector<ID3D11Buffer*> ColoredTriangleDemo::load_vertex_buffer_from_file(objl::Loader meshInfo){
		ID3D11Buffer* vertexBuffer;
		std::vector<ID3D11Buffer*> v_vertexBuffer;
		std::vector<VertexPositionTextureNormalMaterialId> vertices;
		VertexPositionTextureNormalMaterialId currentVertex;
		objl::Mesh currentMesh;

		//First get the list of meshes from meshInfo
		for (int i = 0; i < meshInfo.LoadedMeshes.size(); ++i){
			//Set current Mesh
			currentMesh = meshInfo.LoadedMeshes[i];
			//Clear vector
			vertices = {};

			//Iterate through vertices
			for (int j = 0; j < currentMesh.Vertices.size(); ++j){
				currentVertex =
				{ XMFLOAT4(currentMesh.Vertices[j].Position.X,
				currentMesh.Vertices[j].Position.Y,
				currentMesh.Vertices[j].Position.Z, 1.f),
				XMFLOAT2(currentMesh.Vertices[j].TextureCoordinate.X,
				currentMesh.Vertices[j].TextureCoordinate.Y),
				XMFLOAT3(currentMesh.Vertices[j].Normal.X,
				currentMesh.Vertices[j].Normal.Y,
				currentMesh.Vertices[j].Normal.Z) };

				vertices.push_back(currentVertex);

			}//For: Vertex Iteration
			//Build Vertex Buffer
			D3D11_BUFFER_DESC vertexBufferDesc;
			D3D11_SUBRESOURCE_DATA vertexBufferData;

			ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
			vertexBufferDesc.ByteWidth = vertices.size() * sizeof(VertexPositionTextureNormalMaterialId);
			vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

			ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
			vertexBufferData.pSysMem = &vertices[0];

			HRESULT vb = mGame->Direct3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertexBuffer);

			if (FAILED(vb)){
				std::cout << "Vertex Buffer Creation Failed" << std::endl;
				exit;
			}

			//Push vertexBuffer to vector
			v_vertexBuffer.push_back(vertexBuffer);
		}//For: Mesh iteration

		
		return v_vertexBuffer;
	}//End: load_vertex_buffer_from_file()

	//Load index buffer from file
	std::vector<ID3D11Buffer*> ColoredTriangleDemo::load_index_buffer_from_file(objl::Loader meshInfo){
		ID3D11Buffer* mIndexBuffer;
		std::vector<ID3D11Buffer*> v_indexBuffer;
		std::vector<UINT> indices;
		objl::Mesh currentMesh;

		for (int i = 0; i < meshInfo.LoadedMeshes.size(); ++i){
			currentMesh = meshInfo.LoadedMeshes[i];
			indices = {};	//Empty the vector
			for (int j = 0; j < currentMesh.Indices.size(); ++j){
				indices.push_back(currentMesh.Indices[j]);
			}
			//Build an index buffer and push it to vector
			D3D11_BUFFER_DESC indexBufDesc;
			ZeroMemory(&indexBufDesc, sizeof(indexBufDesc));
			indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			indexBufDesc.ByteWidth = sizeof(UINT) * indices.size();
			indexBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			indexBufDesc.Usage = D3D11_USAGE_DYNAMIC;
			
			//Create subresource data
			D3D11_SUBRESOURCE_DATA indexBufData;
			ZeroMemory(&indexBufData, sizeof(indexBufData));
			indexBufData.pSysMem = &indices[0];

			HRESULT ib = mGame->Direct3DDevice()->CreateBuffer(&indexBufDesc, &indexBufData, &mIndexBuffer);
			if (FAILED(ib)){
				std::cout << "Error. Index Buffer Creation Failed " << ib << std::endl;
				exit;
			}
			//Push to vector
			v_indexBuffer.push_back(mIndexBuffer);
		}
		return v_indexBuffer;

	}//End: Load_index_buffer_from_file()

	//Load materials data
	std::vector<ColoredTriangleDemo::t_objectMaterial> ColoredTriangleDemo::load_materials(objl::Loader meshInfo){
		//Iterate through the meshes and map materials
		objl::Mesh currentMesh;
		std::string materialName;
		t_objectMaterial objMaterial;

		for (int i = 0; i < meshInfo.LoadedMeshes.size(); ++i){
			currentMesh = meshInfo.LoadedMeshes[i];
			materialName = currentMesh.MeshMaterial.name;
			objMaterial = {};	//Empty the struct

			//Push material params
			objMaterial.kAmbient = { currentMesh.MeshMaterial.Ka.X, currentMesh.MeshMaterial.Ka.Y, currentMesh.MeshMaterial.Ka.Z, 1.f };
			objMaterial.kDiffuse = { currentMesh.MeshMaterial.Kd.X, currentMesh.MeshMaterial.Kd.Y, currentMesh.MeshMaterial.Kd.Z, 1.f };
			objMaterial.kSpecular = { currentMesh.MeshMaterial.Ks.X, currentMesh.MeshMaterial.Ks.Y, currentMesh.MeshMaterial.Ks.Z, 1.f };
			objMaterial.ExpSpecular = { currentMesh.MeshMaterial.Ns, currentMesh.MeshMaterial.Ns, currentMesh.MeshMaterial.Ns, 1.f };
			objMaterial.uses_Ka = currentMesh.MeshMaterial.map_Ka.empty() ? 0 : 1;
			objMaterial.uses_Kd = currentMesh.MeshMaterial.map_Kd.empty() ? 0 : 1;
			objMaterial.uses_Ks = currentMesh.MeshMaterial.map_Ks.empty() ? 0 : 1;

			v_materials.push_back(objMaterial);
		}//For: Build material map
		return v_materials;
	}

	//Load textures from file
	std::vector<ColoredTriangleDemo::t_texture2dSRV> ColoredTriangleDemo::load_textures_from_file(objl::Loader meshInfo){
		ID3D11ShaderResourceView* srv_ka; std::string srv_ka_name;
		ID3D11ShaderResourceView* srv_kd; std::string srv_kd_name;
		ID3D11ShaderResourceView* srv_ks; std::string srv_ks_name;
		t_texture2dSRV texture2dSRV;
		HRESULT h_ka, h_kd, h_ks;

		std::vector<ColoredTriangleDemo::t_texture2dSRV> v_texture2dSRV;

		//Iterate through the meshes and load each textures
		for (int i = 0; i < meshInfo.LoadedMeshes.size(); ++i){
			srv_ka_name = meshInfo.LoadedMeshes[i].MeshMaterial.map_Ka;
			srv_kd_name = meshInfo.LoadedMeshes[i].MeshMaterial.map_Kd;
			srv_ks_name = meshInfo.LoadedMeshes[i].MeshMaterial.map_Ks;

			//Reset to null_ptrs
			h_ka = S_FALSE;
			h_kd = S_FALSE;
			h_ks = S_FALSE;
			srv_ka = nullptr;
			srv_kd = nullptr;
			srv_ks = nullptr;

			if (!srv_ka_name.empty()){
				h_ka = CreateWICTextureFromFile(mGame->Direct3DDevice(), NULL, std::wstring(srv_ka_name.begin(), srv_ka_name.end()).c_str(), NULL, &srv_ka);
				if (FAILED(h_ka)){
					std::cout << "Texture Loading failed from file " << h_ka << std::endl;
					exit;
				}
			}
			if (!srv_kd_name.empty()){
				h_kd = CreateWICTextureFromFile(mGame->Direct3DDevice(), NULL, std::wstring(srv_kd_name.begin(), srv_kd_name.end()).c_str(), NULL, &srv_kd);
				if (FAILED(h_kd)){
					std::cout << "Texture Loading failed from file " << h_kd << std::endl;
					exit;
				}
			}
			if (!srv_ks_name.empty()){
				h_ks = CreateWICTextureFromFile(mGame->Direct3DDevice(), NULL, std::wstring(srv_ks_name.begin(), srv_ks_name.end()).c_str(), NULL, &srv_ks);
				if (FAILED(h_ks)){
					std::cout << "Texture Loading failed from file " << h_ks << std::endl;
					exit;
				}
			}

			//Load textures one by one and push to texture array
			texture2dSRV.mTexture2dSRV[0] = srv_ka;
			texture2dSRV.mTexture2dSRV[1] = srv_kd;
			texture2dSRV.mTexture2dSRV[2] = srv_ks;
			v_texture2dSRV.push_back(texture2dSRV);

		}//End: For loop

		return v_texture2dSRV;
	}//End: Load texture from file

	//Create material constant buffers
	std::vector<ID3D11Buffer*> ColoredTriangleDemo::load_material_cbuffer(std::vector<ColoredTriangleDemo::t_objectMaterial> v_materials) {
		t_objectMaterial materialData;
		std::vector<ID3D11Buffer*> v_material_cbuffer;
		ID3D11Buffer* cbuffer;

		for (std::vector<t_objectMaterial>::iterator it = v_materials.begin(); it != v_materials.end(); ++it){
			materialData = *it;
			//cbuffer = nullptr;
			
			//Create constant buffer Desc
			D3D11_BUFFER_DESC cbufferDesc;
			ZeroMemory(&cbufferDesc, sizeof(cbufferDesc));
			cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbufferDesc.ByteWidth = sizeof(materialData);
			cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			
			//Create subresource data
			D3D11_SUBRESOURCE_DATA cbufferData;
			ZeroMemory(&cbufferData, sizeof(cbufferData));
			cbufferData.pSysMem = &materialData;

			HRESULT cb = mGame->Direct3DDevice()->CreateBuffer(&cbufferDesc, &cbufferData, &cbuffer);
			if (FAILED(cb)){
				std::cout << "Ambient Light Constant Buffer Creation Failed " << cb << std::endl;
				exit;
			}
			//Push to vector
			v_material_cbuffer.push_back(cbuffer);
		}
		return v_material_cbuffer;
	}//End: Load material cbuffer

	//Create ambient light buffer
	ID3D11Buffer* ColoredTriangleDemo::create_light_cbuffer(){
		//Currently up color and down color are hardcoded
		ID3D11Buffer* mLightCBuffer;
		XMFLOAT4 ambientLightColor, pointLightColor, pointLightLoc;
		typedef struct {
			XMFLOAT4 ambientLightColor;
			XMFLOAT4 pointLightColor;
			XMFLOAT4 pointLightLoc;
		} t_LightStruct;

		ambientLightColor = { 227.f / 256.f, 234.f / 256.f, 18.f / 256.f, 1.f };
		pointLightColor = { 221.f / 256.f, 153.f / 256.f, 17.f / 256.f, 1.f };
		pointLightLoc = { -3.f, -0 / 5.f, -2.f, 1.f };

		t_LightStruct lightInfo = { ambientLightColor, pointLightColor, pointLightLoc };

		//Create buffer descriptor
		D3D11_BUFFER_DESC lightDesc;
		ZeroMemory(&lightDesc, sizeof(lightDesc));
		lightDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		lightDesc.ByteWidth = sizeof(lightInfo);
		lightDesc.Usage = D3D11_USAGE_DYNAMIC;
		lightDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		//Create SubResource Data
		D3D11_SUBRESOURCE_DATA lightSubResData;
		ZeroMemory(&lightSubResData, sizeof(lightSubResData));
		lightSubResData.pSysMem = &lightInfo;

		HRESULT lcb = mGame->Direct3DDevice()->CreateBuffer(&lightDesc, &lightSubResData, &mLightCBuffer);
		if (FAILED(lcb)){
			std::cout << "Ambient Light Constant Buffer Creation Failed " << lcb << std::endl;
			exit;
		}
		return mLightCBuffer;
	}

	ID3D11Buffer* ColoredTriangleDemo::create_waves_cbuffer(){
		ID3D11Buffer* mWaveBuffer;
		t_wave_consts wave_constants;
		std::vector<ColoredTriangleDemo::t_wave_consts> v_wave_constants;
		int n = 2;		//2 buffers for now

		for (int i = 0; i < n; ++i){
			wave_constants = {};
			wave_constants.amp = float(std::rand() % 10) / 100.f;
			wave_constants.k = float(std::rand() % 10) / 100.f;
			wave_constants.w = float(std::rand() % 10) / 100.f;
			wave_constants.phase = float(std::rand() % 10) / 100.f;
			v_wave_constants.push_back(wave_constants);			
		}
		
		//Copy
		this->v_wave_consts = v_wave_constants;
	
		D3D11_BUFFER_DESC mWaveBufferDesc;
		ZeroMemory(&mWaveBufferDesc, sizeof(mWaveBufferDesc));
		mWaveBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		mWaveBufferDesc.ByteWidth = sizeof(t_wave_consts) * v_wave_constants.size();
		mWaveBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		mWaveBufferDesc.Usage = D3D11_USAGE_DYNAMIC;

		D3D11_SUBRESOURCE_DATA mWaveBufferData;
		ZeroMemory(&mWaveBufferData, sizeof(mWaveBufferData));
		mWaveBufferData.pSysMem = &v_wave_constants[0];

		HRESULT wb = mGame->Direct3DDevice()->CreateBuffer(&mWaveBufferDesc, &mWaveBufferData, &mWaveBuffer);
		if (FAILED(wb)){
			std::cout << "Error. Wave constant buffer creation failed " << wb << std::endl;
			exit;
		}
		return mWaveBuffer;
	}

	//Update wave constants
	void ColoredTriangleDemo::update_wave_constants(){
		//Get key presses. Y incr K, H decr k. U inr w, J decr w. Sensitivity is 1/50. R is for reset
		for (int i = 0; i < 2; ++i){
			float x = pKeyboard->IsKeyDown(DIKEYBOARD_Y) ? (this->v_wave_consts[i].k + 0.02f) :
				pKeyboard->IsKeyDown(DIKEYBOARD_H) ? (this->v_wave_consts[i].k - 0.02f) :
				pKeyboard->IsKeyDown(DIKEYBOARD_R) ? (float(std::rand() % 10) / 100.f) : this->v_wave_consts[i].k;
			float y = pKeyboard->IsKeyDown(DIKEYBOARD_J) ? (this->v_wave_consts[i].w - 0.02f) :
				pKeyboard->IsKeyDown(DIKEYBOARD_U) ? (this->v_wave_consts[i].w + 0.02f) :
				pKeyboard->IsKeyDown(DIKEYBOARD_R) ? (float(std::rand() % 10) / 100.f) : this->v_wave_consts[i].w;
			float z = pKeyboard->IsKeyDown(DIKEYBOARD_K) ? (this->v_wave_consts[i].amp - 0.02f) :
				pKeyboard->IsKeyDown(DIKEYBOARD_I) ? (this->v_wave_consts[i].amp + 0.02f) :
				pKeyboard->IsKeyDown(DIKEYBOARD_R) ? (float(std::rand() % 10) / 100.f) : this->v_wave_consts[i].amp;

			this->v_wave_consts[i].k = x;
			this->v_wave_consts[i].w = y;
			this->v_wave_consts[i].amp = z;
		}
			//Create Mapped subresource
			D3D11_MAPPED_SUBRESOURCE newData;
			ZeroMemory(&newData, sizeof(newData));
			newData.RowPitch = 16;
			newData.DepthPitch = 2;

			//Update subresource data
			mGame->Direct3DDeviceContext()->Map(mWavesCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &newData);
			memcpy(newData.pData, &v_wave_consts[0], sizeof(ColoredTriangleDemo::t_wave_consts)*v_wave_consts.size());
			mGame->Direct3DDeviceContext()->Unmap(mWavesCBuffer, 0);

	}

	//Create time constant buffer
	ID3D11Buffer* ColoredTriangleDemo::create_time_cbuffer(){
		ID3D11Buffer* time_cbuffer;
		XMFLOAT4 time = { 0.f, 0.f, 0.f, 0.f };
		this->time_elapsed = { 0.f, 0.f, 0.f, 0.f };

		D3D11_BUFFER_DESC timeCBufferDesc;
		ZeroMemory(&timeCBufferDesc, sizeof(timeCBufferDesc));
		timeCBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		timeCBufferDesc.ByteWidth = sizeof(XMFLOAT4);
		timeCBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		timeCBufferDesc.Usage = D3D11_USAGE_DYNAMIC;

		D3D11_SUBRESOURCE_DATA timeCBufferData;
		ZeroMemory(&timeCBufferData, sizeof(timeCBufferData));
		timeCBufferData.pSysMem = &time;

		mGame->Direct3DDevice()->CreateBuffer(&timeCBufferDesc, &timeCBufferData, &time_cbuffer);
		return time_cbuffer;
	}

	//Update time constant buffer
	void ColoredTriangleDemo::update_time_cbuffer(){
		XMFLOAT4 time = { this->time_elapsed.x + 1.f, 0.f, 0.f, 0.f };
		this->time_elapsed = time;

		D3D11_MAPPED_SUBRESOURCE timeData;
		ZeroMemory(&timeData, sizeof(timeData));
		timeData.RowPitch = sizeof(XMFLOAT4);
		timeData.DepthPitch = 1;
		timeData.pData = &time;

		//Update subresource data
		mGame->Direct3DDeviceContext()->Map(mTimeCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &timeData);
		memcpy(timeData.pData, &time, sizeof(XMFLOAT4));
		mGame->Direct3DDeviceContext()->Unmap(mTimeCBuffer, 0);
	}


	void ColoredTriangleDemo::Initialize()
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		//Get mouse
		pMouse = (Mouse*)mGame->Services().GetService(Mouse::TypeIdClass());
		assert(pMouse != nullptr);

		//Set default camera location
		this->mouse_loc = { 2.f, 2.f, -2.f };

		//Get Keyboard
		pKeyboard = (Keyboard*)mGame->Services().GetService(Keyboard::TypeIdClass());
		assert(pKeyboard != nullptr);

		// Load a compiled vertex shader
		std::vector<char> compiledVertexShader;
		Utility::LoadBinaryFile(L"Content\\Effects\\VertexShader.cso", compiledVertexShader);		
		ThrowIfFailed(mGame->Direct3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, &mVertexShader), "ID3D11Device::CreatedVertexShader() failed.");

		// Load a compiled pixel shader
		std::vector<char> compiledPixelShader;
		Utility::LoadBinaryFile(L"Content\\Effects\\PixelShader.cso", compiledPixelShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, &mPixelShader), "ID3D11Device::CreatedPixelShader() failed.");

		//Create Rasterizer State
		D3D11_RASTERIZER_DESC rasStateDesc;
		ZeroMemory(&rasStateDesc, sizeof(rasStateDesc));
		rasStateDesc.CullMode = D3D11_CULL_BACK;
		rasStateDesc.FrontCounterClockwise = FALSE;
		rasStateDesc.FillMode = D3D11_FILL_WIREFRAME;
		HRESULT rs = mGame->Direct3DDevice()->CreateRasterizerState(&rasStateDesc, &rasState);
		if (FAILED(rs)){
			std::cout << "Rasterizer State Creation Failed " << rs << std::endl;
			return;
		}

		// Create an input layout for Postion Texture Normal
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ThrowIfFailed(mGame->Direct3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), &mInputLayout), "ID3D11Device::CreateInputLayout() failed.");

		//Load obj file
		mobjMeshLoader = load_mesh_from_file("E:\\DX\\Essential_lessons\\source\\Lesson3.1\\Meshes\\Ocean.obj");

		//Build vertex Buffer
		v_vertexBuffer = load_vertex_buffer_from_file(mobjMeshLoader);

		//Build index Buffer
		v_indexBuffer = load_index_buffer_from_file(mobjMeshLoader);
		
		//Load materials
		v_materials = load_materials(mobjMeshLoader);

		//Load texture
		v_texture2dSRV = load_textures_from_file(mobjMeshLoader);

		//Load material constant buffer
		v_objMaterialCBuffer = load_material_cbuffer(v_materials);

		//Create light constant buffer
		mLightCBuffer = create_light_cbuffer();

		//Create MVP Constant buffer
		mConstantBuffer = create_mvp_cbuffer();

		//Create time constant buffer
		mTimeCBuffer = create_time_cbuffer();

		//Create VS constant buffer to hold ocean sinewave data
		mWavesCBuffer = create_waves_cbuffer();

		//Create sampler State object
		textureSamplerStateObject = create_shader_sampler();
	}

	void ColoredTriangleDemo::Draw(const GameTime& gameTime)
	{ 
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		//Set common configs before issuing drawcalls
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout);
		direct3DDeviceContext->RSSetState(rasState);

		direct3DDeviceContext->VSSetShader(mVertexShader, nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader, nullptr, 0);
		
		direct3DDeviceContext->PSSetSamplers(0, 1, &textureSamplerStateObject);

		UINT stride = sizeof(VertexPositionTextureNormalMaterialId);
		UINT offset = 0;

		//Set MVP constant buffer
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, &mConstantBuffer);

		//Set Wave constant buffer
		direct3DDeviceContext->VSSetConstantBuffers(1, 1, &mWavesCBuffer);
		
		//Set time constant buffer
		direct3DDeviceContext->VSSetConstantBuffers(2, 1, &mTimeCBuffer);

		//Set light constant buffer
		direct3DDeviceContext->PSSetConstantBuffers(0, 1, &mLightCBuffer);

		//Issue drawcalls per mesh
		for (int i = 0; i < mobjMeshLoader.LoadedMeshes.size(); ++i){
			//Set vertex Buffer
			direct3DDeviceContext->IASetVertexBuffers(0, 1, &v_vertexBuffer[i], &stride, &offset);
			//Set index Buffer
			direct3DDeviceContext->IASetIndexBuffer(v_indexBuffer[i], DXGI_FORMAT_R32_UINT, 0);
			//Set texture resources
			direct3DDeviceContext->PSSetShaderResources(0, 3, &v_texture2dSRV[i].mTexture2dSRV[0]);
			//Set material constant buffer
			direct3DDeviceContext->PSSetConstantBuffers(1, 1, &v_objMaterialCBuffer[i]);
			//Issue Drawcall
			direct3DDeviceContext->DrawIndexed(mobjMeshLoader.LoadedMeshes[i].Indices.size(), 0, 0);
		}
		Sleep(10);		//Sleep for 100 ms
	}
	void ColoredTriangleDemo::Update(const GameTime& gametime){
		//Update VS constant buffer
		this->update_wave_constants();
		this->update_time_cbuffer();
		//Update camera location
		this->update_camera_loc();
	}
}