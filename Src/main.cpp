#include "SoundManager.h"
#include "PiplineManager.h"
#include "RenderTarget.h"
#include "Camera.h"
#include "Input.h"
#include "TextureManager.h"
#include "FbxLoader.h"

#include "BLASRegister.h"
#include "PorygonInstance.h"
#include "TLAS.h"
#include "RayRootsignature.h"
#include "DynamicConstBuffer.h"
#include "DescriptorHeapMgr.h"
#include "Vec.h"
#include "PorygonInstanceRegister.h"
#include "HitGroupMgr.h"
#include "RaytracingPipline.h"

#include "HitGroup.h"

#include <utilapiset.h>

#define COLORHEX(hex) hex / 255.0f

#define SCREEN_VIRTUAL_WIDTH 300

static const wchar_t* hitGroupName = L"hitGroupName";

// fps更新
void FPS();

struct KariConstBufferData {

	XMMATRIX mtxView;			// ビュー行列。
	XMMATRIX mtxProj;			// プロジェクション行列。
	XMMATRIX mtxViewInv;		// ビュー逆行列。
	XMMATRIX mtxProjInv;		// プロジェクション逆行列。
	XMFLOAT4 lightDirection;	// 平行光源の向き。
	XMFLOAT4 lightColor;		// 平行光源色。
	XMFLOAT4 ambientColor;		// 環境光。

};

#include "BLAS.h"
// スラリンラボから持ってきた関数
namespace surarin {

	void WriteToHostVisibleMemory(ComPtr<ID3D12Resource>& resource, const void* pData, size_t dataSize) {
		if (resource == nullptr) {
			return;
		}
		void* mapped = nullptr;
		HRESULT hr = resource->Map(0, nullptr, (void**)&mapped);
		if (SUCCEEDED(hr)) {
			memcpy(mapped, pData, dataSize);
			resource->Unmap(0, nullptr);
		}
	}

	ComPtr<ID3D12Resource> CreateBuffer(size_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initialState, D3D12_HEAP_TYPE heapType, const wchar_t* name = nullptr) {
		D3D12_HEAP_PROPERTIES heapProps{};
		if (heapType == D3D12_HEAP_TYPE_DEFAULT) {
			heapProps = D3D12_HEAP_PROPERTIES{
			D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1
			};
		}
		if (heapType == D3D12_HEAP_TYPE_UPLOAD) {
			heapProps = D3D12_HEAP_PROPERTIES{
			D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1
			};
		}
		HRESULT hr;
		ComPtr<ID3D12Resource> resource;
		D3D12_RESOURCE_DESC resDesc{};
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resDesc.Alignment = 0;
		resDesc.Width = size;
		resDesc.Height = 1;
		resDesc.DepthOrArraySize = 1;
		resDesc.MipLevels = 1;
		resDesc.Format = DXGI_FORMAT_UNKNOWN;
		resDesc.SampleDesc = { 1, 0 };
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resDesc.Flags = flags;

		hr = DirectXBase::Instance()->dev->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			initialState,
			nullptr,
			IID_PPV_ARGS(resource.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			OutputDebugStringA("CreateBuffer failed.\n");
		}
		if (resource != nullptr && name != nullptr) {
			resource->SetName(name);
		}
		return resource;
	}

	ComPtr<ID3D12Resource> CreateDataBuffer(
		size_t requestSize,
		const void* initData,
		D3D12_HEAP_TYPE heapType,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE
	) {

		auto initialState = D3D12_RESOURCE_STATE_COPY_DEST;
		if (heapType == D3D12_HEAP_TYPE_UPLOAD) {
			initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
		}

		auto resource = CreateBuffer(requestSize, flags, initialState, heapType);

		if (initData != nullptr) {
			if (heapType == D3D12_HEAP_TYPE_DEFAULT) {
				WriteToHostVisibleMemory(resource, initData, requestSize);
			}
			if (heapType == D3D12_HEAP_TYPE_UPLOAD) {
				WriteToHostVisibleMemory(resource, initData, requestSize);
			}
		}
		return resource;

	}

