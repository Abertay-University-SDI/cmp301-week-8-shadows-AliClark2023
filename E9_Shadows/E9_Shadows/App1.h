// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "TextureShader.h"
#include "ShadowShader.h"
#include "DepthShader.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	
	void depthPass();
	void renderShadowData();
	void finalPass();

	void lightDepthPass();
	void gLightDepthPass();
	void renderGLightDepth();
	void rLightDepthPass();
	void renderRLightDepth();
	void finalLightDepthPass();

	bool render();
	void gui();

	void updateLights();
	void renderLightSphere(XMFLOAT3 position);

private:
	TextureShader* textureShader;
	PlaneMesh* mesh;
	// additional geometry
	SphereMesh* sphere;
	CubeMesh* box;

	// movement variables
	float totalTime;
	float speed = 1.0f;
	// imGui variables
	float greenLightPos[3];
	float greenLightDir[3];
	float redLightPos[3];
	float redLightDir[3];

	// rendering the shadow data
	OrthoMesh* shadowData;
	OrthoMesh* gShadowData;
	OrthoMesh* rShadowData;
	RenderTexture* shadowTexture;
	RenderTexture* greenShadowTexture;
	RenderTexture* redShadowTexture;
	Light* sceneLights[2];
	Light* gLight;
	Light* rLight;
	AModel* model;
	ShadowShader* shadowShader;
	DepthShader* depthShader;

	ShadowMap* shadowMap;
	ShadowMap* gLightSM;
	ShadowMap* rLightSM;
	//ShadowMap* shadowMaps[2];
};

#endif