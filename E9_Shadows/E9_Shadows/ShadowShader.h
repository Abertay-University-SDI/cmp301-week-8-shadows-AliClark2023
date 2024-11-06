// Light shader.h
// Basic single light shader setup
#ifndef _SHADOWSHADER_H_
#define _SHADOWSHADER_H_
#define MAX_LIGHTS 4

#include "DXF.h"

using namespace std;
using namespace DirectX;


class ShadowShader : public BaseShader
{
private:
	//struct LightProperties
	//{

	//	XMFLOAT4 ambient; //move to material struct
	//	XMFLOAT4 diffuse; //move to material struct (change to colour)
	//	//float4 colour;
	//	XMFLOAT3 lightDirection;
	//	float spotCone;
	//	XMFLOAT3 position;
	//	float specularPower;
	//	XMFLOAT4 specularColour;

	//	// attenuation
	//	float constantFactor;
	//	float linearFactor;
	//	float quadraticFactor;
	//	float attenPadding;
	//	// light type (0 = directional, 1 = point light, 2 = spotlight)
	//	int lightType;  //4bytes
	//	bool enabled;   //4bytes
	//	int typePadding0; //4bytes
	//	int typePadding1; //4bytes
	//};
	//// array size must <= MAX_LIGHTS in pixel shader
	//struct LightBufferType
	//{
	//	LightProperties light[MAX_LIGHTS];
	//};

	//struct CameraBufferType
	//{
	//	XMFLOAT3 cameraPosition;
	//	float cameraPadding;
	//};

	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX lightView[2];
		XMMATRIX lightProjection[2];
		/*std::vector<XMMATRIX> lightViews;
		std::vector<XMMATRIX> lightProjections;*/
	};

	struct LightProperties
	{
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT3 direction;
		float padding;
	};

	struct LightBufferType
	{
		LightProperties lights[2];
	};

public:

	ShadowShader(ID3D11Device* device, HWND hwnd);
	~ShadowShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView*depthMap[2], Light* light[2]);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* cameraBuffer;
};

#endif