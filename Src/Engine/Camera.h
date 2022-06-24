#pragma once
#include <DirectXMath.h>
#include "Singleton.h"
#include "Vec.h"

class Camera : public Singleton<Camera> {
public:
	DirectX::XMMATRIX matView;		// ビュー行列
	Vec3 eye;				// ゲームワールド内でのカメラ座標
	Vec3 target;			// ゲームワールド内でカメラが見ている座標
	Vec3 up;				// ゲームワールド内でカメラから見て上方向を指すベクトル

	Vec3 forwardVec;

	Vec3 baseEye;		// 基準となる視点座標 eyeはこの値に向かって補間される。
	Vec3 baseTarget;	// 基準となる注視点座標 targetはこの値に向かって補間される。
	Vec3 baseUp;		// 基準となる上ベクトル upはこの値に向かって補間される。

	DirectX::XMMATRIX rotationMat;		// カメラの回転行列
	DirectX::XMMATRIX upRotationMat;	// カメラの上方向ベクトルの回転行列

	DirectX::XMMATRIX matPerspective;
	DirectX::XMMATRIX matProjection;

	float angleOfView;		// 画角
	float baseAngleOfView;	// 基準となる画角の値

private:

	const float EYE_PLAYER_DISTANCE = 100;			// プレイヤーと視点の距離
	const float TARGET_PLAYER_DISTNACE = 50;		// プレイヤーと注視点の距離
	const float TARGET_UPPER = 50;					// ターゲットを上にずらす量
	const float DEF_ANGLEOFVIEW = 60.0f;			// 画角のデフォルト値
	const float MAX_ANGLEOFVIEW = 100.0f;			// 最大画角

public:

	//コンストラクタ
	friend Singleton<Camera>;
	Camera();

	//初期化
	void Init();

	//ビュー行列生成
	void GenerateMatView();

	//更新処理
	void Update();
	void Update(const Vec3& CharaPos, const Vec3& CharaForwardVec, const Vec3& CharaUpVec, const float& CharaSpeedPer);

};