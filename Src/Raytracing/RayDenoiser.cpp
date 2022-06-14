#include "RayDenoiser.h"
#include "RaytracingOutput.h"
#include "RayComputeShader.h"
#include "DynamicConstBuffer.h"
#include "DirectXBase.h"

void Denoiser::Setting()
{

	/*===== コンストラクタ =====*/

	// ブラー出力用クラスを生成。
	blurXOutput = std::make_shared<RaytracingOutput>();
	blurYOutput = std::make_shared<RaytracingOutput>();

	// ブラー出力テスト用クラスをセット。
	blurXOutput->Setting(DXGI_FORMAT_R8G8B8A8_UNORM);
	blurYOutput->Setting(DXGI_FORMAT_R8G8B8A8_UNORM);

	// デノイズ時に排出する用クラスをセット。
	denoiseOutput = std::make_shared<RaytracingOutput>();
	denoiseOutput->Setting(DXGI_FORMAT_R8G8B8A8_UNORM);

	// 使用するコンピュートシェーダーを生成。
	blurX = std::make_shared<RayComputeShader>();
	blurY = std::make_shared<RayComputeShader>();
	blurFinal = std::make_shared<RayComputeShader>();
	mixColorAndLuminance = std::make_shared<RayComputeShader>();

	// ガウシアンブラーに使用するコンピュートシェーダーをセット。
	blurX->Setting(L"Resource/ShaderFiles/RayTracing/DenoiseBlurX.hlsl", 0, 1, 1, { 0 });
	blurY->Setting(L"Resource/ShaderFiles/RayTracing/DenoiseBlurY.hlsl", 0, 1, 1, { blurXOutput->GetUAVIndex() });
	blurFinal->Setting(L"Resource/ShaderFiles/RayTracing/DenoiseFinal.hlsl", 0, 0, 1, { blurYOutput->GetUAVIndex() });
	mixColorAndLuminance->Setting(L"Resource/ShaderFiles/RayTracing/MixColorAndLuminance.hlsl", 0, 0, 4, { 0,0 });

	// ガウシアンブラーの重みテーブルを計算。
	CalcWeightsTableFromGaussian(10);

	// 定数バッファを生成。
	weightTableCBX = std::make_shared<DynamicConstBuffer>();
	weightTableCBX->Generate(sizeof(float) * GAUSSIAN_WEIGHTS_COUNT, L"GaussianWeightCBX");
	weightTableCBY = std::make_shared<DynamicConstBuffer>();
	weightTableCBY->Generate(sizeof(float) * GAUSSIAN_WEIGHTS_COUNT, L"GaussianWeightCBY");

}

void Denoiser::ApplyGaussianBlur(const int& InputUAVIndex, const int& OutputUAVIndex, const int& BlurPower)
{

	/*===== デノイズ処理 =====*/

	// 重みテーブルを計算。
	CalcWeightsTableFromGaussian(BlurPower);

	// 重みテーブルを書き込む。
	weightTableCBX->Write(DirectXBase::Ins()->swapchain->GetCurrentBackBufferIndex(), gaussianWeights.data(), sizeof(float) * GAUSSIAN_WEIGHTS_COUNT);
	weightTableCBY->Write(DirectXBase::Ins()->swapchain->GetCurrentBackBufferIndex(), gaussianWeights.data(), sizeof(float) * GAUSSIAN_WEIGHTS_COUNT);

	// 出力用UAVの状態を変える。
	blurXOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	blurYOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// コンピュートシェーダーを実行。
	blurX->ChangeInputUAVIndex({ InputUAVIndex });
	blurX->Dispatch((window_width / 2.0f) / 4, window_height / 4, 1, blurXOutput->GetUAVIndex(), { weightTableCBX->GetBuffer(DirectXBase::Ins()->swapchain->GetCurrentBackBufferIndex())->GetGPUVirtualAddress() });
	blurY->Dispatch((window_width / 2.0f) / 4, (window_height / 2.0f) / 4, 1, blurYOutput->GetUAVIndex(), { weightTableCBY->GetBuffer(DirectXBase::Ins()->swapchain->GetCurrentBackBufferIndex())->GetGPUVirtualAddress() });
	blurFinal->Dispatch(window_width / 4, window_height / 4, 1, OutputUAVIndex, {});
	//blurX->ChangeInputUAVIndex({ InputUAVIndex });
	//blurX->Dispatch((window_width / 1.0f) / 4, window_height / 4, 1, blurXOutput->GetUAVIndex(), { weightTableCBX->GetBuffer(DirectXBase::Ins()->swapchain->GetCurrentBackBufferIndex())->GetGPUVirtualAddress() });
	//blurY->Dispatch((window_width / 1.0f) / 4, (window_height / 1.0f) / 4, 1, blurYOutput->GetUAVIndex(), { weightTableCBY->GetBuffer(DirectXBase::Ins()->swapchain->GetCurrentBackBufferIndex())->GetGPUVirtualAddress() });
	//blurFinal->Dispatch(window_width / 4, window_height / 4, 1, OutputUAVIndex, {});

	// 出力用UAVの状態を変える。
	blurXOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	blurYOutput->SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

}

