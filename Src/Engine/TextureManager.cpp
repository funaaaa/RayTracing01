#include "TextureManager.h"
#include "Engine.h"
#include "DescriptorHeapMgr.h"
#include <array>
#include <DirectXTex/include/DDS.h>
#include <cassert>

TextureManager::TextureManager() {
}

void TextureManager::WriteTextureData(CD3DX12_RESOURCE_DESC& TexresDesc, DirectX::TexMetadata& MetaData, Microsoft::WRL::ComPtr<ID3D12Resource>& TexBuff,
	std::vector<D3D12_SUBRESOURCE_DATA> Subresource) {

	// Footprint(コピー可能なリソースのレイアウト)を取得。
	std::array<D3D12_PLACED_SUBRESOURCE_FOOTPRINT, 16> footprint;
	UINT64 totalBytes = 0;
	std::array<UINT64, 16> rowSizeInBytes = { 0 };
	std::array<UINT, 16> numRow = { 0 };
	Engine::Ins()->device_.dev_->GetCopyableFootprints(&TexresDesc, 0, static_cast<UINT>(MetaData.mipLevels), 0, footprint.data(), numRow.data(), rowSizeInBytes.data(), &totalBytes);

	// Upload用のバッファを作成する。
	D3D12_RESOURCE_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = totalBytes;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

	D3D12_HEAP_PROPERTIES heap = D3D12_HEAP_PROPERTIES();
	heap.Type = D3D12_HEAP_TYPE_UPLOAD;

	Microsoft::WRL::ComPtr<ID3D12Resource> iUploadBuffer = nullptr;
	Engine::Ins()->device_.dev_->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&iUploadBuffer));

	// UploadBufferへの書き込み。
	void* ptr = nullptr;
	iUploadBuffer->Map(0, nullptr, &ptr);

	// 1ピクセルのサイズ
	//UINT pixelSize = rowSizeInBytes / MetaData.width;

	for (uint32_t mip = 0; mip < MetaData.mipLevels; ++mip) {

		assert(Subresource[mip].RowPitch == static_cast<LONG_PTR>(rowSizeInBytes[mip]));
		assert(Subresource[mip].RowPitch <= footprint[mip].Footprint.RowPitch);

		uint8_t* uploadStart = reinterpret_cast<uint8_t*>(ptr) + footprint[mip].Offset;

		for (uint32_t height = 0; height < numRow[mip]; ++height) {

			memcpy(uploadStart + height * footprint[mip].Footprint.RowPitch, reinterpret_cast<const void*>(reinterpret_cast<uintptr_t>(Subresource[mip].pData) + height * Subresource[mip].RowPitch), Subresource[mip].RowPitch);

		}

	}

	// コピーする。
	for (uint32_t mip = 0; mip < MetaData.mipLevels; ++mip) {

		D3D12_TEXTURE_COPY_LOCATION copyDestLocation;
		copyDestLocation.pResource = TexBuff.Get();
		copyDestLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		copyDestLocation.SubresourceIndex = mip;

		D3D12_TEXTURE_COPY_LOCATION copySrcLocation;
		copySrcLocation.pResource = iUploadBuffer.Get();
		copySrcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		copySrcLocation.PlacedFootprint = footprint[mip];

		Engine::Ins()->mainGraphicsCmdList_->CopyTextureRegion(
			&copyDestLocation,
			0, 0, 0,
			&copySrcLocation,
			nullptr
		);

	}


	// リソースバリアをセット。
	D3D12_RESOURCE_BARRIER resourceBarrier = D3D12_RESOURCE_BARRIER();
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Transition.pResource = TexBuff.Get();
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	Engine::Ins()->mainGraphicsCmdList_->ResourceBarrier(1, &resourceBarrier);

	// コピーコマンドの実行。
	Engine::Ins()->mainGraphicsCmdList_->Close();
	ID3D12CommandList* commandLists[] = { Engine::Ins()->mainGraphicsCmdList_.Get() };
	Engine::Ins()->graphicsCmdQueue_->ExecuteCommandLists(1, commandLists);
	Engine::Ins()->graphicsCmdQueue_->Signal(Engine::Ins()->asFence_.Get(), ++Engine::Ins()->asfenceValue_);

	// グラフィックコマンドリストの完了待ち
	UINT64 gpuFence = Engine::Ins()->asFence_->GetCompletedValue();
	if (gpuFence != Engine::Ins()->asfenceValue_) {
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);
		Engine::Ins()->asFence_->SetEventOnCompletion(Engine::Ins()->asfenceValue_, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	// コマンドアロケータのリセット
	Engine::Ins()->mainGraphicsCmdAllocator_->Reset();	//キューをクリア

	// コマンドリストのリセット
	Engine::Ins()->mainGraphicsCmdList_->Reset(Engine::Ins()->mainGraphicsCmdAllocator_.Get(), nullptr);//再びコマンドリストを貯める準備



}


