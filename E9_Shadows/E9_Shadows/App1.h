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

	void twoLDepthPass();
	void render2LDepthData();
	void twoLFinalPass();

	bool render();
	void gui();

	void updateLights();

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
	RenderTexture* shadowTexture;

	std::vector<Light> sceneLights;
	Light* gLight;
	Light* rLight;
	AModel* model;

	ShadowShader* shadowShader;
	DepthShader* depthShader;

	ShadowMap* gShadowMap;
	ShadowMap* rShadowMap;
	std::vector<ShadowMap> shadowMaps;
};

#endif