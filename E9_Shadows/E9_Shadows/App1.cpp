// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create Mesh object and shader object
	mesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	model = new AModel(renderer->getDevice(), "res/teapot.obj");
	sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	box = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	shadowData = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth/4, screenHeight/4, screenWidth / 2.7, screenHeight / 2.7);
	rShadowData = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth/4, screenHeight/4, screenWidth / 2.7, screenHeight / 2.7);
	gShadowData = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth/4, screenHeight/4, -screenWidth / 2.7, screenHeight / 2.7);
	
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");

	// render targets
	shadowTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	greenShadowTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	redShadowTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);

	// initial shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);

	// Variables for defining shadow map
	// 1024
	int shadowmapWidth = 5120;
	int shadowmapHeight = 5120;
	// determines area in which shadows will apply to (light view frustum)
	// 100
	int sceneWidth = 100;
	int sceneHeight = 100;
	float sceneNear = 5;
	float sceneFar = 50;

	// This is your shadow map
	shadowMap = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
	gLightSM = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
	rLightSM = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
	//shadowMaps[0] = gLightSM;
	//shadowMaps[1] = rLightSM;

	// Configure directional light
	gLight = new Light();
	gLight->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	gLight->setDiffuseColour(0.0f, 1.0f, 0.0f, 1.0f);
	gLight->setDirection(0.45f, 0.0f, 0.0f);
	gLight->setPosition(-10.f, 5.f, 25.f);
	gLight->generateProjectionMatrix(sceneNear, sceneFar);
	gLight->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
	sceneLights[0] = gLight;

	greenLightDir[0] = gLight->getDirection().x;
	greenLightDir[1] = gLight->getDirection().y;
	greenLightDir[2] = gLight->getDirection().z;

	greenLightPos[0] = gLight->getPosition().x;
	greenLightPos[1] = gLight->getPosition().y;
	greenLightPos[2] = gLight->getPosition().z;

	rLight = new Light();
	rLight->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	rLight->setDiffuseColour(1.0f, 0.0f, 0.0f, 1.0f);
	rLight->setDirection(-0.45f, 0.0f, 0.0f);
	rLight->setPosition(10.f, 5.f, 25.f);
	rLight->generateProjectionMatrix(sceneNear, sceneFar);
	rLight->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
	sceneLights[1] = rLight;

	redLightDir[0] = rLight->getDirection().x;
	redLightDir[1] = rLight->getDirection().y;
	redLightDir[2] = rLight->getDirection().z;

	redLightPos[0] = rLight->getPosition().x;
	redLightPos[1] = rLight->getPosition().y;
	redLightPos[2] = rLight->getPosition().z;
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.

}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	totalTime += timer->getTime();
	updateLights();

	// Perform depth pass
	//depthPass();
	//renderShadowData();
	//// Render scene
	//finalPass();

	lightDepthPass();
	renderGLightDepth();
	renderRLightDepth();
	finalLightDepthPass();

	return true;
}