int TextureManager::LoadTexture(LPCWSTR FileName) {

	// ファイルがロード済みかをチェック
	if (0 < texture_.size()) {

		int counter = 0;
		for (auto& index_ : texture_) {

			// ロードしてあったら識別番号を返す
			if (index_.filePath_ == FileName) {

				return descriptorHeadMgrIndex_[counter];

			}
			++counter;
		}
	}

	// テクスチャデータを保存
	Texture proTexture{};
	proTexture.filePath_ = FileName;

	DirectX::TexMetadata metadata;
	DirectX::ScratchImage scratchImg;
	HRESULT result = LoadFromDDSFile(FileName, DirectX::DDS_FLAGS::DDS_FLAGS_NONE, &metadata, scratchImg);
	if (FAILED(result)) {
		result = LoadFromWICFile(FileName, DirectX::WIC_FLAGS::WIC_FLAGS_NONE/*WIC_FLAGS_FORCE_RGB*/, &metadata, scratchImg);
	}
	if (FAILED(result)) {
		assert(0);
	}

	const DirectX::Image* img = scratchImg.GetImages();

	// MipMapを取得。
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	result = PrepareUpload(
		Engine::Ins()->device_.dev_.Get(), img, scratchImg.GetImageCount(), metadata, subresources);

	// リソース設定
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		static_cast<UINT>(metadata.width),
		static_cast<UINT>(metadata.height),
		static_cast<UINT16>(metadata.arraySize),
		static_cast<UINT16>(metadata.mipLevels));
	texresDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

	// テクスチャバッファの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> texbuff = nullptr;
	CD3DX12_HEAP_PROPERTIES texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	result = Engine::Ins()->device_.dev_->CreateCommittedResource(&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&texbuff));


	// ここから追記。

	WriteTextureData(texresDesc, metadata, texbuff, subresources);

	// ここまで追記。

	// テクスチャ配列の最後尾にロードしたテクスチャ情報を記録
	proTexture.metadata_ = metadata;
	proTexture.scratchImg_ = &scratchImg;
	proTexture.texBuff_ = texbuff;
	texture_.emplace_back(proTexture);
	descriptorHeadMgrIndex_.emplace_back(DescriptorHeapMgr::Ins()->GetHead());

	// ディスクリプタヒープのアドレスを取得
	CD3DX12_CPU_DESCRIPTOR_HANDLE basicHeapHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		DescriptorHeapMgr::Ins()->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(), DescriptorHeapMgr::Ins()->GetHead(), Engine::Ins()->device_.dev_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	// シェーダーリソースビューの生成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);
	// ヒープにシェーダーリソースビュー生成
	Engine::Ins()->device_.dev_->CreateShaderResourceView(
		texbuff.Get(),
		&srvDesc,
		basicHeapHandle
	);

	// ディスクリプタヒープをインクリメント
	DescriptorHeapMgr::Ins()->IncrementHead();

	return DescriptorHeapMgr::Ins()->GetHead() - 1;
}

