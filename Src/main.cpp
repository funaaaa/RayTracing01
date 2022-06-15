#include "DevDXR.h"

#define COLORHEX(hex) hex / 255.0f

#define SCREEN_VIRTUAL_WIDTH 300

<<<<<<< HEAD
// fps�X�V
void FPS();

XMFLOAT2 CalRepulsionRatio(const float& v1, const float& m1, const float& v2, const float& m2)
{
	XMFLOAT2 honraiVel = { 0,0 };

	//�����W��
	float e = 1.0f;

	float honraiV1 = ((m1 - m2) * v1 + (2 * m2 * v2)) / (m1 + m2);
	float honraiV2 = ((m2 - m1) * v2 + (2 * m1 * v1)) / (m1 + m2);


	honraiVel.x = honraiV1;
	honraiVel.y = honraiV2;

	return honraiVel;
}

struct RayPointLightData {

	Vec3 lightPos;
	float lightSize;
	Vec3 lightColor;
	float lightPower;
	int isActive;
	Vec3 pad;

};

struct KariConstBufferData {

	XMMATRIX mtxView;			// �r���[�s��B
	XMMATRIX mtxProj;			// �v���W�F�N�V�����s��B
	XMMATRIX mtxViewInv;		// �r���[�t�s��B
	XMMATRIX mtxProjInv;		// �v���W�F�N�V�����t�s��B
	XMVECTOR lightDirection;	// ���s�����̌����B
	XMVECTOR lightColor;		// ���s�����F�B
	XMVECTOR ambientColor;		// �����B
	RayPointLightData pointLight;
	int seed;
	int counter;
	int aoSampleCount;
	int isNoiseScene;
	int isNoiseOnlyScene;
	int isLightHitScene;
	int isNormalScene;
	int isMeshScene;
	int isNoAO;

};

// �f�o�b�O�p�̃p�C�v���C����؂�ւ����B
enum DEGU_PIPLINE_ID {
	DEF_PIPLINE,
	AO_PIPLINE,
	DENOISE_AO_PIPLINE,
};

// ���͑���
void Input(KariConstBufferData& constBufferData, bool& isMoveLight, DEGU_PIPLINE_ID& degugPiplineID);

=======
>>>>>>> AtmosphericScattering
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	/*----------DirectX初期化処理----------*/
	ImGuiWindow::Ins()->Init();
	DirectXBase::Ins()->Init();									// DirectX基盤の初期化
	SoundManager::Ins()->SettingSoundManager();	// サウンドマネージャーをセットする

	/*----------パイプライン生成----------*/
	PiplineManager::Ins()->Init();

	/*----------変数宣言----------*/
	srand(time(NULL));

	// ディスクリプタヒープを初期化。
	DescriptorHeapMgr::Ins()->GenerateDescriptorHeap();

	// FBXLoaderを初期化。
	FbxLoader::Ins()->Init();

	// ヒットグループを設定。
	HitGroupMgr::Ins()->Setting();

	// デノイズ用のクラスを初期化。
	Denoiser::Ins()->Setting();