void Denoiser::MixColorAndLuminance(const int& InputColorIndex, const int& InputLuminanceIndex, const int& InputLightLuminanceIndex, const int& InputGIIndex, const int& OutputUAVIndex)
{

	/*===== 色情報と明るさ情報の乗算 =====*/

	// コンピュートシェーダーを実行。
	mixColorAndLuminance->ChangeInputUAVIndex({ InputColorIndex,InputLuminanceIndex, InputLightLuminanceIndex, InputGIIndex });
	mixColorAndLuminance->Dispatch(window_width / 4, window_height / 4, 1, OutputUAVIndex, {});

}

void Denoiser::Denoise(const int& InOutImg, const int& DenoisePower, const int& DenoiseCount)
{

	/*===== デノイズ =====*/

	// デノイズする数が1回だったら。
	if (DenoiseCount == 1) {

		// ガウシアンブラーをかける。
		ApplyGaussianBlur(InOutImg, InOutImg, DenoisePower);

	}
	// デノイズする数が2回だったら。
	else if (DenoiseCount == 2) {

		// ガウシアンブラーをかける。
		ApplyGaussianBlur(InOutImg, denoiseOutput->GetUAVIndex(), DenoisePower);
		ApplyGaussianBlur(denoiseOutput->GetUAVIndex(), InOutImg, DenoisePower);

	}
	else {

		for (int index = 0; index < DenoiseCount; ++index) {

			// デノイズが最初の一回だったら。
			if (index == 0) {

				ApplyGaussianBlur(InOutImg, denoiseOutput->GetUAVIndex(), DenoisePower);

			}
			// デノイズの最終段階だったら。
			else if (index == DenoiseCount - 1) {

				ApplyGaussianBlur(denoiseOutput->GetUAVIndex(), InOutImg, DenoisePower);

			}
			else {

				ApplyGaussianBlur(denoiseOutput->GetUAVIndex(), denoiseOutput->GetUAVIndex(), DenoisePower);

			}

		}

	}


}

void Denoiser::CalcWeightsTableFromGaussian(float Pwer)
{

	/*===== ガウシアンブラーの重みを計算 =====*/

	// 重みの合計を記録する変数を定義する。
	float total = 0;

	// ここからガウス関数を用いて重みを計算している。
	// ループ変数のxが基準テクセルからの距離。
	for (int x = 0; x < GAUSSIAN_WEIGHTS_COUNT; x++)
	{
		gaussianWeights[x] = expf(-0.5f * (float)(x * x) / Pwer);
		total += 2.0f * gaussianWeights.at(x);
	}

	// 重みの合計で除算することで、重みの合計を1にしている。
	for (int i = 0; i < GAUSSIAN_WEIGHTS_COUNT; i++)
	{
		gaussianWeights[i] /= total;
	}

}