int TextureManager::LoadTexture(std::array<wchar_t, 128> FileName)
{
	// ファイルがロード済みかをチェック
	if (0 < texture_.size()) {

		int counter = 0;
		for (auto& index_ : texture_) {

			// ロードしてあったら識別番号を返す
			if (index_.filePath_ == FileName.data()) {

				return descriptorHeadMgrIndex_[counter];

			}
			++counter;
		}
	}

	// テクスチャデータを保存
	Texture proTexture{};
	proTexture.filePathP_ = FileName;
	proTexture.filePath_ = proTexture.filePathP_.data();

	// ロードしていなかったらロードする
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage scratchImg;
	HRESULT result = LoadFromDDSFile(proTexture.filePath_, DirectX::DDS_FLAGS::DDS_FLAGS_NONE, &metadata, scratchImg);
	if (FAILED(result)) {
		result = LoadFromWICFile(proTexture.filePath_, DirectX::WIC_FLAGS::WIC_FLAGS_NONE/*WIC_FLAGS_FORCE_RGB*/, &metadata, scratchImg);
	}
	if (FAILED(result)) {
		assert(0);
	}
	const DirectX::Image* img = scratchImg.GetImage(0, 0, 0);

	// DDSだったらMipMapを取得。
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	result = PrepareUpload(
		Engine::Ins()->device_.dev_.Get(), img, scratchImg.GetImageCount(), metadata, subresources);

	// リソース設定
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		static_cast<UINT>(metadata.width),
		static_cast<UINT>(metadata.height),
		static_cast<UINT16>(metadata.arraySize),
		static_cast<UINT16>(metadata.mipLevels));

	// テクスチャバッファの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> texbuff = nullptr;
	CD3DX12_HEAP_PROPERTIES texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	result = Engine::Ins()->device_.dev_->CreateCommittedResource(&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&texbuff));


	// ここから

	WriteTextureData(texresDesc, metadata, texbuff, subresources);

	// ここまで追記。

	// テクスチャ配列の最後尾にロードしたテクスチャ情報を記録
	proTexture.metadata_ = metadata;
	proTexture.scratchImg_ = &scratchImg;
	proTexture.texBuff_ = texbuff;
	texture_.emplace_back(proTexture);
	descriptorHeadMgrIndex_.emplace_back(DescriptorHeapMgr::Ins()->GetHead());

	// ディスクリプタヒープのアドレスを取得
	CD3DX12_CPU_DESCRIPTOR_HANDLE basicHeapHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		DescriptorHeapMgr::Ins()->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(), DescriptorHeapMgr::Ins()->GetHead(), Engine::Ins()->device_.dev_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	// シェーダーリソースビューの生成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);
	// ヒープにシェーダーリソースビュー生成
	Engine::Ins()->device_.dev_->CreateShaderResourceView(
		texbuff.Get(),
		&srvDesc,
		basicHeapHandle
	);

	// ディスクリプタヒープをインクリメント
	DescriptorHeapMgr::Ins()->IncrementHead();

	return DescriptorHeapMgr::Ins()->GetHead() - 1;
}

int TextureManager::LoadTexture(std::wstring FileName, const void* Src, const UINT64& Size)
{

	// データがnullかどうかをチェック。
	if (Src == nullptr) {

		assert(0);

	}

	// ファイルがロード済みかをチェック
	if (0 < texture_.size()) {

		int counter = 0;
		for (auto& index_ : texture_) {

			// ロードしてあったら識別番号を返す
			if (index_.filePath_ == FileName.data()) {

				return descriptorHeadMgrIndex_[counter];

			}
			++counter;
		}
	}

	// テクスチャデータを保存
	Texture proTexture{};
	proTexture.filePath_ = FileName.data();

	// ロードしていなかったらロードする
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage scratchImg;

	HRESULT hr = E_FAIL;
	hr = LoadFromDDSMemory(Src, Size, DirectX::DDS_FLAGS::DDS_FLAGS_NONE, &metadata, scratchImg);
	if (FAILED(hr)) {
		hr = LoadFromWICMemory(Src, Size, DirectX::WIC_FLAGS::WIC_FLAGS_NONE/*WIC_FLAGS_FORCE_RGB*/, &metadata, scratchImg);
	}
	if (FAILED(hr)) {
		assert(0);
	}

	// リソース設定
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		static_cast<UINT>(metadata.width),
		static_cast<UINT>(metadata.height),
		static_cast<UINT16>(metadata.arraySize),
		static_cast<UINT16>(metadata.mipLevels));

	// テクスチャバッファの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> texbuff = nullptr;
	CD3DX12_HEAP_PROPERTIES texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	HRESULT result = Engine::Ins()->device_.dev_->CreateCommittedResource(&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texbuff));

	// データ転送
	const DirectX::Image* img = scratchImg.GetImage(0, 0, 0);
	result = texbuff->WriteToSubresource(
		0,
		nullptr,							// 全領域コピー
		img->pixels,						// 元データの先頭アドレス
		static_cast<UINT>(img->rowPitch),	// 一ラインのサイズ
		static_cast<UINT>(img->slicePitch)	// いちまいのサイズ
	);

	// テクスチャ配列の最後尾にロードしたテクスチャ情報を記録
	proTexture.metadata_ = metadata;
	proTexture.scratchImg_ = &scratchImg;
	proTexture.texBuff_ = texbuff;
	texture_.emplace_back(proTexture);
	descriptorHeadMgrIndex_.emplace_back(DescriptorHeapMgr::Ins()->GetHead());

	// ディスクリプタヒープのアドレスを取得
	CD3DX12_CPU_DESCRIPTOR_HANDLE basicHeapHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		DescriptorHeapMgr::Ins()->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(), DescriptorHeapMgr::Ins()->GetHead(), Engine::Ins()->device_.dev_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	// シェーダーリソースビューの生成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	// ヒープにシェーダーリソースビュー生成
	Engine::Ins()->device_.dev_->CreateShaderResourceView(
		texbuff.Get(),
		&srvDesc,
		basicHeapHandle
	);

	// ディスクリプタヒープをインクリメント
	DescriptorHeapMgr::Ins()->IncrementHead();

	return DescriptorHeapMgr::Ins()->GetHead() - 1;
}


