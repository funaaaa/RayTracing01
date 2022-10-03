#include "RoughnessBlur.h"
#include "RaytracingOutput.h"
#include "RayComputeShader.h"
#include "DynamicConstBuffer.h"
#include "DirectXBase.h"
#include "WindowsAPI.h"

void RoughnessBlur::Setting()
{

	/*===== �R���X�g���N�^ =====*/

	// �u���[�o�͗p�N���X�𐶐��B
	blurXOutput_ = std::make_shared<RaytracingOutput>();
	blurYOutput_ = std::make_shared<RaytracingOutput>();

	// �u���[�o�̓e�X�g�p�N���X���Z�b�g�B
	blurXOutput_->Setting(DXGI_FORMAT_R8G8B8A8_UNORM, L"DenoiseBlurXOutput");
	blurYOutput_->Setting(DXGI_FORMAT_R8G8B8A8_UNORM, L"DenoiseBlurYOutput");

	// �f�m�C�Y���ɔr�o����p�N���X���Z�b�g�B
	denoiseOutput_ = std::make_shared<RaytracingOutput>();
	denoiseOutput_->Setting(DXGI_FORMAT_R8G8B8A8_UNORM, L"DenoiseBlurBuffOutput");

	// �g�p����R���s���[�g�V�F�[�_�[�𐶐��B
	blurX_ = std::make_shared<RayComputeShader>();
	blurY_ = std::make_shared<RayComputeShader>();

	// �K�E�V�A���u���[�Ɏg�p����R���s���[�g�V�F�[�_�[���Z�b�g�B
	blurX_->Setting(L"Resource/ShaderFiles/RayTracing/RoughnessBlurX.hlsl", 0, 1, 3, { 0 });
	blurY_->Setting(L"Resource/ShaderFiles/RayTracing/RoughnessBlurY.hlsl", 0, 1, 3, { blurXOutput_->GetUAVIndex() });

	// �K�E�V�A���u���[�̏d�݃e�[�u�����v�Z�B
	CalcWeightsTableFromGaussian(10);

	// �萔�o�b�t�@�𐶐��B
	weightTableCBX_ = std::make_shared<DynamicConstBuffer>();
	weightTableCBX_->Generate(sizeof(float) * GAUSSIAN_WEIGHTS_COUNT, L"GaussianWeightCBX");
	weightTableCBY_ = std::make_shared<DynamicConstBuffer>();
	weightTableCBY_->Generate(sizeof(float) * GAUSSIAN_WEIGHTS_COUNT, L"GaussianWeightCBY");

}

void RoughnessBlur::ApplyGaussianBlur(const int& InputReflectionIndex, const int& InputRoughnessIndex, const int& DenoiseMaskIndex, const int& OutputUAVIndex, const int& BlurPower)
{

	/*===== �f�m�C�Y���� =====*/

	// �d�݃e�[�u�����v�Z�B
	CalcWeightsTableFromGaussian(static_cast<float>(BlurPower));

	// �d�݃e�[�u�����������ށB
	weightTableCBX_->Write(DirectXBase::Ins()->swapchain_->GetCurrentBackBufferIndex(), gaussianWeights_.data(), sizeof(float) * GAUSSIAN_WEIGHTS_COUNT);
	weightTableCBY_->Write(DirectXBase::Ins()->swapchain_->GetCurrentBackBufferIndex(), gaussianWeights_.data(), sizeof(float) * GAUSSIAN_WEIGHTS_COUNT);

	// �o�͗pUAV�̏�Ԃ�ς���B
	blurXOutput_->SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	blurYOutput_->SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	denoiseOutput_->SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// �R���s���[�g�V�F�[�_�[�����s�B
	blurX_->ChangeInputUAVIndex({ InputReflectionIndex, InputRoughnessIndex, DenoiseMaskIndex });
	blurY_->ChangeInputUAVIndex({ blurXOutput_->GetUAVIndex(), DenoiseMaskIndex });
	blurX_->Dispatch(static_cast<UINT>(WINDOW_WIDTH / 32) + 1, static_cast<UINT>(WINDOW_HEIGHT / 32) + 1, static_cast<UINT>(1), blurXOutput_->GetUAVIndex(), { weightTableCBX_->GetBuffer(DirectXBase::Ins()->swapchain_->GetCurrentBackBufferIndex())->GetGPUVirtualAddress() });
	blurY_->Dispatch(static_cast<UINT>((WINDOW_WIDTH / 1.0f) / 32) + 1, static_cast<UINT>((WINDOW_HEIGHT / 1.0f) / 32) + 1, static_cast<UINT>(1), OutputUAVIndex, { weightTableCBY_->GetBuffer(DirectXBase::Ins()->swapchain_->GetCurrentBackBufferIndex())->GetGPUVirtualAddress() });

	// �o�͗pUAV�̏�Ԃ�ς���B
	blurXOutput_->SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	blurYOutput_->SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	denoiseOutput_->SetResourceBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

}

