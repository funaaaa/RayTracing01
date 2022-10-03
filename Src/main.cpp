﻿#include "SceneMgr.h"
#include "SoundManager.h"
#include "FbxLoader.h"
#include "RayDenoiser.h"
#include "Camera.h"
#include "HitGroupMgr.h"
#include "DescriptorHeapMgr.h"
#include "PiplineManager.h"
#include "PolygonInstanceRegister.h"
#include "Input.h"
#include "BLASRegister.h"
#include "RoughnessBlur.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	/*----------DirectX初期化処理----------*/
	DirectXBase::Ins()->Init();									// DirectX基盤の初期化
	SoundManager::Ins()->SettingSoundManager();	// サウンドマネージャーをセットする

	/*----------パイプライン生成----------*/
	PiplineManager::Ins()->Init();

	/*----------変数宣言----------*/
	srand(static_cast<unsigned int>(time(NULL)));

	// ディスクリプタヒープを初期化。
	DescriptorHeapMgr::Ins()->GenerateDescriptorHeap();

	// BLASを準備。
	BLASRegister::Ins()->Setting();

	// インスタンスをセット。
	PolygonInstanceRegister::Ins()->Setting();

	// FBXLoaderを初期化。
	FbxLoader::Ins()->Init();

	// ヒットグループを設定。
	HitGroupMgr::Ins()->Setting();

	// デノイズ用のクラスを初期化。
	Denoiser::Ins()->Setting();
	RoughnessBlur::Ins()->Setting();

	// カメラを初期化。
	Camera::Ins()->Init();

	// シーンクラス。
	SceneMgr::Ins()->Init();

	/*----------ゲームループ----------*/
	while (true) {

		// 描画前処理
		DirectXBase::Ins()->ProcessBeforeDrawing();


		SceneMgr::Ins()->Update();
		SceneMgr::Ins()->Draw();

		if (Input::Ins()->IsKeyTrigger(DIK_ESCAPE)) {

			exit(0);

		}


		// 描画後処理
		DirectXBase::Ins()->ProcessAfterDrawing();

	}

	return 0;
}