int TextureManager::CreateTexture(DirectX::XMFLOAT4 Color)
{
	// 同じ色のテクスチャがすでに生成済みかをチェックする
	LPCWSTR selfTex = L"SelfTexture";

	int counter = 0;
	for (auto& index_ : texture_) {

		if (index_.filePath_ == selfTex && index_.colorData_.x == Color.x && index_.colorData_.y == Color.y &&
			index_.colorData_.z == Color.z && index_.colorData_.w == Color.w) {

			// すでに生成してあるテクスチャなのでSRVヒープの番号を返す
			return counter;

		}
		++counter;

	}

	// 画像データを作成する
	static const int texWidth = 256;
	const int imageDataCount = texWidth * texWidth;
	// 画像イメージデータ配列
	std::array<DirectX::XMFLOAT4, texWidth> imageData;

	// 全ピクセルの色を初期化
	for (auto& index_ : imageData) {
		index_.x = Color.x;
		index_.y = Color.y;
		index_.z = Color.z;
		index_.w = Color.w;
	}

	// リソース設定
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		texWidth,
		static_cast<UINT>(texWidth),
		static_cast<UINT16>(1),
		static_cast<UINT16>(1));

	// テクスチャバッファの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> texbuff = nullptr;
	CD3DX12_HEAP_PROPERTIES texHeapPropBuff = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	HRESULT result = Engine::Ins()->device_.dev_->CreateCommittedResource(&texHeapPropBuff,
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texbuff));

	// データ転送
	result = texbuff->WriteToSubresource(
		0,
		nullptr,									//全領域コピー
		imageData.data(),							//元データの先頭アドレス
		sizeof(DirectX::XMFLOAT4) * texWidth,		//一ラインのサイズ
		sizeof(DirectX::XMFLOAT4) * imageDataCount	//いちまいのサイズ
	);

	// テクスチャ配列の最後尾にロードしたテクスチャ情報を記録
	Texture proTexture{};
	proTexture.filePath_ = L"SelfTexture";
	proTexture.metadata_ = {};
	proTexture.scratchImg_ = {};
	proTexture.texBuff_ = texbuff;
	proTexture.colorData_ = Color;
	texture_.push_back(proTexture);
	descriptorHeadMgrIndex_.emplace_back(DescriptorHeapMgr::Ins()->GetHead());

	// ディスクリプタヒープのアドレスを取得
	CD3DX12_CPU_DESCRIPTOR_HANDLE basicHeapHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		DescriptorHeapMgr::Ins()->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(), DescriptorHeapMgr::Ins()->GetHead(), Engine::Ins()->device_.dev_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	// シェーダーリソースビューの生成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	// ヒープにシェーダーリソースビュー生成
	Engine::Ins()->device_.dev_->CreateShaderResourceView(
		texbuff.Get(),
		&srvDesc,
		basicHeapHandle
	);

	// ディスクリプタヒープをインクリメント
	DescriptorHeapMgr::Ins()->IncrementHead();

	return DescriptorHeapMgr::Ins()->GetHead() - 1;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSRV(int IDNum) {

	D3D12_GPU_DESCRIPTOR_HANDLE basicHeapHandle = DescriptorHeapMgr::Ins()->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();

	// 消費した分だけアドレスをずらす。
	for (int i = 0; i < IDNum; ++i) {

		basicHeapHandle.ptr += Engine::Ins()->device_.dev_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	}

	return basicHeapHandle;
}

Texture TextureManager::GetTexture(int ID)
{
	return texture_[ID];
}