void RoughnessBlur::Denoise(const int& InputReflectionIndex, const int& InputRoughnessIndex, const int& DenoiseMaskIndex, const int& OutputUAVIndex, const int& DenoisePower, const int& DenoiseCount)
{

	/*===== �f�m�C�Y =====*/

	// �f�m�C�Y���鐔��1�񂾂�����B
	if (DenoiseCount == 1) {

		// �K�E�V�A���u���[��������B
		ApplyGaussianBlur(InputReflectionIndex, InputRoughnessIndex, DenoiseMaskIndex, OutputUAVIndex, DenoisePower);

	}
	// �f�m�C�Y���鐔��2�񂾂�����B
	else if (DenoiseCount == 2) {

		// �K�E�V�A���u���[��������B
		ApplyGaussianBlur(InputReflectionIndex, InputRoughnessIndex, DenoiseMaskIndex, OutputUAVIndex, DenoisePower);
		ApplyGaussianBlur(denoiseOutput_->GetUAVIndex(), InputRoughnessIndex, DenoiseMaskIndex, OutputUAVIndex, DenoisePower);

	}
	else {

		for (int index = 0; index < DenoiseCount; ++index) {

			// �f�m�C�Y���ŏ��̈�񂾂�����B
			if (index == 0) {

				ApplyGaussianBlur(InputReflectionIndex, InputRoughnessIndex, DenoiseMaskIndex, denoiseOutput_->GetUAVIndex(), DenoisePower);

			}
			// �f�m�C�Y�̍ŏI�i�K��������B
			else if (index == DenoiseCount - 1) {

				ApplyGaussianBlur(denoiseOutput_->GetUAVIndex(), InputRoughnessIndex, DenoiseMaskIndex, OutputUAVIndex, DenoisePower);

			}
			else {

				ApplyGaussianBlur(denoiseOutput_->GetUAVIndex(), InputRoughnessIndex, DenoiseMaskIndex, denoiseOutput_->GetUAVIndex(), DenoisePower);

			}

		}

	}

}

void RoughnessBlur::CalcWeightsTableFromGaussian(float Power)
{

	/*===== �K�E�V�A���u���[�̏d�݂��v�Z =====*/

	// �d�݂̍��v���L�^����ϐ����`����B
	float total = 0;

	// ��������K�E�X�֐���p���ďd�݂��v�Z���Ă���B
	// ���[�v�ϐ���x����e�N�Z������̋����B
	for (int x_ = 0; x_ < GAUSSIAN_WEIGHTS_COUNT; x_++)
	{
		gaussianWeights_[x_] = expf(-0.5f * static_cast<float>(x_ * x_) / Power);
		total += 2.0f * gaussianWeights_.at(x_);
	}

	// �d�݂̍��v�ŏ��Z���邱�ƂŁA�d�݂̍��v��1�ɂ��Ă���B
	for (int i = 0; i < GAUSSIAN_WEIGHTS_COUNT; i++)
	{
		gaussianWeights_[i] /= total;
	}

}