<<<<<<< HEAD
	// SPONZA��ǂݍ��ށB
	//std::vector<int> sponzaInstance = MultiMeshLoadOBJ::Ins()->RayMultiMeshLoadOBJ("Resource/", "sponza.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF_HIT_GROUP]);

	// ���C�g�p�̃X�t�B�A��ǂݍ��ށB
	int sphereBlas = BLASRegister::Ins()->GenerateObj("Resource/", "sphere.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::AO_HIT_GROUP], { L"Resource/white.png" });
	int sphereIns = PorygonInstanceRegister::Ins()->CreateInstance(sphereBlas, 3);
	PorygonInstanceRegister::Ins()->AddScale(sphereIns, Vec3(15, 15, 15));
	PorygonInstanceRegister::Ins()->ChangeTrans(sphereIns, Vec3(0, 350, 100));

	int sphereIns2 = PorygonInstanceRegister::Ins()->CreateInstance(sphereBlas, 3);
	PorygonInstanceRegister::Ins()->AddScale(sphereIns2, Vec3(10, 10, 10));
	PorygonInstanceRegister::Ins()->ChangeTrans(sphereIns2, Vec3(0, 250, 0));

	int sphereIns3 = PorygonInstanceRegister::Ins()->CreateInstance(sphereBlas, 3);
	PorygonInstanceRegister::Ins()->AddScale(sphereIns3, Vec3(12, 12, 12));
	PorygonInstanceRegister::Ins()->ChangeTrans(sphereIns3, Vec3(0, 350, 100));




	Vec3 pos1 = Vec3(0, 10, -60);

	Vec3 pos2 = Vec3(0, -10, 60);

	Vec3 pos3 = Vec3(0, -10, 60);

	float angle = 0;
	float radius = 100;



	PorygonInstanceRegister::Ins()->CalWorldMat();

	// TLAS�𐶐��B
	TLAS tlas;
	tlas.GenerateTLAS(L"TlasDescriptorHeap");

	// ���C�g���o�͗p�N���X���Z�b�g�B
	RaytracingOutput raytracingOutput;
	raytracingOutput.Setting(DXGI_FORMAT_R8G8B8A8_UNORM);

	RaytracingOutput raytracingOutputData;
	raytracingOutputData.Setting(DXGI_FORMAT_R32G32B32A32_FLOAT);

	// �V�F�[�_�[�e�[�u���𐶐��B
	aoPipline.ConstructionShaderTable();
	deAOPipline.ConstructionShaderTable();
	defPipline.ConstructionShaderTable();

	// �f�m�C�Y�p�̃K�E�V�A���u���[�R���s���[�g�V�F�[�_�[���Z�b�g�B
	//ComputeShader blurX;
	//blurX.Init(L"Resource/hlsl/Raytracing/GaussianBlur.hlsl",)

	// ���̒萔�o�b�t�@��錾
	KariConstBufferData constBufferData;
	constBufferData.ambientColor = { 1,1,1,1 };
	constBufferData.lightColor = { 1,1,1,1 };
	constBufferData.lightDirection = { 0,1,-0.27f,0 };
	constBufferData.lightDirection = XMVector4Normalize(constBufferData.lightDirection);
	constBufferData.mtxProj = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(60.0f),				//��p(60�x)
		(float)window_width / window_height,	//�A�X�y�N�g��
		0.1f, 1000000.0f							//�O�[�A���[
	);
	constBufferData.mtxProjInv = XMMatrixInverse(nullptr, constBufferData.mtxProj);
	Vec3 eye = { 0,0,-10 };
	Vec3 target = { 0,0,0 };
	Vec3 up = { 0,1,0 };
	constBufferData.mtxView = XMMatrixLookAtLH(eye.ConvertXMVECTOR(), target.ConvertXMVECTOR(), up.ConvertXMVECTOR());
	constBufferData.mtxViewInv = XMMatrixInverse(nullptr, constBufferData.mtxView);
	constBufferData.counter = 0;
	constBufferData.isNoiseScene = true;
	constBufferData.isNoiseOnlyScene = false;
	constBufferData.pointLight.lightPos = Vec3(0, 300, 0);
	constBufferData.pointLight.lightSize = 30.0f;
	constBufferData.pointLight.lightPower = 300.0f;
	constBufferData.aoSampleCount = 1;
	constBufferData.isLightHitScene = false;
	constBufferData.isNormalScene = false;
	constBufferData.isMeshScene = false;
	constBufferData.isNoAO = false;

	DynamicConstBuffer constBuff;
	constBuff.Generate(sizeof(KariConstBufferData), L"constBuffer");

	// �f�o�b�O�p�Ńm�C�Y��ʂ��o���t���O�B
	DEGU_PIPLINE_ID debugPiplineID = AO_PIPLINE;

	// �J�������������B
	Camera::Ins()->Init();

	bool isHit = false;

	/*----------�Q�[�����[�v----------*/
	while (true) {
		/*----------���t���[������(�`��O����)----------*/
		directXBase.processBeforeDrawing();

		/*----- �X�V���� -----*/

		// �r���[�s��𐶐��B
		Camera::Ins()->GenerateMatView();

		FPS();




		// �傫����
		PorygonInstanceRegister::Ins()->ChangeTrans(sphereIns, pos1);
		// ��������
		PorygonInstanceRegister::Ins()->ChangeTrans(sphereIns2, pos2);
		PorygonInstanceRegister::Ins()->ChangeTrans(sphereIns3, pos3);

		// ������������]������B
		angle += 0.1f;

		radius += 0.1f;

		pos2 = pos1 + Vec3(cosf(angle), 0, sinf(angle)) * radius;
		pos3 = pos1 + Vec3(cosf(angle), 0, sinf(angle)) * (radius * 2.0f);


		// �����̎���X�V�B
		constBufferData.seed = FHelper::GetRand(0, 1000);

		// ���C�g����������
		bool isMoveLight = false;
		constBufferData.counter = 0;

		Input(constBufferData, isMoveLight, debugPiplineID);

		// �J�������X�V�B
		Camera::Ins()->Update();

		eye = Camera::Ins()->eye;
		target = Camera::Ins()->target;
		up = Camera::Ins()->up;

		// ���C�g���������Ƃ��̂݁A���[���h�s����Čv�Z����TLAS���X�V����B
		//if (isMoveLight) {

			//PorygonInstanceRegister::Ins()->ChangeTrans(sphereIns, constBufferData.pointLight.lightPos);
			//PorygonInstanceRegister::Ins()->ChangeScale(sphereIns, constBufferData.pointLight.lightSize);
			//PorygonInstanceRegister::Ins()->ChangeTrans(sphereIns2, constBufferData.pointLight.lightPos);
			//PorygonInstanceRegister::Ins()->ChangeScale(sphereIns2, constBufferData.pointLight.lightSize);

		tlas.Update();

		//}

		/*----- �`�揈�� -----*/

		// ��ʂɕ\������郌���_�[�^�[�Q�b�g�ɖ߂��B
		//DirectXBase::Ins()->SetRenderTarget();

		RaytracingPipline setPipline = {};

		// �f�o�b�O�p�̃p�C�v���C��ID�ɉ������p�C�v���C�����Z�b�g����B
		if (debugPiplineID == DEF_PIPLINE) {

			constBufferData.counter = 0;
			setPipline = defPipline;

		}
		else if (debugPiplineID == AO_PIPLINE) {

			setPipline = aoPipline;

		}
		else if (debugPiplineID == DENOISE_AO_PIPLINE) {

			setPipline = deAOPipline;

		}

		auto frameIndex = DirectXBase::Ins()->swapchain->GetCurrentBackBufferIndex();
		constBufferData.mtxView = XMMatrixLookAtLH(eye.ConvertXMVECTOR(), target.ConvertXMVECTOR(), up.ConvertXMVECTOR());
		constBufferData.mtxViewInv = XMMatrixInverse(nullptr, constBufferData.mtxView);
		// �萔�o�b�t�@�̒��g���X�V����B
		constBuff.Write(frameIndex, &constBufferData, sizeof(KariConstBufferData));
		auto sceneConstantBuffer = constBuff.GetBuffer(frameIndex);

		// �O���[�o�����[�g�V�O�l�`���Ŏg���Ɛ錾���Ă��郊�\�[�X����Z�b�g�B
		ID3D12DescriptorHeap* descriptorHeaps[] = { DescriptorHeapMgr::Ins()->GetDescriptorHeap().Get() };
		DirectXBase::Ins()->cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		DirectXBase::Ins()->cmdList->SetComputeRootSignature(setPipline.GetGlobalRootSig()->GetRootSig().Get());
		DirectXBase::Ins()->cmdList->SetComputeRootDescriptorTable(0, DescriptorHeapMgr::Ins()->GetGPUHandleIncrement(tlas.GetDescriptorHeapIndex()));
		raytracingOutput.SetComputeRootDescriptorTalbe(2);
		raytracingOutputData.SetComputeRootDescriptorTalbe(3);
		DirectXBase::Ins()->cmdList->SetComputeRootConstantBufferView(1, sceneConstantBuffer->GetGPUVirtualAddress());


		// ���C�g���[�V���O���ʃo�b�t�@��UAV��Ԃ�
		raytracingOutput.SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		raytracingOutputData.SetResourceBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		DirectXBase::Ins()->cmdList->SetPipelineState1(setPipline.GetStateObject().Get());

		DirectXBase::Ins()->cmdList->DispatchRays(&setPipline.GetDispatchRayDesc());

		// �f�o�b�O�p�̃p�C�v���C�����f�m�C�Y�p�p�C�v���C����������A�R���s���[�g�V�F�[�_�[���g���ăf�m�C�Y��������B
		if (debugPiplineID == DENOISE_AO_PIPLINE) {



		}

		// �o�b�N�o�b�t�@�̃C���f�b�N�X���擾����B
		UINT backBufferIndex = DirectXBase::Ins()->swapchain->GetCurrentBackBufferIndex();
=======
	// カメラを初期化。
	Camera::Ins()->Init();

	// 開発用
	DevDXR dev;
	dev.Init();
>>>>>>> AtmosphericScattering

	/*----------ゲームループ----------*/
	while (true) {

		
		dev.Update();
		dev.Draw();
		

	}

	return 0;
<<<<<<< HEAD
}