void App1::depthPass()
{
	// Set the render target to be the render to texture.
	gLightSM->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get the world, view, and projection matrices from the camera and d3d objects.
	gLight->generateViewMatrix();
	XMMATRIX lightViewMatrix = gLight->getViewMatrix();
	XMMATRIX lightProjectionMatrix = gLight->getProjectionMatrix();
	//XMMATRIX lightProjectionMatrix = gLight->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// render cube (left of model)
	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX translateM = XMMatrixTranslation(-20.f, 1.f, 5.f);
	XMMATRIX rotationMatrix = XMMatrixRotationY(10 + (totalTime * speed));
	worldMatrix = XMMatrixMultiply(worldMatrix, rotationMatrix);
	worldMatrix = XMMatrixMultiply(worldMatrix, translateM);
	box->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), box->getIndexCount());

	// render sphere (right of model)
	worldMatrix = renderer->getWorldMatrix();
	translateM = XMMatrixTranslation(5.f, 1.f, 25.f);
	rotationMatrix = XMMatrixRotationY(10 + (totalTime * speed));
	worldMatrix = XMMatrixMultiply(worldMatrix, translateM);
	worldMatrix = XMMatrixMultiply(worldMatrix, rotationMatrix);
	sphere->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::renderShadowData() {
	// Set the render target to be the render to texture.
	shadowTexture->setRenderTarget(renderer->getDeviceContext());
	shadowTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 0.0f, 0.0f, 1.0f);

	// get the world, view, and projection matrices from the camera and d3d objects.
	gLight->generateViewMatrix();
	XMMATRIX lightViewMatrix = gLight->getViewMatrix();
	XMMATRIX lightProjectionMatrix = gLight->getProjectionMatrix();
	//XMMATRIX lightProjectionMatrix = gLight->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	// Render model
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// render cube (left of model)
	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX translateM = XMMatrixTranslation(-20.f, 1.f, 5.f);
	XMMATRIX rotationMatrix = XMMatrixRotationY(10 + (totalTime * speed));
	worldMatrix = XMMatrixMultiply(worldMatrix, rotationMatrix);
	worldMatrix = XMMatrixMultiply(worldMatrix, translateM);
	box->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), box->getIndexCount());

	// render sphere (right of model)
	worldMatrix = renderer->getWorldMatrix();
	translateM = XMMatrixTranslation(5.f, 1.f, 25.f);
	rotationMatrix = XMMatrixRotationY(10 + (totalTime * speed));
	worldMatrix = XMMatrixMultiply(worldMatrix, translateM);
	worldMatrix = XMMatrixMultiply(worldMatrix, rotationMatrix);
	sphere->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();
	ID3D11ShaderResourceView* depthMaps[2] = { gLightSM->getDepthMapSRV(), rLightSM->getDepthMapSRV() };

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, 
		textureMgr->getTexture(L"brick"), depthMaps, sceneLights);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), depthMaps, sceneLights);
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// render cube (left of model)
	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX translateM = XMMatrixTranslation(-20.f, 1.f, 5.f);
	XMMATRIX rotationMatrix = XMMatrixRotationY(10 + (totalTime * speed));
	worldMatrix = XMMatrixMultiply(worldMatrix, rotationMatrix);
	worldMatrix = XMMatrixMultiply(worldMatrix, translateM);
	box->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(nullptr), depthMaps, sceneLights);
	shadowShader->render(renderer->getDeviceContext(), box->getIndexCount());

	// render sphere (right of model)
	worldMatrix = renderer->getWorldMatrix();
	translateM = XMMatrixTranslation(5.f, 1.f, 25.f);
	rotationMatrix = XMMatrixRotationY(10 + (totalTime * speed));
	worldMatrix = XMMatrixMultiply(worldMatrix, translateM);
	worldMatrix = XMMatrixMultiply(worldMatrix, rotationMatrix);
	sphere->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(nullptr), depthMaps, sceneLights);
	shadowShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	// render sphere at light locations (only shows in final pass)
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(gLight->getPosition().x, gLight->getPosition().y, gLight->getPosition().z);
	sphere->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(nullptr));
	textureShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	//render shadow data
	worldMatrix = renderer->getWorldMatrix();
	renderer->setZBuffer(false);
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering

	shadowData->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, shadowTexture->getShaderResourceView());
	textureShader->render(renderer->getDeviceContext(), shadowData->getIndexCount());

	renderer->setZBuffer(true);

	gui();
	renderer->endScene();
}


void App1::lightDepthPass() {
	gLightDepthPass();
	rLightDepthPass();
}