	void ExecuteCommandList(ComPtr<ID3D12GraphicsCommandList4> command)
	{
		ID3D12CommandList* commandLists[] = {
			command.Get(),
		};
		DirectXBase::Instance()->cmdQueue->ExecuteCommandLists(1, commandLists);
	}

	void WaitForIdleGpu() {
		if (DirectXBase::Instance()->cmdQueue) {
			auto commandList = DirectXBase::Instance()->cmdList;
			auto waitFence = DirectXBase::Instance()->fence;
			UINT64 fenceValue = 1;
			auto event = CreateEvent(nullptr, false, false, nullptr);
			waitFence->SetEventOnCompletion(fenceValue, event);
			DirectXBase::Instance()->cmdQueue->Signal(waitFence.Get(), fenceValue);
			WaitForSingleObject(event, INFINITE);
		}
	}

	ComPtr<ID3D12Resource> CreateTexture2D(UINT width, UINT height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initialState, D3D12_HEAP_TYPE heapType) {
		D3D12_HEAP_PROPERTIES heapProps{};
		if (heapType == D3D12_HEAP_TYPE_DEFAULT) {
			heapProps = D3D12_HEAP_PROPERTIES{
			D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1
			};
		}
		if (heapType == D3D12_HEAP_TYPE_UPLOAD) {
			heapProps = D3D12_HEAP_PROPERTIES{
			D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1
			};
		}
		HRESULT hr;
		ComPtr<ID3D12Resource> resource;
		D3D12_RESOURCE_DESC resDesc{};
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resDesc.Alignment = 0;
		resDesc.Width = width;
		resDesc.Height = height;
		resDesc.DepthOrArraySize = 1;
		resDesc.MipLevels = 1;
		resDesc.Format = format;
		resDesc.SampleDesc = { 1, 0 };
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.Flags = flags;

		hr = DirectXBase::Instance()->dev->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			initialState,
			nullptr,
			IID_PPV_ARGS(resource.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			OutputDebugStringA("CreateTexture2D failed.\n");
		}
		return resource;
	}

	UINT RoundUp(size_t size, UINT align) {
		return UINT(size + align - 1) & ~(align - 1);
	}

	UINT WriteShaderIdentifier(void* dst, const void* shaderId)
	{
		memcpy(dst, shaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		return D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	}

	UINT WriteGPUDescriptor(
		void* dst, const D3D12_GPU_DESCRIPTOR_HANDLE* descriptor)
	{
		memcpy(dst, descriptor, sizeof(descriptor));
		return UINT(sizeof(descriptor));
	}

}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	/*----------DirectX初期化処理----------*/
	DirectXBase directXBase;							// DirectX基盤部分
	directXBase.Init();									// DirectX基盤の初期化
	SoundManager::Instance()->SettingSoundManager();	// サウンドマネージャーをセットする

	/*----------パイプライン生成----------*/
	PiplineManager::Instance()->Init();

	/*----------変数宣言----------*/
	srand(time(NULL));

	// ディスクリプタヒープを初期化。
	DescriptorHeapMgr::Instance()->GenerateDescriptorHeap();

	// FBXLoaderを初期化。
	FbxLoader::Instance()->Init();

	// ヒットグループを設定。
	HitGroupMgr::Instance()->Setting();

	// 使用するシェーダーを列挙。
	vector<RayPiplineShaderData> useShaders;

	useShaders.push_back({ "Resource/ShaderFiles/RayTracing/triangleShaderHeader.hlsl", {L"mainRayGen", L"mainMS", L"shadowMS", L"mainCHS", L"mainAnyHit"} });

	// レイトレパイプラインを設定。
	RaytracingPipline pipline;
	pipline.Setting(useShaders);

	// 猿のBLASを生成。
	int boneBlas = BLASRegister::Instance()->GenerateFbx("Resource/", "boneTest.fbx", HitGroupMgr::Instance()->hitGroupNames[HitGroupMgr::DEF_HIT_GROUP], L"Resource/backGround.png");

