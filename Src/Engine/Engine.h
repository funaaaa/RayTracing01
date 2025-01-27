#pragma once

#include <DirectXTex/include/d3dx12.h>
#include <dxgi1_6.h>
#include <xstring>
#include <vector>
#include <array>
#include <wrl.h>
#include <dxgi.h>
#include <Xinput.h>
#include <d3d12.h>
#include <memory>
#include "Singleton.h"
#include "Input.h"

class WindowsAPI;

// DirectX12の基盤クラス
class Engine : public Singleton<Engine> {

public:

	// デバッグに使用する各クラス
	struct Debug {
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController_;
		Microsoft::WRL::ComPtr<ID3D12Debug1> shaderDebugController_;
	};

	// DirectXのデバイス関連
	struct Device {
		Microsoft::WRL::ComPtr<ID3D12Device5> dev_;
		Microsoft::WRL::ComPtr<IDXGIFactory6> dxgiFactory_;
		std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter1>> adapters_;
		Microsoft::WRL::ComPtr<IDXGIAdapter1> tmpAdapter_;
		std::vector<D3D_FEATURE_LEVEL> levels_;
		D3D_FEATURE_LEVEL featureLevel_;
	};

	// SwapChain関連
	struct SwapChain {
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeaps_;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> backBuffers_;
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc_;
		std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> depthBuffer_;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;
		Microsoft::WRL::ComPtr<IDXGISwapChain4> swapchain_;
	};

	// Input関連
	struct DevInput {
		IDirectInput8* dinput_;				// DirectInputオブジェクト
		IDirectInputDevice8* devkeybord_;	// キーボードオブジェクト
		IDirectInputDevice8* devmouse_;		// マウスオブジェクト
	};


public:

	/*===== メンバ変数 =====*/

	std::shared_ptr<WindowsAPI> windowsAPI_;	// WindowsAPIクラス
	Debug debug_;								// デバッグ用のクラス
	Device device_;								// デバイス関連
	SwapChain swapchain_;						// SwapChain関連
	DevInput input_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heapForImgui_;	//imgui用ディスクリプタヒープ


	/*-- GPUの非同期処理にあたって新たに作った変数たち --*/

	// コマンドアロケーター
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mainGraphicsCmdAllocator_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> copyResourceCmdAllocator_;
	std::array<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>, 2> denoiseCmdAllocator_;

	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> mainGraphicsCmdList_;			// メインで使用するグラフィックスコマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> copyResourceCmdList_;			// CopyResourceで使用するグラフィックスコマンドリスト
	std::array<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>, 2> denoiseCmdList_;	// デノイズで使用するコマンドリスト SwapChainのように裏と表を切り替えて使用する。

	// キュー
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> graphicsCmdQueue_;				// グラフィックスキュー レイトレ関数やUIの描画等を行う。
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> computeCmdQueue_;				// コンピュートキュー 主にデノイズを行う。

	// フェンス
	std::array<Microsoft::WRL::ComPtr<ID3D12Fence>, 2> GPUtoCPUFence_;			// mainGraphicsとCPUの同期をとるためのフェンス
	std::array<Microsoft::WRL::ComPtr<ID3D12Fence>, 2> graphicsToCopyFence_;	// copyの処理をgraphicsの処理が終わった後に実行するためのフェンス。
	std::array<Microsoft::WRL::ComPtr<ID3D12Fence>, 2> finishCopyFence_;		// copy終了監視用フェンス
	std::array<Microsoft::WRL::ComPtr<ID3D12Fence>, 2> graphicsToDenoiseFence_;	// mainGraphicsとdenoiseの同期をとるためのフェンス SwapChainのように裏と表を切り替えて使用する。
	std::array<Microsoft::WRL::ComPtr<ID3D12Fence>, 2> denoiseToCopyFence_;		// denoiseとCopyの同期をとるためのフェンス SwapChainのように裏と表を切り替えて使用する。
	UINT64 fenceValue;
	Microsoft::WRL::ComPtr<ID3D12Fence> asFence_;
	UINT64 asfenceValue_;

	// 現在Queueのインデックス。
	int pastQueueIndex_;
	int currentQueueIndex_;
	int frameIndex_;

	// ゲーム終了フラグ
	bool isGameEndReservation_;
	bool isGameEnd_;


public:
	// コンストラクタ
	Engine();
	// 初期化処理
	void Init();
	// 毎フレーム処理(描画前処理)
	void ProcessBeforeDrawing();
	// 毎フレーム処理(描画後処理)
	void ProcessAfterDrawing();

	// レンダーターゲットをセット
	void SetRenderTarget();
	// 描画後のリソースバリア
	void ResourceBarrierAfter();

	// imgui用ヒープ生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeaoForImgui();
};