void App1::gLightDepthPass() {
	// Set the render target to be the render to texture.
	gLightSM->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get the world, view, and projection matrices from the camera and d3d objects.
	gLight->generateViewMatrix();
	XMMATRIX lightViewMatrix = gLight->getViewMatrix();
	//XMMATRIX lightProjectionMatrix = gLight->getProjectionMatrix();
	XMMATRIX lightProjectionMatrix = gLight->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// render sphere
	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX translateM = XMMatrixTranslation(0.f, 1.f, 25.f);
	//XMMATRIX rotationMatrix = XMMatrixRotationY(10 + (totalTime * speed));
	worldMatrix = XMMatrixMultiply(worldMatrix, translateM);
	//worldMatrix = XMMatrixMultiply(worldMatrix, rotationMatrix);
	sphere->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::renderGLightDepth() {
	// Set the render target to be the render to texture.
	greenShadowTexture->setRenderTarget(renderer->getDeviceContext());
	greenShadowTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 0.0f, 0.0f, 1.0f);

	// get the world, view, and projection matrices from the camera and d3d objects.
	gLight->generateViewMatrix();
	XMMATRIX lightViewMatrix = gLight->getViewMatrix();
	//XMMATRIX lightProjectionMatrix = gLight->getProjectionMatrix();
	XMMATRIX lightProjectionMatrix = gLight->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// render sphere
	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX translateM = XMMatrixTranslation(0.f, 1.f, 25.f);
	//XMMATRIX rotationMatrix = XMMatrixRotationY(10 + (totalTime * speed));
	worldMatrix = XMMatrixMultiply(worldMatrix, translateM);
	//worldMatrix = XMMatrixMultiply(worldMatrix, rotationMatrix);
	sphere->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::rLightDepthPass() {
	// Set the render target to be the render to texture.
	rLightSM->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get the world, view, and projection matrices from the camera and d3d objects.
	rLight->generateViewMatrix();
	XMMATRIX lightViewMatrix = rLight->getViewMatrix();
	//XMMATRIX lightProjectionMatrix = rLight->getProjectionMatrix();
	XMMATRIX lightProjectionMatrix = rLight->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// render sphere
	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX translateM = XMMatrixTranslation(0.f, 1.f, 25.f);
	//XMMATRIX rotationMatrix = XMMatrixRotationY(10 + (totalTime * speed));
	worldMatrix = XMMatrixMultiply(worldMatrix, translateM);
	//worldMatrix = XMMatrixMultiply(worldMatrix, rotationMatrix);
	sphere->sendData(renderer->getDeviceContext());
	
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();

}

void App1::renderRLightDepth() {
	// Set the render target to be the render to texture.
	redShadowTexture->setRenderTarget(renderer->getDeviceContext());
	redShadowTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 0.0f, 0.0f, 1.0f);

	// get the world, view, and projection matrices from the camera and d3d objects.
	rLight->generateViewMatrix();
	XMMATRIX lightViewMatrix = rLight->getViewMatrix();
	XMMATRIX lightProjectionMatrix = gLight->getProjectionMatrix();
	//XMMATRIX lightProjectionMatrix = rLight->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// render sphere
	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX translateM = XMMatrixTranslation(0.f, 1.f, 25.f);
	//XMMATRIX rotationMatrix = XMMatrixRotationY(10 + (totalTime * speed));
	worldMatrix = XMMatrixMultiply(worldMatrix, translateM);
	//worldMatrix = XMMatrixMultiply(worldMatrix, rotationMatrix);
	sphere->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::finalLightDepthPass() {
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();
	ID3D11ShaderResourceView* depthMaps[2] = {gLightSM->getDepthMapSRV(), rLightSM->getDepthMapSRV()};

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture(L"brick"), depthMaps, sceneLights);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	
	// render sphere (right of model)
	// render sphere
	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX translateM = XMMatrixTranslation(0.f, 1.f, 25.f);
	//XMMATRIX rotationMatrix = XMMatrixRotationY(10 + (totalTime * speed));
	worldMatrix = XMMatrixMultiply(worldMatrix, translateM);
	//worldMatrix = XMMatrixMultiply(worldMatrix, rotationMatrix);
	sphere->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(nullptr), depthMaps, sceneLights);
	shadowShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	// render sphere at light locations (only shows in final pass)
	renderLightSphere(gLight->getPosition());
	renderLightSphere(rLight->getPosition());

	//render shadow data
	worldMatrix = renderer->getWorldMatrix();
	renderer->setZBuffer(false);
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering

	gShadowData->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, greenShadowTexture->getShaderResourceView());
	textureShader->render(renderer->getDeviceContext(), gShadowData->getIndexCount());

	rShadowData->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, redShadowTexture->getShaderResourceView());
	textureShader->render(renderer->getDeviceContext(), rShadowData->getIndexCount());

	renderer->setZBuffer(true);

	gui();
	renderer->endScene();
}

void App1::updateLights() {
	gLight->setPosition(greenLightPos[0], greenLightPos[1], greenLightPos[2]);
	gLight->setDirection(greenLightDir[0], greenLightDir[1], greenLightDir[2]);
	rLight->setPosition(redLightPos[0], redLightPos[1], redLightPos[2]);
	rLight->setDirection(redLightDir[0], redLightDir[1], redLightDir[2]);
}

void App1::renderLightSphere(XMFLOAT3 position) {
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(position.x, position.y, position.z);
	sphere->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(nullptr));
	textureShader->render(renderer->getDeviceContext(), sphere->getIndexCount());
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	ImGui::PushItemWidth(200);
	ImGui::NewLine;
	if (ImGui::CollapsingHeader("Green light")) {
		ImGui::SliderFloat3("green light position", greenLightPos, -50.0, 100.0);
		ImGui::NewLine;
		ImGui::SliderFloat3("green light direction", greenLightDir, -1.0, 1.0);
	}
	ImGui::NewLine;
	if (ImGui::CollapsingHeader("Red light")) {
		ImGui::SliderFloat3("red light position", redLightPos, -50.0, 100.0);
		ImGui::NewLine;
		ImGui::SliderFloat3("red light direction", redLightDir, -1.0, 1.0);
	}
	

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

