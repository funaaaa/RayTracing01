#pragma once
#include "DynamicConstBuffer.h"
#include "Vec.h"
#include "FHelper.h"
#include "DirectXBase.h"
#include "Camera.h"

// 大気散乱用
struct AtmosphericScattering {

	float kr;				// レイリー散乱定数
	float km;				// ミー散乱定数
	float samples;			// 大気散乱サンプル数
	float outerRadius;		// 大気圏の最頂点の高さ
	float innerRadius;		// 地上の高さ
	float eSun;				// 太陽の強さ
	float g;				// 散乱定数を求める際に使用する値
	float aveHeight;		// 平均大気密度を求めるための高さ

};

struct RayPointLightData {

	Vec3 lightPos;
	float lightSize;
	Vec3 lightColor;
	float lightPower;
	int isActive;
	Vec3 pad;

};

struct RayDirLightData {

	Vec3 lihgtDir;
	int isActive;
	Vec3 lightColor;
	float pad;

};

struct RaySpotLightData {

	Vec3 pos;
	float angle;
	Vec3 dir;
	float power;
	Vec3 color;
	int isActive;

};

struct KariConstBufferData {

	XMMATRIX mtxView;			// ビュー行列。
	XMMATRIX mtxProj;			// プロジェクション行列。
	XMMATRIX mtxViewInv;		// ビュー逆行列。
	XMMATRIX mtxProjInv;		// プロジェクション逆行列。
	XMVECTOR ambientColor;		// 環境光。
	RayDirLightData dirLight;
	RayPointLightData pointLight;
	RaySpotLightData spotLight;
	AtmosphericScattering AS;	// 大気散乱用
	int seed;
	int counter;
	int aoSampleCount;
	int isNoiseScene;
	int isLightHitScene;
	int isNormalScene;
	int isMeshScene;
	int isNoAO;
	int isNoGI;
	int isGIOnlyScene;
	Vec3 eye = { 0,0,-10 };
	Vec3 target = { 0,0,0 };
	Vec3 up = { 0,1,0 };


	void Init() {

		Camera::Ins()->GenerateMatView();
		Camera::Ins()->Update();

		eye = Camera::Ins()->eye;
		target = Camera::Ins()->target;
		up = Camera::Ins()->up;

		ambientColor = { 1,1,1,1 };
		mtxProj = XMMatrixPerspectiveFovLH(
			XMConvertToRadians(60.0f),				//画角(60度)
			(float)window_width / window_height,	//アスペクト比
			0.1f, 1000000.0f							//前端、奥端
		);
		mtxProjInv = XMMatrixInverse(nullptr, mtxProj);
		mtxView = XMMatrixLookAtLH(eye.ConvertXMVECTOR(), target.ConvertXMVECTOR(), up.ConvertXMVECTOR());
		mtxViewInv = XMMatrixInverse(nullptr, mtxView);
		counter = 0;
		isNoiseScene = false;

		// 点光源をセッティング
		pointLight.lightPos = Vec3(0, 300, 0);
		pointLight.lightSize = 30.0f;
		pointLight.lightPower = 300.0f;
		pointLight.isActive = true;

		// 並行光源をセッティング
		dirLight.isActive = false;
		dirLight.lightColor = Vec3{ 0,0,0 };
		dirLight.lihgtDir = Vec3{ -0.5f,-1,0 };
		dirLight.lihgtDir.Normalize();

		// スポットライトをセッティング
		spotLight.isActive = false;
		spotLight.dir = Vec3{ 0,-1,0 };
		spotLight.pos = Vec3{ 0,300,0 };
		spotLight.power = 300.0f;
		spotLight.angle = DirectX::XM_PI;

		// 大気散乱用定数をセッティング
		AS.eSun = 20.0f;
		AS.g = -0.999f;
		AS.innerRadius = 10000.0f;
		AS.outerRadius = 10250.0f;
		AS.km = 0.0000f;
		AS.kr = 0.0025f;
		AS.samples = 2.0f;
		AS.aveHeight = 0.25f;

		// その他のデバッグ情報をセッティング
		seed = FHelper::GetRand(0, 1000);
		aoSampleCount = 1;
		isLightHitScene = false;
		isNormalScene = false;
		isMeshScene = false;
		isNoAO = false;
		isNoGI = false;
		isGIOnlyScene = false;

	}

};