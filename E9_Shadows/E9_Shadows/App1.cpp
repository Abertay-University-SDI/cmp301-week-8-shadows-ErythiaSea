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
	cube = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"light", L"res/DefaultDiffuse.png");

	// initial shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);

	// Variables for defining shadow map
	int shadowmapWidth = 2048;
	int shadowmapHeight = 2048;
	int sceneWidth = 100;
	int sceneHeight = 100;

	// These are my shadow maps
	for (int i = 0; i < size(lights); i++) { shadowMaps[i] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight); }

	// Initialise lights
	for (int i = 0; i < size(lights); i++) { lights[i] = new Light(); }

	// Setup two directional lights
	lights[0]->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	lights[0]->setDiffuseColour(1.0f, 0.0f, 0.0f, 1.0f);
	lights[0]->setDirection(0.0f, -0.7f, 0.7f);
	lights[0]->setPosition(0.f, 5.f, -10.f);
	lights[0]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);

	lights[1]->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	lights[1]->setDiffuseColour(0.0f, 1.0f, 0.0f, 1.0f);
	lights[1]->setDirection(0.0f, -0.7f, 0.7f);
	lights[1]->setPosition(0.f, 5.f, -10.f);
	lights[1]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
	
	// Setup shadow map ortho mesh
	shadowInfoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 4, screenHeight / 4, screenWidth / 2.7f, screenHeight / 2.7f);
	shadowInfoMesh2 = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 4, screenHeight / 4, screenWidth / 2.7f, screenHeight / 5.4f);
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.

}

bool App1::frame()
{
	for (int i = 0; i < 2; i++) {
		lights[i]->setPosition(lightPos[i][0], lightPos[i][1], lightPos[i][2]);
		lights[i]->setDirection(lightDir[i][0], lightDir[i][1], lightDir[i][2]);
	}

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

XMFLOAT3 App1::arrToXMFloat3(float f[3])
{
	return XMFLOAT3(f[0], f[1], f[2]);
}

float* App1::XMFloat3ToArr(XMFLOAT3 f)
{
	float o[3];
	o[0] = f.x; o[1] = f.y; o[2] = f.z;
	return o;
}

bool App1::render()
{

	// Perform depth pass
	depthPass();
	// Render scene
	finalPass();

	return true;
}

void App1::depthPass()
{
	for (int i = 0; i < 2; i++) {
		// FIRST SHADOW MAP
		// Set the render target to be the render to texture.
		shadowMaps[i]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

		// get the world, view, and projection matrices from the camera and d3d objects.
		lights[i]->generateViewMatrix();
		XMMATRIX lightViewMatrix = lights[i]->getViewMatrix();
		XMMATRIX lightProjectionMatrix = lights[i]->getOrthoMatrix();
		XMMATRIX worldMatrix = renderer->getWorldMatrix();

		worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
		// Render floor
		mesh->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

		worldMatrix = renderer->getWorldMatrix();
		worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
		XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixRotationRollPitchYaw(0, teapotRotate, 0);
		worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
		// Render model
		model->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

		// Render additional geometry
		worldMatrix = renderer->getWorldMatrix();
		worldMatrix = XMMatrixTranslation(15.f, 1.f, 0.f);
		sphere->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

		worldMatrix = XMMatrixTranslation(-20.f, 1.f, 0.f);
		cube->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), cube->getIndexCount());

		depthMaps[i] = shadowMaps[i]->getDepthMapSRV();

		// Set back buffer as render target and reset view port.
		renderer->setBackBufferRenderTarget();
		renderer->resetViewport();
	}
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

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,  textureMgr->getTexture(L"brick"), depthMaps, lights);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixRotationRollPitchYaw(0, teapotRotate, 0);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), depthMaps, lights);
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// Render additional geometry
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(15.f, 1.f, 0.f);
	sphere->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), depthMaps, lights);
	shadowShader->render(renderer->getDeviceContext(), sphere->getIndexCount());
	//Cube
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(-20.f, 1.f, 0.f);
	cube->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), depthMaps, lights);
	shadowShader->render(renderer->getDeviceContext(), cube->getIndexCount());

	// Light bulbs
	sphere->sendData(renderer->getDeviceContext());
	for (int i = 0; i < 2; i++) {
		worldMatrix = renderer->getWorldMatrix();
		worldMatrix = XMMatrixTranslation(lights[i]->getPosition().x, lights[i]->getPosition().y, lights[i]->getPosition().z);
		textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"light"));
		textureShader->render(renderer->getDeviceContext(), sphere->getIndexCount());
	}

	// Render ortho meshes
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering
	worldMatrix = renderer->getWorldMatrix();

	renderer->setZBuffer(false);
	shadowInfoMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, depthMaps[0]);
	textureShader->render(renderer->getDeviceContext(), shadowInfoMesh->getIndexCount());
	shadowInfoMesh2->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, depthMaps[1]);
	textureShader->render(renderer->getDeviceContext(), shadowInfoMesh->getIndexCount());
	renderer->setZBuffer(true);

	gui();
	renderer->endScene();
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

	ImGui::SliderFloat("Teapot Rotation", &teapotRotate, 0.0f, 6.283f);
	ImGui::SliderFloat3("Light 1 Position", lightPos[0], -100.0f, 100.0f);
	ImGui::SliderFloat3("Light 1 Direction", lightDir[0], -1.0f, 1.0f);
	ImGui::SliderFloat3("Light 2 Position", lightPos[1], -100.0f, 100.0f);
	ImGui::SliderFloat3("Light 2 Direction", lightDir[1], -1.0f, 1.0f);

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

