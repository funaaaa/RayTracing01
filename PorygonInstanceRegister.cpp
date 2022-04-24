#include "PorygonInstanceRegister.h"
#include <assert.h>

UINT PorygonInstanceRegister::SetRegister(D3D12_RAYTRACING_INSTANCE_DESC instanceDesc)
{

	/*===== InstanceDesc��ۑ����鏈�� =====*/

	// InstanceDesc��ǉ�����B
	instanceDescRegister.push_back(instanceDesc);

	// �Ō����Ԃ��B
	return instanceDescRegister.size() - 1;

}

void PorygonInstanceRegister::AddTrans(const XMMATRIX& transMat, const UINT& id)
{

	/*===== �ړ����� =====*/

	// �ݒ�̈ړ��s����X�V����B
	XMStoreFloat3x4(
		reinterpret_cast<XMFLOAT3X4*>(&instanceDescRegister[id].Transform),
		transMat);

	//instanceDescRegister[id].Transform[0][3] = transMat.r[3].m128_f32[0];
	//instanceDescRegister[id].Transform[1][3] = transMat.r[3].m128_f32[1];
	//instanceDescRegister[id].Transform[2][3] = transMat.r[3].m128_f32[2];

}