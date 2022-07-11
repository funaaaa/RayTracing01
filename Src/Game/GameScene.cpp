#include "GameScene.h"
#include "HitGroupMgr.h"
#include "BLASRegister.h"
#include "PolygonInstanceRegister.h"
#include "Player.h"
#include "FHelper.h"
#include "Camera.h"
#include "RayDenoiser.h"
#include "RayRootsignature.h"
#include "DynamicConstBuffer.h"
#include "RaytracingPipline.h"
#include "RaytracingOutput.h"
#include "TLAS.h"
#include "Input.h"
#include "TextureManager.h"
#include "Sprite.h"
#include "Pipline.h"
#include "WindowsAPI.h"
#include "GimmickMgr.h"

GameScene::GameScene()
{

	/*===== 初期化処理 =====*/

	// 定数バッファを生成。
	constBufferData.Init();
	constBuffer = std::make_shared<DynamicConstBuffer>();
	constBuffer->Generate(sizeof(RayConstBufferData), L"CameraConstBuffer");
	constBuffer->Write(DirectXBase::Ins()->swapchain->GetCurrentBackBufferIndex(), &constBufferData, sizeof(RayConstBufferData));

	// デノイズAO用のパイプラインを設定。
	dAOuseShaders.push_back({ "Resource/ShaderFiles/RayTracing/DenoiseAOShader.hlsl", {L"mainRayGen"}, {L"mainMS", L"shadowMS"}, {L"mainCHS", L"mainAnyHit"} });
	pipline = std::make_shared<RaytracingPipline>();
	pipline->Setting(dAOuseShaders, HitGroupMgr::DENOISE_AO_HIT_GROUP, 1, 1, 5, sizeof(Vec3) * 5 + sizeof(UINT) + sizeof(UINT), sizeof(Vec2), 6);

	// 天球用のスフィアを生成する。
	skyDomeBlas = BLASRegister::Ins()->GenerateObj("Resource/Game/", "skydome.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], { L"Resource/Game/skydome.jpg" });
	skyDomeIns = PolygonInstanceRegister::Ins()->CreateInstance(skyDomeBlas, PolygonInstanceRegister::SHADER_ID::AS);
	PolygonInstanceRegister::Ins()->AddScale(skyDomeIns, Vec3(1000, 1000, 1000));

	// ステージを読み込む。
	stageBlas = BLASRegister::Ins()->GenerateObj("Resource/Game/", "stage3.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], { L"Resource/Game/white.png" });
	stageIns = PolygonInstanceRegister::Ins()->CreateInstance(stageBlas, PolygonInstanceRegister::DEF_GI);
	PolygonInstanceRegister::Ins()->AddScale(stageIns, Vec3(200, 200, 200));
	stageGrassBlas = BLASRegister::Ins()->GenerateObj("Resource/Game/", "grass.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], { L"Resource/Game/green.png",L"Resource/Game/grassNormal.png" });
	stageGrassIns = PolygonInstanceRegister::Ins()->CreateInstance(stageGrassBlas, PolygonInstanceRegister::DEF_GI);
	PolygonInstanceRegister::Ins()->AddScale(stageGrassIns, Vec3(200, 200, 200));



	// ゴール用のオブジェクトを読み込む。
	goalBlas = BLASRegister::Ins()->GenerateObj("Resource/Game/", "goal.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], { L"Resource/Game/red.png" });
	goalIns = PolygonInstanceRegister::Ins()->CreateInstance(goalBlas, PolygonInstanceRegister::SHADER_ID::REFRACTION);
	PolygonInstanceRegister::Ins()->AddScale(goalIns, Vec3(200, 200, 200));
	PolygonInstanceRegister::Ins()->AddTrans(goalIns, GOAL_DEF_POS);

	goalCollisionBlas = BLASRegister::Ins()->GenerateObj("Resource/Game/", "goalCollision.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], { L"Resource/Game/inv.png" }, false, false);
	goalCollisionIns = PolygonInstanceRegister::Ins()->CreateInstance(goalCollisionBlas, PolygonInstanceRegister::SHADER_ID::INVISIBILITY);
	PolygonInstanceRegister::Ins()->AddScale(goalCollisionIns, Vec3(200, 200, 200));

	middlePointCollisionBlas = BLASRegister::Ins()->GenerateObj("Resource/Game/", "middlePointCollision.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], { L"Resource/Game/inv.png" }, false, false);
	middlePointCollisionIns = PolygonInstanceRegister::Ins()->CreateInstance(middlePointCollisionBlas, PolygonInstanceRegister::SHADER_ID::INVISIBILITY);
	PolygonInstanceRegister::Ins()->AddScale(middlePointCollisionIns, Vec3(200, 200, 200));

	// ステージの装飾オブジェクトをロード
	{
		stageOrnamentBlas.emplace_back(BLASRegister::Ins()->GenerateObj("Resource/Game/stageOrnament/", "blockA.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], { L"Resource/Game/gray.png" }));
		stageOrnamentIns.emplace_back(PolygonInstanceRegister::Ins()->CreateInstance(stageOrnamentBlas[static_cast<int>(stageOrnamentBlas.size()) - 1], PolygonInstanceRegister::SHADER_ID::REFLECTION));
		stageOrnamentBlas.emplace_back(BLASRegister::Ins()->GenerateObj("Resource/Game/stageOrnament/", "blockB.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], { L"Resource/Game/gray.png" }));
		stageOrnamentIns.emplace_back(PolygonInstanceRegister::Ins()->CreateInstance(stageOrnamentBlas[static_cast<int>(stageOrnamentBlas.size()) - 1], PolygonInstanceRegister::SHADER_ID::REFLECTION));
		stageOrnamentBlas.emplace_back(BLASRegister::Ins()->GenerateObj("Resource/Game/stageOrnament/", "blockC.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], { L"Resource/Game/green.png" }));
		stageOrnamentIns.emplace_back(PolygonInstanceRegister::Ins()->CreateInstance(stageOrnamentBlas[static_cast<int>(stageOrnamentBlas.size()) - 1], PolygonInstanceRegister::SHADER_ID::REFRACTION));
		stageOrnamentBlas.emplace_back(BLASRegister::Ins()->GenerateObj("Resource/Game/stageOrnament/", "blockD.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], { L"Resource/Game/gray.png" }));
		stageOrnamentIns.emplace_back(PolygonInstanceRegister::Ins()->CreateInstance(stageOrnamentBlas[static_cast<int>(stageOrnamentBlas.size()) - 1], PolygonInstanceRegister::SHADER_ID::REFLECTION));
		stageOrnamentBlas.emplace_back(BLASRegister::Ins()->GenerateObj("Resource/Game/stageOrnament/", "blockE.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], { L"Resource/Game/green.png" }));
		stageOrnamentIns.emplace_back(PolygonInstanceRegister::Ins()->CreateInstance(stageOrnamentBlas[static_cast<int>(stageOrnamentBlas.size()) - 1], PolygonInstanceRegister::SHADER_ID::REFRACTION));
		stageOrnamentBlas.emplace_back(BLASRegister::Ins()->GenerateObj("Resource/Game/stageOrnament/", "blockF.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], { L"Resource/Game/red.png" }));
		stageOrnamentIns.emplace_back(PolygonInstanceRegister::Ins()->CreateInstance(stageOrnamentBlas[static_cast<int>(stageOrnamentBlas.size()) - 1], PolygonInstanceRegister::SHADER_ID::DEF));
		stageOrnamentBlas.emplace_back(BLASRegister::Ins()->GenerateObj("Resource/Game/stageOrnament/", "goalSideObj.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], { L"Resource/Game/gray.png" }));
		stageOrnamentIns.emplace_back(PolygonInstanceRegister::Ins()->CreateInstance(stageOrnamentBlas[static_cast<int>(stageOrnamentBlas.size()) - 1], PolygonInstanceRegister::SHADER_ID::DEF));
	}
	for (auto& index : stageOrnamentIns) {

		PolygonInstanceRegister::Ins()->AddScale(index, Vec3(200, 200, 200));

	}

	// ギミックをロード。
	GenerateGimmick();

	// プレイヤーを初期化。
	Player::StageData stageData;
	stageData.stageBlasIndex = stageBlas;
	stageData.stageInsIndex = stageIns;
	stageData.stageGrassBlasIndex = stageGrassBlas;
	stageData.stageGrassInsIndex = stageGrassIns;
	stageData.middlePointBlasIndex = middlePointCollisionBlas;
	stageData.middlePointInsIndex = middlePointCollisionIns;
	stageData.goalBlasIndex = goalCollisionBlas;
	stageData.goalInsIndex = goalCollisionIns;
	stageData.stageOrnamentBlasIndex = stageOrnamentBlas;
	stageData.stageOrnamentInsIndex = stageOrnamentIns;
	player = std::make_shared<Player>(stageData);

	// ゴール前のふわふわしているオブジェクトのBLASをロード
	beforeTheGoalObjectBlas = BLASRegister::Ins()->GenerateObj("Resource/Game/stageOrnament/", "beforeTheGoalBox.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], { L"Resource/Game/red.png" });
	for (int index = 0; index < 4; ++index) {

		std::pair<int, int> buff;
		std::pair<Vec3, Vec3> defPosBuff;
		buff.first = PolygonInstanceRegister::Ins()->CreateInstance(beforeTheGoalObjectBlas, PolygonInstanceRegister::DEF);
		defPosBuff.first = Vec3(250, 200, 1000 * index + 1500.0f);
		PolygonInstanceRegister::Ins()->AddTrans(buff.first, defPosBuff.first);
		PolygonInstanceRegister::Ins()->AddScale(buff.first, Vec3(40.0f, 40.0f, 40.0f));
		PolygonInstanceRegister::Ins()->AddRotate(buff.first, Vec3(DirectX::XM_2PI / 2.0f * index, DirectX::XM_2PI / 2.0f * index, 0));
		buff.second = PolygonInstanceRegister::Ins()->CreateInstance(beforeTheGoalObjectBlas, PolygonInstanceRegister::DEF);
		defPosBuff.second = Vec3(-250, 200, 1000 * index + 1500.0f);
		PolygonInstanceRegister::Ins()->AddTrans(buff.second, defPosBuff.second);
		PolygonInstanceRegister::Ins()->AddScale(buff.second, Vec3(40.0f, 40.0f, 40.0f));
		PolygonInstanceRegister::Ins()->AddRotate(buff.second, Vec3(DirectX::XM_2PI / 2.0f * index, DirectX::XM_2PI / 2.0f * index, 0));
		beforeTheGoalObjectIns.emplace_back(buff);
		beforeTheGoalObjectDefPos.emplace_back(defPosBuff);
		beforeTheGoalObjectTimer.push_back(1.0f * index);

	}

	// Instanceのワールド行列を生成。
	PolygonInstanceRegister::Ins()->CalWorldMat();

	// TLASを生成。
	tlas = std::make_shared<TLAS>();
	tlas->GenerateTLAS();

	// AO出力用クラスをセット。
	aoOutput = std::make_shared<RaytracingOutput>();
	aoOutput->Setting(DXGI_FORMAT_R8G8B8A8_UNORM, L"AOOutput");
	denoiseAOOutput = std::make_shared<RaytracingOutput>();
	denoiseAOOutput->Setting(DXGI_FORMAT_R8G8B8A8_UNORM, L"DenoiseAOOutput");

	// 色出力用クラスをセット。
	colorOutput = std::make_shared<RaytracingOutput>();
	colorOutput->Setting(DXGI_FORMAT_R8G8B8A8_UNORM, L"ColorOutput");

	// 明るさ情報出力用クラスをセット。
	lightOutput = std::make_shared<RaytracingOutput>();
	lightOutput->Setting(DXGI_FORMAT_R8G8B8A8_UNORM, L"LightOutput");
	denoiseLightOutput = std::make_shared<RaytracingOutput>();
	denoiseLightOutput->Setting(DXGI_FORMAT_R8G8B8A8_UNORM, L"DenoiseLightOutput");

	// GI出力用クラスをセット。
	giOutput = std::make_shared<RaytracingOutput>();
	giOutput->Setting(DXGI_FORMAT_R8G8B8A8_UNORM, L"GIOutput");
	denoiseGiOutput = std::make_shared<RaytracingOutput>();
	denoiseGiOutput->Setting(DXGI_FORMAT_R8G8B8A8_UNORM, L"DenoiseGIOutput");

	// デノイズマスク用クラスをセット。
	denoiseMaskOutput = std::make_shared<RaytracingOutput>();
	denoiseMaskOutput->Setting(DXGI_FORMAT_R8G8B8A8_UNORM, L"DenoiseMaskOutput");

	// 最終出力用クラスをセット。
	denoiseMixTextureOutput = std::make_shared<RaytracingOutput>();
	denoiseMixTextureOutput->Setting(DXGI_FORMAT_R8G8B8A8_UNORM, L"DenoiseMixTextureOutput");

	// シェーダーテーブルを生成。
	pipline->ConstructionShaderTable();

	// 太陽に関する変数
	sunAngle = 0;
	sunSpeed = 0.0001f;

	isDisplayFPS = false;

	nextScene = SCENE_ID::RESULT;
	isTransition = false;

	isPassedMiddlePoint = false;
	rapCount = 0;

	// スプライトを生成。
	nowRapCountSprite = std::make_shared<Sprite>();
	nowRapCountSprite->GenerateSpecifyTextureID(Vec3(95, 80, 0.1f), Vec2(16.0f, 32.0f), Pipline::PROJECTIONID::UI, Pipline::PIPLINE_ID::PIPLINE_SPRITE_ALPHA, 0);
	maxRapCountSprite = std::make_shared<Sprite>();
	maxRapCountSprite->GenerateSpecifyTextureID(Vec3(180, 80, 0.1f), Vec2(16.0f, 32.0f), Pipline::PROJECTIONID::UI, Pipline::PIPLINE_ID::PIPLINE_SPRITE_ALPHA, 0);
	rapSlashSprite = std::make_shared<Sprite>();
	rapSlashSprite->GenerateSpecifyTextureID(Vec3(140, 80, 0.1f), Vec2(16.0f, 32.0f), Pipline::PROJECTIONID::UI, Pipline::PIPLINE_ID::PIPLINE_SPRITE_ALPHA, 0);

	// フォントをロード
	{
		numFontHandle[0] = TextureManager::Ins()->LoadTexture(L"Resource/Game/Font/0.png");
		numFontHandle[1] = TextureManager::Ins()->LoadTexture(L"Resource/Game/Font/1.png");
		numFontHandle[2] = TextureManager::Ins()->LoadTexture(L"Resource/Game/Font/2.png");
		numFontHandle[3] = TextureManager::Ins()->LoadTexture(L"Resource/Game/Font/3.png");
		numFontHandle[4] = TextureManager::Ins()->LoadTexture(L"Resource/Game/Font/4.png");
		numFontHandle[5] = TextureManager::Ins()->LoadTexture(L"Resource/Game/Font/5.png");
		numFontHandle[6] = TextureManager::Ins()->LoadTexture(L"Resource/Game/Font/6.png");
		numFontHandle[7] = TextureManager::Ins()->LoadTexture(L"Resource/Game/Font/7.png");
		numFontHandle[8] = TextureManager::Ins()->LoadTexture(L"Resource/Game/Font/8.png");
		numFontHandle[9] = TextureManager::Ins()->LoadTexture(L"Resource/Game/Font/9.png");
		numFontHandle[10] = TextureManager::Ins()->LoadTexture(L"Resource/Game/Font/slash.png");
	}

}

void GameScene::Init()
{

	/*===== 初期化処理 =====*/

	nextScene = SCENE_ID::RESULT;
	isTransition = false;
	player->Init();
	Camera::Ins()->Init();

	isPassedMiddlePoint = false;
	rapCount = 0;
	sunAngle = 0;

}

void GameScene::Update()
{

	/*===== 更新処理 =====*/

	// 入力処理
	Input();

	// ウィンドウの名前を更新。
	if (isDisplayFPS) {

		FPS();

	}
	else {

		SetWindowText(DirectXBase::Ins()->windowsAPI->hwnd, L"LE3A_21_フナクラベ_タクミ");

	}

	// 中間地点のオブジェクトを元の大きさに戻す。 AOの影でてしまうバグの対策用。シェーダー側で変える時間がなかったので臨時でずらしてます！
	PolygonInstanceRegister::Ins()->ChangeScale(middlePointCollisionIns, Vec3(200, 200, 200));
	PolygonInstanceRegister::Ins()->ChangeTrans(middlePointCollisionIns, Vec3(0, 0, 0));

	// プレイヤーを更新。
	player->Update(constBufferData, isPassedMiddlePoint, rapCount);

	// 乱数の種を更新。
	constBufferData.debug.seed = FHelper::GetRand(0, 1000);

	// カメラを更新。
	Camera::Ins()->Update(player->GetPos(), player->GetForwardVec(), player->GetUpVec(), player->GetNowSpeedPer());


	// 中間地点に達していたらゴールを定位置に出す。
	if (isPassedMiddlePoint) {

		PolygonInstanceRegister::Ins()->ChangeTrans(goalIns, GOAL_DEF_POS);
		PolygonInstanceRegister::Ins()->ChangeScale(goalIns, Vec3(200, 200, 200));
		PolygonInstanceRegister::Ins()->ChangeTrans(goalCollisionIns, GOAL_DEF_POS);
		PolygonInstanceRegister::Ins()->ChangeScale(goalCollisionIns, Vec3(200, 200, 200));

	}
	else {

		PolygonInstanceRegister::Ins()->ChangeTrans(goalIns, Vec3(0, -100, 0));
		PolygonInstanceRegister::Ins()->ChangeScale(goalIns, Vec3(0, 0, 0));
		PolygonInstanceRegister::Ins()->ChangeTrans(goalCollisionIns, Vec3(0, -100, 0));
		PolygonInstanceRegister::Ins()->ChangeScale(goalCollisionIns, Vec3(0, 0, 0));

	}

	// 3週していたらリザルトシーンに移動する。
	if (3 <= rapCount) {

		isTransition = true;

	}

	// ラップ数のUIを更新。
	maxRapCountSprite->ChangeTextureID(numFontHandle[3], 0);
	nowRapCountSprite->ChangeTextureID(numFontHandle[rapCount], 0);
	rapSlashSprite->ChangeTextureID(numFontHandle[10], 0);

	// ゴールオブジェクトを回転させる。
	PolygonInstanceRegister::Ins()->AddRotate(goalIns, Vec3(0.01f, 0, 0));

	// ゴール前のオブジェクトを回転させる。
	static const int BEFORE_THE_GOAL_OBJECT_COUNT = 4;
	for (int index = 0; index < BEFORE_THE_GOAL_OBJECT_COUNT; ++index) {

		const float SIN_WAVE_MOVE = 50.0f;

		PolygonInstanceRegister::Ins()->AddRotate(beforeTheGoalObjectIns[index].first, Vec3(0.01f, 0.01f, 0));
		PolygonInstanceRegister::Ins()->AddRotate(beforeTheGoalObjectIns[index].second, Vec3(0.01f, 0.01f, 0));
		PolygonInstanceRegister::Ins()->ChangeTrans(beforeTheGoalObjectIns[index].first, beforeTheGoalObjectDefPos[index].first + Vec3(0, sinf(beforeTheGoalObjectTimer[index]) * SIN_WAVE_MOVE, 0));
		PolygonInstanceRegister::Ins()->ChangeTrans(beforeTheGoalObjectIns[index].second, beforeTheGoalObjectDefPos[index].second + Vec3(0, sinf(beforeTheGoalObjectTimer[index]) * SIN_WAVE_MOVE, 0));

		beforeTheGoalObjectTimer[index] += 0.05f;

	}

	// 中間地点のオブジェクトを小さくする。 AOの影でてしまうバグの対策用。シェーダー側で変える時間がなかったので臨時で小さくしています！
	PolygonInstanceRegister::Ins()->ChangeScale(middlePointCollisionIns, Vec3(0, 0, 0));
	PolygonInstanceRegister::Ins()->ChangeTrans(middlePointCollisionIns, Vec3(0, -100, 0));


	tlas->Update();

	// 太陽の角度を更新。
	sunAngle += sunSpeed;
	constBufferData.light.dirLight.lihgtDir = Vec3(-cos(sunAngle), -sin(sunAngle), 0.5f);
	constBufferData.light.dirLight.lihgtDir.Normalize();
	// 天球自体も回転させる。
	PolygonInstanceRegister::Ins()->AddRotate(skyDomeIns, Vec3(0.001f, 0, 0));

}

void GameScene::Draw()
{

	/*===== 描画処理 =====*/

	// カメラ行列を更新。
	auto frameIndex = DirectXBase::Ins()->swapchain->GetCurrentBackBufferIndex();
	constBufferData.camera.mtxView = Camera::Ins()->matView;
	constBufferData.camera.mtxViewInv = DirectX::XMMatrixInverse(nullptr, constBufferData.camera.mtxView);
	constBufferData.camera.mtxProj = Camera::Ins()->matPerspective;
	constBufferData.camera.mtxProjInv = DirectX::XMMatrixInverse(nullptr, constBufferData.camera.mtxProj);

	// 定数バッファをセット。
	constBuffer->Write(DirectXBase::Ins()->swapchain->GetCurrentBackBufferIndex(), &constBufferData, sizeof(constBufferData));


	// グローバルルートシグネチャで使うと宣言しているリソースらをセット。
	ID3D12DescriptorHeap* descriptorHeaps[] = { DescriptorHeapMgr::Ins()->GetDescriptorHeap().Get() };
	DirectXBase::Ins()->cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	DirectXBase::Ins()->cmdList->SetComputeRootSignature(pipline->GetGlobalRootSig()->GetRootSig().Get());

	// TLASを設定。
	DirectXBase::Ins()->cmdList->SetComputeRootDescriptorTable(0, DescriptorHeapMgr::Ins()->GetGPUHandleIncrement(tlas->GetDescriptorHeapIndex()));

	// 定数バッファをセット
	DirectXBase::Ins()->cmdList->SetComputeRootConstantBufferView(1, constBuffer->GetBuffer(frameIndex)->GetGPUVirtualAddress());

	// バリアを設定し各リソースの状態を遷移させる.
	aoOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	denoiseAOOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	lightOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	denoiseLightOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	colorOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	giOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	denoiseGiOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	denoiseMaskOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// 出力用UAVを設定。
	aoOutput->SetComputeRootDescriptorTalbe(2);		// AOの結果出力用
	lightOutput->SetComputeRootDescriptorTalbe(3);	// ライトの明るさの結果出力用
	colorOutput->SetComputeRootDescriptorTalbe(4);	// テクスチャの色情報出力用
	giOutput->SetComputeRootDescriptorTalbe(5);		// giの結果出力用
	denoiseMaskOutput->SetComputeRootDescriptorTalbe(6);// デノイズをする際のマスク出力用

	// パイプラインを設定。
	DirectXBase::Ins()->cmdList->SetPipelineState1(pipline->GetStateObject().Get());

	// レイトレーシングを実行。
	D3D12_DISPATCH_RAYS_DESC rayDesc = pipline->GetDispatchRayDesc();
	DirectXBase::Ins()->cmdList->DispatchRays(&rayDesc);

	// [ノイズを描画]のときはデノイズをかけない。
	if (!constBufferData.debug.isNoiseScene) {

		// デバッグ機能で[法線描画][メッシュ描画][ライトに当たった点のみ描画]のときはデノイズをかけないようにする。
		if (!constBufferData.debug.isMeshScene && !constBufferData.debug.isNormalScene && !constBufferData.debug.isLightHitScene) {

			D3D12_RESOURCE_BARRIER barrierToUAV[] = { CD3DX12_RESOURCE_BARRIER::UAV(
				lightOutput->GetRaytracingOutput().Get()),CD3DX12_RESOURCE_BARRIER::UAV(
				denoiseLightOutput->GetRaytracingOutput().Get()),CD3DX12_RESOURCE_BARRIER::UAV(
				denoiseMaskOutput->GetRaytracingOutput().Get())
			};

			DirectXBase::Ins()->cmdList->ResourceBarrier(3, barrierToUAV);

			// ライトにデノイズをかける。
			Denoiser::Ins()->Denoise(lightOutput->GetUAVIndex(), denoiseLightOutput->GetUAVIndex(), denoiseMaskOutput->GetUAVIndex(), 1, 1);

		}

		// AO情報にデノイズをかける。
		{
			D3D12_RESOURCE_BARRIER barrierToUAV[] = { CD3DX12_RESOURCE_BARRIER::UAV(
				aoOutput->GetRaytracingOutput().Get()),CD3DX12_RESOURCE_BARRIER::UAV(
				denoiseAOOutput->GetRaytracingOutput().Get()),CD3DX12_RESOURCE_BARRIER::UAV(
				denoiseMaskOutput->GetRaytracingOutput().Get())
			};

			DirectXBase::Ins()->cmdList->ResourceBarrier(3, barrierToUAV);

			// AOにデノイズをかける。
			Denoiser::Ins()->Denoise(aoOutput->GetUAVIndex(), denoiseAOOutput->GetUAVIndex(), denoiseMaskOutput->GetUAVIndex(), 100, 6);
		}


		// GI情報にデノイズをかける。
		{
			D3D12_RESOURCE_BARRIER barrierToUAV[] = { CD3DX12_RESOURCE_BARRIER::UAV(
				giOutput->GetRaytracingOutput().Get()),CD3DX12_RESOURCE_BARRIER::UAV(
				denoiseGiOutput->GetRaytracingOutput().Get()),CD3DX12_RESOURCE_BARRIER::UAV(
				denoiseMaskOutput->GetRaytracingOutput().Get())
			};

			DirectXBase::Ins()->cmdList->ResourceBarrier(3, barrierToUAV);

			// GIにデノイズをかける。
			Denoiser::Ins()->Denoise(giOutput->GetUAVIndex(), denoiseGiOutput->GetUAVIndex(), denoiseMaskOutput->GetUAVIndex(), 100, 1);

		}

		denoiseMixTextureOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		D3D12_RESOURCE_BARRIER barrierToUAV[] = { CD3DX12_RESOURCE_BARRIER::UAV(
			colorOutput->GetRaytracingOutput().Get()),CD3DX12_RESOURCE_BARRIER::UAV(
			denoiseAOOutput->GetRaytracingOutput().Get()),CD3DX12_RESOURCE_BARRIER::UAV(
			denoiseLightOutput->GetRaytracingOutput().Get()),CD3DX12_RESOURCE_BARRIER::UAV(
			denoiseGiOutput->GetRaytracingOutput().Get()),CD3DX12_RESOURCE_BARRIER::UAV(
			denoiseMixTextureOutput->GetRaytracingOutput().Get())
		};

		DirectXBase::Ins()->cmdList->ResourceBarrier(5, barrierToUAV);

		// デノイズをかけたライティング情報と色情報を混ぜる。
		Denoiser::Ins()->MixColorAndLuminance(colorOutput->GetUAVIndex(), denoiseAOOutput->GetUAVIndex(), denoiseLightOutput->GetUAVIndex(), denoiseGiOutput->GetUAVIndex(), denoiseMixTextureOutput->GetUAVIndex());
		denoiseMixTextureOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

	}
	// デノイズしないデバッグ状態の場合は、レイトレ関数から出力された生の値を合成する。
	else {


		denoiseMixTextureOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		D3D12_RESOURCE_BARRIER barrierToUAV[] = { CD3DX12_RESOURCE_BARRIER::UAV(
			colorOutput->GetRaytracingOutput().Get()),CD3DX12_RESOURCE_BARRIER::UAV(
			aoOutput->GetRaytracingOutput().Get()),CD3DX12_RESOURCE_BARRIER::UAV(
			lightOutput->GetRaytracingOutput().Get()),CD3DX12_RESOURCE_BARRIER::UAV(
			giOutput->GetRaytracingOutput().Get()),CD3DX12_RESOURCE_BARRIER::UAV(
			denoiseMixTextureOutput->GetRaytracingOutput().Get())
		};

		DirectXBase::Ins()->cmdList->ResourceBarrier(5, barrierToUAV);

		// デノイズをかけたライティング情報と色情報を混ぜる。
		Denoiser::Ins()->MixColorAndLuminance(colorOutput->GetUAVIndex(), aoOutput->GetUAVIndex(), lightOutput->GetUAVIndex(), giOutput->GetUAVIndex(), denoiseMixTextureOutput->GetUAVIndex());
		denoiseMixTextureOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

	}


	// バックバッファのインデックスを取得する。
	UINT backBufferIndex = DirectXBase::Ins()->swapchain->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
		DirectXBase::Ins()->backBuffers[backBufferIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_COPY_DEST),
	};
	DirectXBase::Ins()->cmdList->ResourceBarrier(_countof(barriers), barriers);

	// デバッグ情報によって描画するデータを変える。
	if (constBufferData.debug.isLightHitScene || constBufferData.debug.isMeshScene || constBufferData.debug.isNormalScene) {

		// デノイズされた通常の描画
		lightOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		DirectXBase::Ins()->cmdList->CopyResource(DirectXBase::Ins()->backBuffers[backBufferIndex].Get(), lightOutput->GetRaytracingOutput().Get());
		lightOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	}
	else {

		DirectXBase::Ins()->cmdList->CopyResource(DirectXBase::Ins()->backBuffers[backBufferIndex].Get(), denoiseMixTextureOutput->GetRaytracingOutput().Get());

	}

	// レンダーターゲットのリソースバリアをもとに戻す。
	D3D12_RESOURCE_BARRIER endBarriers[] = {

	CD3DX12_RESOURCE_BARRIER::Transition(
	DirectXBase::Ins()->backBuffers[backBufferIndex].Get(),
	D3D12_RESOURCE_STATE_COPY_DEST,
	D3D12_RESOURCE_STATE_RENDER_TARGET)

	};

	DirectXBase::Ins()->cmdList->ResourceBarrier(_countof(endBarriers), endBarriers);

	// バリアを設定し各リソースの状態を遷移させる.
	aoOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	denoiseAOOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	lightOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	denoiseLightOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	colorOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	giOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	denoiseGiOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	denoiseMaskOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

	// UIを描画
	nowRapCountSprite->Draw();
	maxRapCountSprite->Draw();
	rapSlashSprite->Draw();

}

#pragma comment (lib, "winmm.lib")

void GameScene::FPS()
{

	/*===== タイトルバーにFPS表示 =====*/

	static DWORD prev_time = timeGetTime();	// 前回の時間
	static int frame_count = 0;		// フレームカウント
	DWORD now_time = timeGetTime();		// 今回のフレームの時間

	frame_count++;	// フレーム数をカウントする

	// 経過時間が１秒を超えたらカウントと時間をリセット
	if (now_time - prev_time >= 1000)
	{
		wchar_t fps[1000];
		_itow_s(frame_count, fps, 10);
		wchar_t moji[] = L"FPS";
		wcscat_s(fps, moji);
		SetWindowText(DirectXBase::Ins()->windowsAPI->hwnd, fps);
		//OutputDebugString(fps);

		prev_time = now_time;
		frame_count = 0;
	}

}

void GameScene::Input()
{

	/*===== 入力処理 =====*/

	if (Input::Ins()->IsPadBottomTrigger(XINPUT_GAMEPAD_A) || Input::Ins()->IsKeyTrigger(DIK_RETURN)) {

		isTransition = true;

	}

	InputImGUI();

}

void GameScene::InputImGUI()
{

	/*===== IMGUI更新 =====*/

	// 太陽の移動速度を更新。
	ImGui::SliderFloat("Sun Speed", &sunSpeed, 0.0f, 0.1f, "%.5f");

	// メッシュを表示する。
	bool isMesh = constBufferData.debug.isMeshScene;
	ImGui::Checkbox("Mesh Scene", &isMesh);
	constBufferData.debug.isMeshScene = isMesh;

	// 法線を表示する。
	bool isNormal = constBufferData.debug.isNormalScene;
	ImGui::Checkbox("Normal Scene", &isNormal);
	constBufferData.debug.isNormalScene = isNormal;

	// ライトがあたった面だけ表示するフラグを更新。
	bool isLightHit = constBufferData.debug.isLightHitScene;
	ImGui::Checkbox("LightHit Scene", &isLightHit);
	constBufferData.debug.isLightHitScene = isLightHit;

	// デバッグ用でノイズ画面を出すためのフラグをセット。
	bool isNoise = constBufferData.debug.isNoiseScene;
	ImGui::Checkbox("Noise Scene", &isNoise);
	constBufferData.debug.isNoiseScene = isNoise;

	// AOを行うかのフラグをセット。
	bool isNoAO = constBufferData.debug.isNoAO;
	ImGui::Checkbox("NoAO Scene", &isNoAO);
	constBufferData.debug.isNoAO = isNoAO;

	// GIを行うかのフラグをセット。
	bool isNoGI = constBufferData.debug.isNoGI;
	ImGui::Checkbox("NoGI Scene", &isNoGI);
	constBufferData.debug.isNoGI = isNoGI;

	// GIのみを描画するかのフラグをセット。
	bool isGIOnlyScene = constBufferData.debug.isGIOnlyScene;
	ImGui::Checkbox("GIOnly Scene", &isGIOnlyScene);
	constBufferData.debug.isGIOnlyScene = isGIOnlyScene;

	// FPSを表示するかのフラグをセット。
	//ImGui::Checkbox("Display FPS", &isDisplayFPS);

	//// 座標を更新。
	//Vec3 gimmickPos = PolygonInstanceRegister::Ins()->GetPos(GimmickMgr::Ins()->GetGimmickData()[boostGimmickTest]->GetINSTANCEIndex());

	//ImGui::DragFloat("PosX", &gimmickPos.x, 1);
	//ImGui::DragFloat("PosY", &gimmickPos.y, 1);
	//ImGui::DragFloat("PosZ", &gimmickPos.z, 1);

	//// 回転を更新。
	//Vec3 gimmickRot = PolygonInstanceRegister::Ins()->GetRotVec3(GimmickMgr::Ins()->GetGimmickData()[boostGimmickTest]->GetINSTANCEIndex());

	//ImGui::DragFloat("RotX", &gimmickRot.x, 0.01f);
	//ImGui::DragFloat("RotY", &gimmickRot.y, 0.01f);
	//ImGui::DragFloat("RotZ", &gimmickRot.z, 0.01f);

	//// 大きさを更新。
	//Vec3 gimmickScale = PolygonInstanceRegister::Ins()->GetScaleVec3(GimmickMgr::Ins()->GetGimmickData()[boostGimmickTest]->GetINSTANCEIndex());

	//ImGui::DragFloat("ScaleX", &gimmickScale.x, 1);
	//ImGui::DragFloat("ScaleY", &gimmickScale.y, 1);
	//ImGui::DragFloat("ScaleZ", &gimmickScale.z, 1);

	//GimmickMgr::Ins()->ChangeTrans(boostGimmickTest, gimmickPos);
	//GimmickMgr::Ins()->ChangeRotate(boostGimmickTest, gimmickRot);
	//GimmickMgr::Ins()->ChangeScale(boostGimmickTest, gimmickScale);

	// 1個目
	GimmickMgr::Ins()->ChangeTrans(0, Vec3(100, -15, 1400));
	GimmickMgr::Ins()->ChangeRotate(0, Vec3(0, 0, 0));
	GimmickMgr::Ins()->ChangeScale(0, Vec3(100, 200, 200));

	// 2個目
	GimmickMgr::Ins()->ChangeTrans(1, Vec3(-80, -15, 3000));
	GimmickMgr::Ins()->ChangeRotate(1, Vec3(0, 0, 0));
	GimmickMgr::Ins()->ChangeScale(1, Vec3(100, 200, 200));

	// 3個目
	GimmickMgr::Ins()->ChangeTrans(2, Vec3(100, -15, 4000));
	GimmickMgr::Ins()->ChangeRotate(2, Vec3(0, 0, 0));
	GimmickMgr::Ins()->ChangeScale(2, Vec3(100, 200, 200));

	// 4個目
	GimmickMgr::Ins()->ChangeTrans(3, Vec3(-5842, -29, -167));
	GimmickMgr::Ins()->ChangeRotate(3, Vec3(0, 1.48f, 0));
	GimmickMgr::Ins()->ChangeScale(3, Vec3(200, 200, 200));

	// 5個目
	GimmickMgr::Ins()->ChangeTrans(4, Vec3(-2352, -18, 6336));
	GimmickMgr::Ins()->ChangeRotate(4, Vec3(0, 1.58f, 0));
	GimmickMgr::Ins()->ChangeScale(4, Vec3(190, 200, 200));


}

void GameScene::GenerateGimmick()
{

	// 1個目
	GimmickMgr::Ins()->AddGimmick(BaseGimmick::ID::BOOST, "Resource/Game/", "goal.obj", { L"Resource/Game/yellow.png" }, HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], PolygonInstanceRegister::SHADER_ID::DEF);

	// 2個目
	GimmickMgr::Ins()->AddGimmick(BaseGimmick::ID::BOOST, "Resource/Game/", "goal.obj", { L"Resource/Game/yellow.png" }, HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], PolygonInstanceRegister::SHADER_ID::DEF);
	
	// 3個目
	GimmickMgr::Ins()->AddGimmick(BaseGimmick::ID::BOOST, "Resource/Game/", "goal.obj", { L"Resource/Game/yellow.png" }, HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], PolygonInstanceRegister::SHADER_ID::DEF);
	
	// 4個目
	GimmickMgr::Ins()->AddGimmick(BaseGimmick::ID::BOOST, "Resource/Game/", "goal.obj", { L"Resource/Game/yellow.png" }, HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], PolygonInstanceRegister::SHADER_ID::DEF);
	
	// 5個目
	GimmickMgr::Ins()->AddGimmick(BaseGimmick::ID::BOOST, "Resource/Game/", "goal.obj", { L"Resource/Game/yellow.png" }, HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], PolygonInstanceRegister::SHADER_ID::DEF);
	
}