// �^�C�g���o�[��FPS�̍X�V
void FPS()
{
	static DWORD prev_time = timeGetTime();	// �O��̎���
	static int frame_count = 0;		// �t���[���J�E���g
	DWORD now_time = timeGetTime();		// ����̃t���[���̎���

	frame_count++;	// �t���[�������J�E���g����

	// �o�ߎ��Ԃ��P�b�𒴂�����J�E���g�Ǝ��Ԃ����Z�b�g
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

void Input(KariConstBufferData& constBufferData, bool& isMoveLight, DEGU_PIPLINE_ID& debugPiplineID) {

	bool isMove = false;

	float speed = 5.0f;
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

	// DirLight�ɂ���

	// �K�w�\���ɂ���B
	if (ImGui::TreeNode("Lighting")) {

		// �l��ۑ�����B
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

		// �ς���Ă�����
		if (dirX != constBufferData.pointLight.lightPos.x || dirY != constBufferData.pointLight.lightPos.y || dirZ != constBufferData.pointLight.lightPos.z || lightSize != constBufferData.pointLight.lightSize || pointLightPower != constBufferData.pointLight.lightPower) {

			isMove = true;
			isMoveLight = true;

		}

		// ���C�g�̐F��ݒ�B
		array<float, 3> lightColor = { constBufferData.pointLight.lightColor.x,constBufferData.pointLight.lightColor.y,constBufferData.pointLight.lightColor.z };
		ImGui::ColorPicker3("LightColor", lightColor.data());
		// �F���ς���Ă�����B
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

	// ���b�V����\������B
	bool isMesh = constBufferData.isMeshScene;
	bool prevIsMesh = isMesh;
	ImGui::Checkbox("Mesh Scene", &isMesh);
	constBufferData.isMeshScene = isMesh;
	// �l�������������Ă�����A�T���v�����O������������B
	if (isMesh != prevIsMesh) {
		constBufferData.counter = 0;
	}

	// �@����\������B
	bool isNormal = constBufferData.isNormalScene;
	bool prevIsNormal = isNormal;
	ImGui::Checkbox("Normal Scene", &isNormal);
	constBufferData.isNormalScene = isNormal;
	// �l�������������Ă�����A�T���v�����O������������B
	if (isNormal != prevIsNormal) {
		constBufferData.counter = 0;
	}

	// ���C�g�����������ʂ����\������t���O���X�V�B
	bool isLightHit = constBufferData.isLightHitScene;
	bool prevIsLightHit = isLightHit;
	ImGui::Checkbox("LightHit Scene", &isLightHit);
	constBufferData.isLightHitScene = isLightHit;
	// �l�������������Ă�����A�T���v�����O������������B
	if (isLightHit != prevIsLightHit) {
		constBufferData.counter = 0;
	}

	// �p�C�v���C����I���B
	int debugPiplineBuff = debugPiplineID;
	ImGui::RadioButton("DEF PIPLINE", &debugPiplineBuff, 0);
	ImGui::SameLine();
	ImGui::RadioButton("AO PIPLINE", &debugPiplineBuff, 1);
	ImGui::SameLine();
	ImGui::RadioButton("DENOISE AO PIPLINE", &debugPiplineBuff, 2);
	debugPiplineID = (DEGU_PIPLINE_ID)debugPiplineBuff;

	// AO�̃p�C�v���C����I������Ă����Ƃ��̂݁A�m�C�Y���o�����̃t���O��\������B
	if (debugPiplineID == AO_PIPLINE) {

		// �f�o�b�O�p�Ńm�C�Y��ʂ��o�����߂̃t���O���Z�b�g�B
		bool isNoise = constBufferData.isNoiseScene;
		ImGui::Checkbox("Noise Scene", &isNoise);
		constBufferData.isNoiseScene = isNoise;

		// �f�o�b�O�p�Ńm�C�Y��ʂ݂̂��o�����߂̃t���O���Z�b�g�B
		bool isNoiseOnly = constBufferData.isNoiseOnlyScene;
		ImGui::Checkbox("NoiseOnly Scene", &isNoiseOnly);
		// �t���O������������Ă�����f�m�C�Y�J�E���^�[������������B
		if (isNoiseOnly != constBufferData.isNoiseOnlyScene) {
			constBufferData.counter = 0;
		}
		constBufferData.isNoiseOnlyScene = isNoiseOnly;

		// �A���r�G���g�I�N�����[�W�������s�����̃t���O���Z�b�g�B
		bool isNoAO = constBufferData.isNoAO;
		ImGui::Checkbox("NoAO Scene", &isNoAO);
		// �t���O������������Ă�����f�m�C�Y�J�E���^�[������������B
		if (isNoAO != constBufferData.isNoAO) {
			constBufferData.counter = 0;
		}
		constBufferData.isNoAO = isNoAO;

	}

=======
>>>>>>> AtmosphericScattering
}