	// 猿のBLASを生成。
	int monkeyBlas = BLASRegister::Instance()->GenerateObj("Resource/", "monkey.obj", HitGroupMgr::Instance()->hitGroupNames[HitGroupMgr::DEF_HIT_GROUP], L"Resource/backGround.png");

	// 床のBLASを生成。
	int groundBlas = BLASRegister::Instance()->GenerateObj("Resource/", "ground.obj", HitGroupMgr::Instance()->hitGroupNames[HitGroupMgr::DEF_HIT_GROUP], L"Resource/Fine_Basin.jpg");

	// インスタンスを生成
	int boneA = PorygonInstanceRegister::Instance()->CreateInstance(BLASRegister::Instance()->GetBLASBuffer(boneBlas), 0, 2);
	int boneB = PorygonInstanceRegister::Instance()->CreateInstance(BLASRegister::Instance()->GetBLASBuffer(boneBlas), 0, 1);
	int monkey = PorygonInstanceRegister::Instance()->CreateInstance(BLASRegister::Instance()->GetBLASBuffer(monkeyBlas), 1, 2);
	int ground = PorygonInstanceRegister::Instance()->CreateInstance(BLASRegister::Instance()->GetBLASBuffer(groundBlas), 2, 2);

	// 移動させる。
	PorygonInstanceRegister::Instance()->AddTrans(boneA, -2.0f, 0.0f, 0);
	PorygonInstanceRegister::Instance()->AddTrans(boneB, 2.0f, 0.0f, 0);
	PorygonInstanceRegister::Instance()->AddTrans(monkey, 0.0f, 0.0f, 0);
	PorygonInstanceRegister::Instance()->AddTrans(ground, 0.0f, -1.0f, 0);

	// ある程度回転させる。
	PorygonInstanceRegister::Instance()->AddRotate(boneA, 0.0f, -0.1f, 0);
	PorygonInstanceRegister::Instance()->AddRotate(boneB, 0.0f, -0.1f, 0);

	// TLASを生成。
	TLAS tlas;
	tlas.GenerateTLAS(L"TlasDescriptorHeap");

	/*==========  UAV出力バッファの準備  ==========*/

	// UAVを設定
	ComPtr<ID3D12Resource> rayTracingOutput = surarin::CreateTexture2D(
		window_width, window_height, DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		D3D12_HEAP_TYPE_DEFAULT
	);

	// 先頭ハンドルを取得
	CD3DX12_CPU_DESCRIPTOR_HANDLE basicHeapHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		DescriptorHeapMgr::Instance()->GetDescriptorHeap().Get()->GetCPUDescriptorHandleForHeapStart(), DescriptorHeapMgr::Instance()->GetHead(), DirectXBase::Instance()->dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	// ディスクリプタヒープにUAVを確保
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	DirectXBase::Instance()->dev->CreateUnorderedAccessView(
		rayTracingOutput.Get(), nullptr, &uavDesc, basicHeapHandle);

	// UAVのディスクリプタヒープのインデックスを取得
	int uavDescriptorIndex = DescriptorHeapMgr::Instance()->GetHead();

	// ディスクリプタヒープをインクリメント
	DescriptorHeapMgr::Instance()->IncrementHead();

	rayTracingOutput->SetName(L"RayTracingOutputUAV");





	/*==========  ShaderTableの生成  ==========*/			// シェーダーテーブルでは使用するローカルルートシグネチャを元にサイズを決定する。

	const auto ShaderRecordAlignment = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;

	// RayGenerationシェーダーではローカルルートシグネチャ未使用。
	UINT raygenRecordSize = 0;
	raygenRecordSize += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	raygenRecordSize = surarin::RoundUp(raygenRecordSize, ShaderRecordAlignment);

	// Missシェーダーではローカルルートシグネチャ未使用。
	UINT missRecordSize = 0;
	missRecordSize += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	missRecordSize = surarin::RoundUp(missRecordSize, ShaderRecordAlignment);

