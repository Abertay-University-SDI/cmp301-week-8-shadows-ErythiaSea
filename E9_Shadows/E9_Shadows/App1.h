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
	XMFLOAT3 arrToXMFloat3(float f[3]);
	float* XMFloat3ToArr(XMFLOAT3 f);
protected:
	bool render();
	void depthPass();
	void finalPass();
	void gui();

private:
	TextureShader* textureShader;
	PlaneMesh* mesh;
	SphereMesh* sphere;
	CubeMesh* cube;

	Light* lights[2];
	AModel* model;
	ShadowShader* shadowShader;
	DepthShader* depthShader;

	ShadowMap* shadowMaps[2];
	ID3D11ShaderResourceView* depthMaps[2];

	OrthoMesh* shadowInfoMesh;
	OrthoMesh* shadowInfoMesh2;
	float teapotRotate = 0.0f;
	float lightPos[2][3] = { {0.f, 5.f, -10.f}, {0.f, 5.f, 10.f} };
	float lightDir[2][3] = { {0.0f, -0.7f, 0.7f }, { 0.0f, -0.7f, 0.7f} };
};

#endif