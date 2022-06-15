#include "DevDXR.h"

void DevDXR::Init() {


	constBufferData.Init();
	constBuff.Generate(sizeof(KariConstBufferData), L"constBuffer");
	constBuff.Write(DirectXBase::Ins()->swapchain->GetCurrentBackBufferIndex(), &constBufferData, sizeof(KariConstBufferData));

	// デノイズAO用のパイプラインを設定。
	dAOuseShaders.push_back({ "Resource/ShaderFiles/RayTracing/DenoiseAOShader.hlsl", {L"mainRayGen"}, {L"mainMS", L"shadowMS"}, {L"mainCHS", L"mainAnyHit"} });
	deAOPipline.Setting(dAOuseShaders, HitGroupMgr::DENOISE_AO_HIT_GROUP, 1, 1, 4, sizeof(DirectX::XMFLOAT3) + sizeof(DirectX::XMFLOAT3) + sizeof(DirectX::XMFLOAT3) + sizeof(DirectX::XMFLOAT3) + sizeof(UINT), sizeof(DirectX::XMFLOAT2));

	// SPONZAを読み込む。
	//sponzaInstance = MultiMeshLoadOBJ::Ins()->RayMultiMeshLoadOBJ("Resource/", "sponza.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP]);

	// ライト用のスフィアを読み込む。
	//sphereBlas = BLASRegister::Ins()->GenerateObj("Resource/", "sphere.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], { L"Resource/white.png" });
	//sphereIns = PorygonInstanceRegister::Ins()->CreateInstance(sphereBlas, 1);
	//PorygonInstanceRegister::Ins()->AddScale(sphereIns, Vec3(10, 10, 10));
	//PorygonInstanceRegister::Ins()->ChangeTrans(sphereIns, Vec3(0, 300, 0));

	// 天球用のスフィアを生成する。
	skyDomeBlas = BLASRegister::Ins()->GenerateObj("Resource/", "skydome.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], { L"Resource/skydome.png" });
	skyDomeIns = PorygonInstanceRegister::Ins()->CreateInstance(skyDomeBlas, 1);
	PorygonInstanceRegister::Ins()->AddScale(skyDomeIns, Vec3(1000, 1000, 1000));

	PorygonInstanceRegister::Ins()->CalWorldMat();

	// TLASを生成。
	tlas.GenerateTLAS(L"TlasDescriptorHeap");

	// レイトレ出力用クラスをセット。
	aoOutput.Setting(DXGI_FORMAT_R8G8B8A8_UNORM);

	// レイトレ出力用クラスをセット。
	colorOutput.Setting(DXGI_FORMAT_R8G8B8A8_UNORM);

	// 明るさ情報出力用クラスをセット。
	lightOutput.Setting(DXGI_FORMAT_R8G8B8A8_UNORM);

	// gi出力用クラスをセット。
	giOutput.Setting(DXGI_FORMAT_R8G8B8A8_UNORM);

	// デノイズの結果出力用クラスをセット。
	denoiseMixTextureOutput.Setting(DXGI_FORMAT_R8G8B8A8_UNORM);

	// シェーダーテーブルを生成。
	deAOPipline.ConstructionShaderTable();

	// デバッグ用でノイズ画面を出すフラグ。
	debugPiplineID = DENOISE_AO_PIPLINE;

	// ライトが動いたか
	isMoveLight = false;

	// デバッグ用のパイプラインを切り替えるやつ。
	enum DEGU_PIPLINE_ID {
		DEF_PIPLINE,
		AO_PIPLINE,
		DENOISE_AO_PIPLINE,
	};

	isDisplayFPS = false;

}

void DevDXR::Update() {

	// IMGUI系
	ImGuiWindow::Ins()->processBeforeDrawing();

	// ウィンドウの名前を再設定。
	SetWindowText(ImGuiWindow::Ins()->windowsAPI.hwnd, L"ImGuiWindow");

	isMoveLight = false;
	Input(constBufferData, isMoveLight, debugPiplineID);

	ImGuiWindow::Ins()->processAfterDrawing();


	/*----------毎フレーム処理(描画前処理)----------*/
	DirectXBase::Ins()->processBeforeDrawing();

	/*----- 更新処理 -----*/

	// 天球を回転させる。
	//PorygonInstanceRegister::Ins()->AddRotate(skyDomeIns, { 0.01,0.01,0.01 });

	// ウィンドウの名前を更新。
	if (isDisplayFPS) {

		FPS();

	}
	else {

		SetWindowText(DirectXBase::Ins()->windowsAPI.hwnd, L"LE3A_21_フナクラベ_タクミ");

	}

	// ビュー行列を生成。
	Camera::Ins()->GenerateMatView();

	// 乱数の種を更新。
	constBufferData.seed = FHelper::GetRand(0, 1000);

	// カメラを更新。
	Camera::Ins()->Update();

	constBufferData.eye = Camera::Ins()->eye;
	constBufferData.target = Camera::Ins()->target;
	constBufferData.up = Camera::Ins()->up;

	// ライトが動いたときのみ、ワールド行列を再計算してTLASを更新する。
	//if (isMoveLight) {

		// 点光源の位置を更新。
		PorygonInstanceRegister::Ins()->ChangeTrans(sphereIns, constBufferData.pointLight.lightPos);
		PorygonInstanceRegister::Ins()->ChangeScale(sphereIns, constBufferData.pointLight.lightSize);

		tlas.Update();

	//}

	/*----- 描画処理 -----*/

	// 画面に表示されるレンダーターゲットに戻す。
	//DirectXBase::Ins()->SetRenderTarget();

}

void DevDXR::Draw() {


	RaytracingPipline setPipline = {};

	// TLAS更新。
	//tlas.Update();

	// デバッグ用のパイプラインIDに応じたパイプラインをセットする。
	setPipline = deAOPipline;

	// カメラ行列を更新。
	auto frameIndex = DirectXBase::Ins()->swapchain->GetCurrentBackBufferIndex();
	constBufferData.mtxView = XMMatrixLookAtLH(constBufferData.eye.ConvertXMVECTOR(), constBufferData.target.ConvertXMVECTOR(), constBufferData.up.ConvertXMVECTOR());
	constBufferData.mtxViewInv = XMMatrixInverse(nullptr, constBufferData.mtxView);

	// 定数バッファの中身を更新する。
	constBuff.Write(frameIndex, &constBufferData, sizeof(KariConstBufferData));
	auto sceneConstantBuffer = constBuff.GetBuffer(frameIndex);

	// バリアを設定し各リソースの状態を遷移させる.
	aoOutput.SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	lightOutput.SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	colorOutput.SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	giOutput.SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// グローバルルートシグネチャで使うと宣言しているリソースらをセット。
	ID3D12DescriptorHeap* descriptorHeaps[] = { DescriptorHeapMgr::Ins()->GetDescriptorHeap().Get() };
	DirectXBase::Ins()->cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	DirectXBase::Ins()->cmdList->SetComputeRootSignature(setPipline.GetGlobalRootSig()->GetRootSig().Get());

	// TLASを設定。
	DirectXBase::Ins()->cmdList->SetComputeRootDescriptorTable(0, DescriptorHeapMgr::Ins()->GetGPUHandleIncrement(tlas.GetDescriptorHeapIndex()));

	// 出力用UAVを設定。
	lightOutput.SetComputeRootDescriptorTalbe(2);
	aoOutput.SetComputeRootDescriptorTalbe(3);
	colorOutput.SetComputeRootDescriptorTalbe(4);
	giOutput.SetComputeRootDescriptorTalbe(5);
	DirectXBase::Ins()->cmdList->SetComputeRootConstantBufferView(1, sceneConstantBuffer->GetGPUVirtualAddress());

	// パイプラインを設定。
	DirectXBase::Ins()->cmdList->SetPipelineState1(setPipline.GetStateObject().Get());

	// レイトレーシングを実行。
	DirectXBase::Ins()->cmdList->DispatchRays(&setPipline.GetDispatchRayDesc());

	// デバッグ用のパイプラインがデノイズ用パイプラインだったら、コンピュートシェーダーを使ってデノイズをかける。
	if (debugPiplineID == DENOISE_AO_PIPLINE) {

		//// [ノイズを描画]のときはデノイズをかけない。
		//if (!constBufferData.isNoiseScene) {

		//	// デバッグ機能で[法線描画][メッシュ描画][ライトに当たった点のみ描画]のときはデノイズをかけないようにする。
		//	if (!constBufferData.isMeshScene && !constBufferData.isNormalScene && !constBufferData.isLightHitScene) {

		//		// ライトにデノイズをかける。
		//		Denoiser::Ins()->Denoise(lightOutput.GetUAVIndex(), 1, 3);

		//	}

		//	// [AOを行わない]のときはデノイズをかけない。
		//	if (!constBufferData.isNoAO) {

		//		// AOにデノイズをかける。
		//		Denoiser::Ins()->Denoise(aoOutput.GetUAVIndex(), 100, 9);

		//	}

		//	// [GIを行わない]のときはデノイズをかけない。
		//	if (!constBufferData.isNoGI) {

		//		// GIにデノイズをかける。
		//		Denoiser::Ins()->Denoise(giOutput.GetUAVIndex(), 100, 5);

		//	}

		//}

		//// デノイズをかけたライティング情報と色情報を混ぜる。
		denoiseMixTextureOutput.SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		//Denoiser::Ins()->MixColorAndLuminance(colorOutput.GetUAVIndex(), aoOutput.GetUAVIndex(), lightOutput.GetUAVIndex(), giOutput.GetUAVIndex(), denoiseMixTextureOutput.GetUAVIndex());
		denoiseMixTextureOutput.SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

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
	if (constBufferData.isLightHitScene || constBufferData.isMeshScene || constBufferData.isNormalScene) {

		// デノイズされた通常の描画
		DirectXBase::Ins()->cmdList->CopyResource(DirectXBase::Ins()->backBuffers[backBufferIndex].Get(), lightOutput.GetRaytracingOutput().Get());

	}
	else {

		// デノイズされた通常の描画
		colorOutput.SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		DirectXBase::Ins()->cmdList->CopyResource(DirectXBase::Ins()->backBuffers[backBufferIndex].Get(), colorOutput.GetRaytracingOutput().Get());

	}

	// レンダーターゲットのリソースバリアをもとに戻す。
	D3D12_RESOURCE_BARRIER endBarriers[] = {

	CD3DX12_RESOURCE_BARRIER::Transition(
	DirectXBase::Ins()->backBuffers[backBufferIndex].Get(),
	D3D12_RESOURCE_STATE_COPY_DEST,
	D3D12_RESOURCE_STATE_RENDER_TARGET)

	};

	// バリアを設定し各リソースの状態を遷移させる.
	aoOutput.SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	lightOutput.SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	//colorOutput.SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	giOutput.SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

	DirectXBase::Ins()->cmdList->ResourceBarrier(_countof(endBarriers), endBarriers);

	DirectXBase::Ins()->processAfterDrawing();

}


// タイトルバーのFPSの更新
void DevDXR::FPS()
{
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
		SetWindowText(DirectXBase::Ins()->windowsAPI.hwnd, fps);
		//OutputDebugString(fps);

		prev_time = now_time;
		frame_count = 0;
	}
}

void DevDXR::Input(KariConstBufferData& constBufferData, bool& isMoveLight, DEGU_PIPLINE_ID& debugPiplineID) {

	float inputValue = 1.0f;
	float x = 1.0f - (inputValue);
	static const float fScaleDepth = 0.25f;
	float buff = fScaleDepth * std::exp(-0.00287f + x * (0.459f + x * (3.83f + x * (-6.8f + x * 5.25f))));

	bool isMove = false;

	float speed = 10.0f;
	float rot = 0.03f;
	if (Input::isKey(DIK_W)) {
		Camera::Ins()->Move(speed);
		isMove = true;
	}
	if (Input::isKey(DIK_S)) {
		Camera::Ins()->Move(-speed);
		isMove = true;
	}
	if (Input::isKey(DIK_A)) {
		Camera::Ins()->MoveRight(speed);
		isMove = true;
	}
	if (Input::isKey(DIK_D)) {
		Camera::Ins()->MoveRight(-speed);
		isMove = true;
	}
	if (Input::isKey(DIK_UP)) {
		Camera::Ins()->forwardVec.y += rot;
		isMove = true;
	}
	if (Input::isKey(DIK_DOWN)) {
		Camera::Ins()->forwardVec.y -= rot;
		isMove = true;
	}
	if (Input::isKey(DIK_LEFT)) {
		Camera::Ins()->AddRotationXZ(rot);
		isMove = true;
	}
	if (Input::isKey(DIK_RIGHT)) {
		Camera::Ins()->AddRotationXZ(-rot);
		isMove = true;
	}
	if (Input::isKey(DIK_LSHIFT)) {
		Camera::Ins()->eye.y -= 10.0f;
		isMove = true;
	}
	if (Input::isKey(DIK_SPACE)) {
		Camera::Ins()->eye.y += 10.0f;
		isMove = true;
	}

	InputImGUI(constBufferData, isMoveLight, debugPiplineID, isMove);

}

void DevDXR::InputImGUI(KariConstBufferData& constBufferData, bool& isMoveLight, DEGU_PIPLINE_ID& debugPiplineID, bool& isMove)
{

	// DirLightについて
	if (ImGui::TreeNode("DirLight")) {

		// ライトを表示するかどうかのフラグを更新。
		bool isActive = static_cast<bool>(constBufferData.dirLight.isActive);
		ImGui::Checkbox("IsActive", &isActive);
		if (isActive != constBufferData.dirLight.isActive) isMove = true;
		constBufferData.dirLight.isActive = static_cast<int>(isActive);

		// 値を保存する。
		float dirX = constBufferData.dirLight.lihgtDir.x;
		float dirY = constBufferData.dirLight.lihgtDir.y;
		float dirZ = constBufferData.dirLight.lihgtDir.z;
		ImGui::SliderFloat("DirLightX", &constBufferData.dirLight.lihgtDir.x, -1.0f, 1.0f);
		ImGui::SliderFloat("DirLightY", &constBufferData.dirLight.lihgtDir.y, -1.0f, 1.0f);
		ImGui::SliderFloat("DirLightZ", &constBufferData.dirLight.lihgtDir.z, -1.0f, 1.0f);

		// 変わっていたら
		if (dirX != constBufferData.dirLight.lihgtDir.x || dirY != constBufferData.dirLight.lihgtDir.y || dirZ != constBufferData.dirLight.lihgtDir.z) {

			isMove = true;
			isMoveLight = true;

		}

		// 正規化する。
		constBufferData.dirLight.lihgtDir.Normalize();

		// ライトの色を設定。
		array<float, 3> lightColor = { constBufferData.dirLight.lightColor.x,constBufferData.dirLight.lightColor.y,constBufferData.dirLight.lightColor.z };
		ImGui::ColorPicker3("LightColor", lightColor.data());
		// 色が変わっていたら。
		if (lightColor[0] != constBufferData.dirLight.lightColor.x || lightColor[1] != constBufferData.dirLight.lightColor.y || lightColor[2] != constBufferData.dirLight.lightColor.z) {
			isMove = true;
		}
		constBufferData.dirLight.lightColor.x = lightColor[0];
		constBufferData.dirLight.lightColor.y = lightColor[1];
		constBufferData.dirLight.lightColor.z = lightColor[2];

		ImGui::TreePop();

	}

	// PointLightについて
	if (ImGui::TreeNode("PointLight")) {

		// ライトを表示するかどうかのフラグを更新。
		bool isActive = static_cast<bool>(constBufferData.pointLight.isActive);
		ImGui::Checkbox("IsActive", &isActive);
		if (isActive != constBufferData.pointLight.isActive) isMove = true;
		constBufferData.pointLight.isActive = static_cast<int>(isActive);

		// 値を保存する。
		float dirX = constBufferData.pointLight.lightPos.x;
		float dirY = constBufferData.pointLight.lightPos.y;
		float dirZ = constBufferData.pointLight.lightPos.z;
		float lightSize = constBufferData.pointLight.lightSize;
		float aoSampleCount = constBufferData.aoSampleCount;
		float pointLightPower = constBufferData.pointLight.lightPower;
		float MOVE_LENGTH = 1500.0f;
		ImGui::SliderFloat("PointLightX", &constBufferData.pointLight.lightPos.x, -MOVE_LENGTH, MOVE_LENGTH);
		ImGui::SliderFloat("PointLightY", &constBufferData.pointLight.lightPos.y, 0.0f, 1000.0f);
		ImGui::SliderFloat("PointLightZ", &constBufferData.pointLight.lightPos.z, -MOVE_LENGTH, MOVE_LENGTH);
		ImGui::SliderFloat("PointLightRadius", &constBufferData.pointLight.lightSize, 1.0f, 50.0f);
		ImGui::SliderFloat("PointLightPower", &constBufferData.pointLight.lightPower, 300.0f, 1000.0f);
		ImGui::SliderFloat("AOSampleCount", &aoSampleCount, 1.0f, 30.0f);
		constBufferData.aoSampleCount = aoSampleCount;

		// 変わっていたら
		if (dirX != constBufferData.pointLight.lightPos.x || dirY != constBufferData.pointLight.lightPos.y || dirZ != constBufferData.pointLight.lightPos.z || lightSize != constBufferData.pointLight.lightSize || pointLightPower != constBufferData.pointLight.lightPower) {

			isMove = true;
			isMoveLight = true;

		}

		// ライトの色を設定。
		array<float, 3> lightColor = { constBufferData.pointLight.lightColor.x,constBufferData.pointLight.lightColor.y,constBufferData.pointLight.lightColor.z };
		ImGui::ColorPicker3("LightColor", lightColor.data());
		// 色が変わっていたら。
		if (lightColor[0] != constBufferData.pointLight.lightColor.x || lightColor[1] != constBufferData.pointLight.lightColor.y || lightColor[2] != constBufferData.pointLight.lightColor.z) {
			isMove = true;
		}
		constBufferData.pointLight.lightColor.x = lightColor[0];
		constBufferData.pointLight.lightColor.y = lightColor[1];
		constBufferData.pointLight.lightColor.z = lightColor[2];

		ImGui::TreePop();

	}


	if (isMove) {
		constBufferData.counter = 0;
	}
	else {
		++constBufferData.counter;
	}

	// 階層構造にする。
	if (ImGui::TreeNode("Debug")) {

		// メッシュを表示する。
		bool isMesh = constBufferData.isMeshScene;
		bool prevIsMesh = isMesh;
		ImGui::Checkbox("Mesh Scene", &isMesh);
		constBufferData.isMeshScene = isMesh;
		// 値が書き換えられていたら、サンプリングを初期化する。
		if (isMesh != prevIsMesh) {
			constBufferData.counter = 0;
		}

		// 法線を表示する。
		bool isNormal = constBufferData.isNormalScene;
		bool prevIsNormal = isNormal;
		ImGui::Checkbox("Normal Scene", &isNormal);
		constBufferData.isNormalScene = isNormal;
		// 値が書き換えられていたら、サンプリングを初期化する。
		if (isNormal != prevIsNormal) {
			constBufferData.counter = 0;
		}

		// ライトがあたった面だけ表示するフラグを更新。
		bool isLightHit = constBufferData.isLightHitScene;
		bool prevIsLightHit = isLightHit;
		ImGui::Checkbox("LightHit Scene", &isLightHit);
		constBufferData.isLightHitScene = isLightHit;
		// 値が書き換えられていたら、サンプリングを初期化する。
		if (isLightHit != prevIsLightHit) {
			constBufferData.counter = 0;
		}

		// デバッグ用でノイズ画面を出すためのフラグをセット。
		bool isNoise = constBufferData.isNoiseScene;
		ImGui::Checkbox("Noise Scene", &isNoise);
		constBufferData.isNoiseScene = isNoise;

		// AOを行うかのフラグをセット。
		bool isNoAO = constBufferData.isNoAO;
		ImGui::Checkbox("NoAO Scene", &isNoAO);
		constBufferData.isNoAO = isNoAO;

		// GIを行うかのフラグをセット。
		bool isNoGI = constBufferData.isNoGI;
		ImGui::Checkbox("NoGI Scene", &isNoGI);
		constBufferData.isNoGI = isNoGI;

		// GIのみを描画するかのフラグをセット。
		bool isGIOnlyScene = constBufferData.isGIOnlyScene;
		ImGui::Checkbox("GIOnly Scene", &isGIOnlyScene);
		constBufferData.isGIOnlyScene = isGIOnlyScene;

		// FPSを表示するかのフラグをセット。
		ImGui::Checkbox("Display FPS", &isDisplayFPS);


		ImGui::TreePop();

	}

	// 階層構造にする。
	if (ImGui::TreeNode("AS")) {

		// 太陽光線の強さを設定する。
		ImGui::SliderFloat("Sun Power", &constBufferData.AS.eSun, -10, 100);

		// レイリー散乱定数の値を設定する。
		ImGui::SliderFloat("Rayleigh Scattering Power", &constBufferData.AS.kr, -1, 1);

		// ミー散乱定数の値を設定する。
		ImGui::SliderFloat("Mie Scattering Power", &constBufferData.AS.km, -1, 1);

		// サンプリング数を設定する。
		ImGui::SliderFloat("Sample Count", &constBufferData.AS.samples, 0, 10);

		// 大気圏の一番上の高さ
		ImGui::SliderFloat("Outer Radius", &constBufferData.AS.outerRadius, 0, 20000);

		// 地上の高さ
		ImGui::SliderFloat("Inner Radius", &constBufferData.AS.innerRadius, 0, 20000);

		// 大気散乱を求める際に使用する定数
		ImGui::SliderFloat("Scattering G", &constBufferData.AS.g, -1.0f, 1.0f);

		// 平均大気密度を求めるための高さ
		ImGui::SliderFloat("Aveheight", &constBufferData.AS.aveHeight, 0.0f, 1.0f);

		ImGui::TreePop();

	}

}



/*

角度は内積を使うことで1~-1の範囲にしている。
→ライティングのときと同じ感じ
でも-になることでScale関数の値がいかれる。
→恐らく+しか想定していない？
→そもそも-になるのが正しいのかを考える。
　→地上を法線としたら-になる部分は見えない部分になるはず。

*/