	// ヒットグループでは、ShaderIndentiferとローカルルートシグネチャによるVB/IB(SRV)を使用。
	UINT hitgroupRecordSize = 0;
	hitgroupRecordSize += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	hitgroupRecordSize += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE);	// 頂点バッファビュー
	hitgroupRecordSize += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE);	// インデックスバッファビュー
	hitgroupRecordSize += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE);	// テクスチャ
	hitgroupRecordSize = surarin::RoundUp(hitgroupRecordSize, ShaderRecordAlignment);

	// 使用する各シェーダーの個数より、シェーダーテーブルのサイズを求める。
	UINT hitgroupCount = 1;
	UINT raygenSize = 1 * raygenRecordSize;
	UINT missSize = 2 * missRecordSize;
	UINT hitGroupSize = hitgroupCount * hitgroupRecordSize;

	// 各テーブルの開始位置にアライメント制約があるので調整する。
	UINT tableAlign = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	UINT raygenRegion = surarin::RoundUp(raygenRecordSize, tableAlign);
	UINT missRegion = surarin::RoundUp(missSize, tableAlign);
	UINT hitgroupRegion = surarin::RoundUp(hitGroupSize, tableAlign);

	// シェーダーテーブルのサイズ.
	UINT tableSize = raygenRegion + missRegion + hitgroupRegion;



	/*========== シェーダーテーブルの構築 ==========*/

	// シェーダーテーブル確保。
	ComPtr<ID3D12Resource> shaderTable = surarin::CreateBuffer(
		tableSize, D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		D3D12_HEAP_TYPE_UPLOAD,
		L"ShaderTable");

	ComPtr<ID3D12StateObjectProperties> rtsoProps;
	pipline.GetStateObject().As(&rtsoProps);

	// 各シェーダーレコードを書き込んでいく。
	void* mapped = nullptr;
	shaderTable->Map(0, nullptr, &mapped);
	uint8_t* pStart = static_cast<uint8_t*>(mapped);

	// RayGeneration 用のシェーダーレコードを書き込み。
	uint8_t* rgsStart = pStart;
	{
		uint8_t* p = rgsStart;
		void* id = rtsoProps->GetShaderIdentifier(L"mainRayGen");
		p += surarin::WriteShaderIdentifier(p, id);

	}

	// Miss Shader 用のシェーダーレコードを書き込み。
	uint8_t* missStart = pStart + raygenRegion;
	{
		uint8_t* p = missStart;
		auto id = rtsoProps->GetShaderIdentifier(L"mainMS");
		p += surarin::WriteShaderIdentifier(p, id);
		// Shadow Miss Shader 用のシェーダーレコードを書き込み。
		id = rtsoProps->GetShaderIdentifier(L"shadowMS");
		p += surarin::WriteShaderIdentifier(p, id);
	}

	// Hit Group 用のシェーダーレコードを書き込み。
	uint8_t* hitgroupStart = pStart + raygenRegion + missRegion;
	{

		uint8_t* pRecord = hitgroupStart;
		// monekyに対応するシェーダーレコードを書き込む
		pRecord = BLASRegister::Instance()->WriteShaderRecord(pRecord, boneBlas, hitgroupRecordSize, pipline.GetStateObject());
		// skydome に対応するシェーダーレコードを書き込む
		pRecord = BLASRegister::Instance()->WriteShaderRecord(pRecord, monkeyBlas, hitgroupRecordSize, pipline.GetStateObject());
		// ground に対応するシェーダーレコードを書き込む
		pRecord = BLASRegister::Instance()->WriteShaderRecord(pRecord, groundBlas, hitgroupRecordSize, pipline.GetStateObject());

	}
	shaderTable->Unmap(0, nullptr);


	/*==========  D3D12_DISPATCH_RAYS_DESCの設定  ==========*/

	D3D12_DISPATCH_RAYS_DESC dispatchRayDesc = {};

	// DispatchRays のために情報をセットしておく.
	auto startAddress = shaderTable->GetGPUVirtualAddress();
	// RayGenerationシェーダーの情報
	auto& shaderRecordRG = dispatchRayDesc.RayGenerationShaderRecord;
	shaderRecordRG.StartAddress = startAddress;
	shaderRecordRG.SizeInBytes = raygenSize;
	startAddress += raygenRegion;
	// Missシェーダーの情報
	auto& shaderRecordMS = dispatchRayDesc.MissShaderTable;
	shaderRecordMS.StartAddress = startAddress;
	shaderRecordMS.SizeInBytes = missSize;
	shaderRecordMS.StrideInBytes = missRecordSize;
	startAddress += missRegion;
	// HitGroupの情報
	auto& shaderRecordHG = dispatchRayDesc.HitGroupTable;
	shaderRecordHG.StartAddress = startAddress;
	shaderRecordHG.SizeInBytes = hitGroupSize;
	shaderRecordHG.StrideInBytes = hitgroupRecordSize;
	startAddress += hitgroupRegion;
	// レイの情報
	dispatchRayDesc.Width = window_width;
	dispatchRayDesc.Height = window_height;
	dispatchRayDesc.Depth = 1;



	// 仮の定数バッファを宣言
	KariConstBufferData constBufferData;
	constBufferData.ambientColor = { 1,1,1,1 };
	constBufferData.lightColor = { 1,1,1,1 };
	constBufferData.lightDirection = { 1,1,0,0 };
	constBufferData.mtxProj = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(60.0f),				//画角(60度)
		(float)window_width / window_height,	//アスペクト比
		0.1f, 1000000.0f							//前端、奥端
	);
	constBufferData.mtxProjInv = XMMatrixInverse(nullptr, constBufferData.mtxProj);
	Vec3 eye = { 0,0,-10 };
	Vec3 target = { 0,0,0 };
	Vec3 up = { 0,1,0 };
	constBufferData.mtxView = XMMatrixLookAtLH(eye.ConvertXMVECTOR(), target.ConvertXMVECTOR(), up.ConvertXMVECTOR());
	constBufferData.mtxViewInv = XMMatrixInverse(nullptr, constBufferData.mtxView);

	DynamicConstBuffer constBuff;
	constBuff.Generate(sizeof(KariConstBufferData), L"constBuffer");

	// カメラを初期化。
	Camera::Instance()->Init();


	/*----------ゲームループ----------*/
	while (true) {
		/*----------毎フレーム処理(描画前処理)----------*/
		directXBase.processBeforeDrawing();



		/*----- 更新処理 -----*/

		// ビュー行列を生成。
		Camera::Instance()->GenerateMatView();

		FPS();

		//Camera::target = triangle.GetPos();

		// スキニングアニメーションさせる。
		BLASRegister::Instance()->ComputeSkin(boneBlas);

		float speed = 0.1f;
		float rot = 0.05f;
		if (Input::isKey(DIK_W)) Camera::Instance()->Move(speed);
		if (Input::isKey(DIK_S)) Camera::Instance()->Move(-speed);
		if (Input::isKey(DIK_A)) Camera::Instance()->MoveRight(speed);
		if (Input::isKey(DIK_D)) Camera::Instance()->MoveRight(-speed);
		if (Input::isKey(DIK_UP)) Camera::Instance()->eye.y += speed;
		if (Input::isKey(DIK_DOWN)) Camera::Instance()->eye.y -= speed;

		//if (Input::isKey(DIK_I)) porygonInstance[0].AddTrans(0.0f, 0.0f, -0.1f);
		//if (Input::isKey(DIK_K)) porygonInstance[0].AddTrans(0.0f, 0.0f, 0.1f);
		//if (Input::isKey(DIK_J)) porygonInstance[0].AddTrans(0.1f, 0.0f, 0.0f);
		//if (Input::isKey(DIK_L)) porygonInstance[0].AddTrans(-0.1f, 0.0f, 0.0f);

		if (Input::isKey(DIK_1)) {

			BLASRegister::Instance()->InitAnimation(boneBlas);

		}
		if (Input::isKey(DIK_2)) {

			BLASRegister::Instance()->PlayAnimation(boneBlas);

		}
		if (Input::isKey(DIK_3)) {

			BLASRegister::Instance()->StopAnimation(boneBlas);

		}

		BLASRegister::Instance()->Update(boneBlas);

		// TLASを更新。
		tlas.Update();

		eye = Camera::Instance()->eye;
		target = Camera::Instance()->target;
		up = Camera::Instance()->up;

		// カメラを更新。
		Camera::Instance()->Update();


		/*----- 描画処理 -----*/

		// 画面に表示されるレンダーターゲットに戻す。
		//DirectXBase::Instance()->SetRenderTarget();

		auto frameIndex = DirectXBase::Instance()->swapchain->GetCurrentBackBufferIndex();
		constBufferData.mtxView = XMMatrixLookAtLH(eye.ConvertXMVECTOR(), target.ConvertXMVECTOR(), up.ConvertXMVECTOR());
		constBufferData.mtxViewInv = XMMatrixInverse(nullptr, constBufferData.mtxView);
		// 定数バッファの中身を更新する。
		constBuff.Write(frameIndex, &constBufferData, sizeof(KariConstBufferData));
		auto sceneConstantBuffer = constBuff.GetBuffer(frameIndex);

		// グローバルルートシグネチャで使うと宣言しているリソースらをセット。
		ID3D12DescriptorHeap* descriptorHeaps[] = { DescriptorHeapMgr::Instance()->GetDescriptorHeap().Get() };
		DirectXBase::Instance()->cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		DirectXBase::Instance()->cmdList->SetComputeRootSignature(globalRootSig.GetRootSig().Get());
		DirectXBase::Instance()->cmdList->SetComputeRootDescriptorTable(0, DescriptorHeapMgr::Instance()->GetGPUHandleIncrement(tlas.GetDescriptorHeapIndex()));
		DirectXBase::Instance()->cmdList->SetComputeRootDescriptorTable(2, DescriptorHeapMgr::Instance()->GetGPUHandleIncrement(uavDescriptorIndex));
		DirectXBase::Instance()->cmdList->SetComputeRootConstantBufferView(1, sceneConstantBuffer->GetGPUVirtualAddress());


		// レイトレーシング結果バッファをUAV状態へ
		auto barrierToUAV = CD3DX12_RESOURCE_BARRIER::Transition(
			rayTracingOutput.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		);
		DirectXBase::Instance()->cmdList->ResourceBarrier(1, &barrierToUAV);

		DirectXBase::Instance()->cmdList->SetPipelineState1(pipline.GetStateObject().Get());

		DirectXBase::Instance()->cmdList->DispatchRays(&dispatchRayDesc);

		// バックバッファのインデックスを取得する。
		UINT backBufferIndex = DirectXBase::Instance()->swapchain->GetCurrentBackBufferIndex();

		// バリアを設定し各リソースの状態を遷移させる.
		D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
		rayTracingOutput.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COPY_SOURCE),
		CD3DX12_RESOURCE_BARRIER::Transition(
		DirectXBase::Instance()->backBuffers[backBufferIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_COPY_DEST),
		};
		DirectXBase::Instance()->cmdList->ResourceBarrier(_countof(barriers), barriers);
		DirectXBase::Instance()->cmdList->CopyResource(DirectXBase::Instance()->backBuffers[backBufferIndex].Get(), rayTracingOutput.Get());

		// レンダーターゲットのリソースバリアをもとに戻す。
		D3D12_RESOURCE_BARRIER endBarriers[] = {

		CD3DX12_RESOURCE_BARRIER::Transition(
		DirectXBase::Instance()->backBuffers[backBufferIndex].Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_RENDER_TARGET)

		};
		DirectXBase::Instance()->cmdList->ResourceBarrier(_countof(endBarriers), endBarriers);

		directXBase.processAfterDrawing();

	}

	return 0;
}


// タイトルバーのFPSの更新
void FPS()
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
		SetWindowText(DirectXBase::Instance()->windowsAPI.hwnd, fps);
		//OutputDebugString(fps);

		prev_time = now_time;
		frame_count = 0;
	}
}