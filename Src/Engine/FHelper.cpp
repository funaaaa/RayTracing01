#include "FHelper.h"

bool FHelper::RayToModelCollision(RayToModelCollisionData CollisionData, Vec3& ImpactPos, float& Distance, Vec3& HitNormal)
{

	// �������Ă���|���S���̃f�[�^��ۑ�����悤�ϐ�	�Փ˒n�_�A�����A�Փ˖ʂ̖@��
	struct HitPorygonData
	{
		Vec3 pos;
		float distance;
		Vec3 normal;
	};

	std::vector<HitPorygonData> hitPorygon{};

	/*----- target����|���S���𔲂���� -----*/

	struct CheckHitVertex {

		Vec3 pos;
		Vec3 normal;

	};

	// ���C�Ƃ̓����蔻��p�̃|���S���\����
	struct CheckHitPorygon {
		bool isActive = true;
		CheckHitVertex p1;
		CheckHitVertex p2;
		CheckHitVertex p3;
	};

	std::vector<CheckHitPorygon> targetPorygon;		//�|���S���ۑ��p�R���e�i

	// target�̃|���S�����ɍ��킹�ă��T�C�Y
	targetPorygon.resize(static_cast<unsigned __int64>(static_cast<float>(CollisionData.targetVertex.size()) / 3.0f));

	// �|���S���̒��g����
	int targetPorygonSize = static_cast<int>(targetPorygon.size());
	for (int index = 0; index < targetPorygonSize; ++index) {

		// ���_1
		targetPorygon[index].p1.pos = CollisionData.targetVertex[static_cast<UINT>(CollisionData.targetIndex[static_cast<UINT>(index * 3)])];
		targetPorygon[index].p1.normal = CollisionData.targetNormal[static_cast<UINT>(CollisionData.targetIndex[static_cast<UINT>(index * 3)])];
		// ���_2
		targetPorygon[index].p2.pos = CollisionData.targetVertex[static_cast<UINT>(CollisionData.targetIndex[static_cast<UINT>(index * 3 + 1)])];
		targetPorygon[index].p2.normal = CollisionData.targetNormal[static_cast<UINT>(CollisionData.targetIndex[static_cast<UINT>(index * 3 + 1)])];
		// ���_3
		targetPorygon[index].p3.pos = CollisionData.targetVertex[static_cast<UINT>(CollisionData.targetIndex[static_cast<UINT>(index * 3 + 2)])];
		targetPorygon[index].p3.normal = CollisionData.targetNormal[static_cast<UINT>(CollisionData.targetIndex[static_cast<UINT>(index * 3 + 2)])];
		// �L�����t���O
		targetPorygon[index].isActive = true;
	}

	/*----- �ۑ������|���S���̒��_���W�Ƀ��[���h�ϊ��s��������� -----*/
	// ���[���h�s��
	DirectX::XMMATRIX matWorld = DirectX::XMMatrixIdentity();
	matWorld *= CollisionData.matScale;
	matWorld *= CollisionData.matRot;
	matWorld *= CollisionData.matTrans;
	targetPorygonSize = static_cast<int>(targetPorygon.size());
	for (int index = 0; index < targetPorygonSize; ++index) {
		// ���_��ϊ�
		targetPorygon[index].p1.pos = DirectX::XMVector3Transform(targetPorygon[index].p1.pos.ConvertXMVECTOR(), matWorld);
		targetPorygon[index].p2.pos = DirectX::XMVector3Transform(targetPorygon[index].p2.pos.ConvertXMVECTOR(), matWorld);
		targetPorygon[index].p3.pos = DirectX::XMVector3Transform(targetPorygon[index].p3.pos.ConvertXMVECTOR(), matWorld);
		// �@������]�s�񕪂����ϊ�
		targetPorygon[index].p1.normal = XMVector3Transform(targetPorygon[index].p1.normal.ConvertXMVECTOR(), CollisionData.matRot);
		targetPorygon[index].p1.normal.Normalize();
		targetPorygon[index].p2.normal = XMVector3Transform(targetPorygon[index].p2.normal.ConvertXMVECTOR(), CollisionData.matRot);
		targetPorygon[index].p2.normal.Normalize();
		targetPorygon[index].p3.normal = XMVector3Transform(targetPorygon[index].p3.normal.ConvertXMVECTOR(), CollisionData.matRot);
		targetPorygon[index].p3.normal.Normalize();
	}

	/*----- ���C�̕����Ɩ@�������������Ȃ珜�O -----*/
	for (int index = 0; index < targetPorygonSize; ++index) {
		// �܂���1�ڂ̒��_���`�F�b�N
		if (-0.0001f < targetPorygon[index].p1.normal.Dot(CollisionData.rayDir)) {
			// �����܂ŗ����犮�S�ɔ��Α��������Ă���̂ŁA�폜�t���O�����Ă�
			targetPorygon[index].isActive = false;
			continue;
		}
	}

	/*----- �|���S����3���_�Ƌ��E���̓����蔻����s���A���ȏ㋗�����������ꍇ�͏������΂� -----*/

	/*----- �|���S�����Ƃɓ����蔻�� -----*/
	// ���������̓|���S�����ƂɌv�Z����
	for (int index = 0; index < targetPorygonSize; ++index) {
		/*----- ���C�ƕ��ʂ̏Փ˓_���v�Z���� -----*/

		// �|���S��������������Ă����玟�̏�����
		if (!targetPorygon[index].isActive) continue;

		// ���C�̊J�n�n�_���畽�ʂɂ��낵�������̒��������߂�
		Vec3 planeNorm = -targetPorygon[index].p1.normal;
		float rayToOriginLength = CollisionData.rayPos.Dot(planeNorm);
		float planeToOriginLength = targetPorygon[index].p1.pos.Dot(planeNorm);
		// ���_���畽�ʂɂ��낵�������̒���
		float perpendicularLine = rayToOriginLength - planeToOriginLength;

		// �O�p�֐��𗘗p���Ď��_����Փ˓X�܂ł̋��������߂�
		float dist = planeNorm.Dot(CollisionData.rayDir);
		float impDistance = perpendicularLine / -dist;

		// �Փ˒n�_
		Vec3 impactPoint = CollisionData.rayPos + CollisionData.rayDir * impDistance;

		/*----- �Փ˓_���|���S���̓����ɂ��邩�𒲂ׂ� -----*/
		Vec3 m;

		/* ��1�{�� */
		Vec3 P1ToImpactPos = (impactPoint - targetPorygon[index].p1.pos).GetNormal();
		Vec3 P1ToP2 = (targetPorygon[index].p2.pos - targetPorygon[index].p1.pos).GetNormal();
		Vec3 P1ToP3 = (targetPorygon[index].p3.pos - targetPorygon[index].p1.pos).GetNormal();

		// �Փ˓_�ƕ�1�̓���
		float impactDot = P1ToImpactPos.Dot(P1ToP2);
		// �_1�Ɠ_3�̓���
		float P1Dot = P1ToP2.Dot(P1ToP3);

		// �Փ˓_�ƕ�1�̓��ς��_1�Ɠ_3�̓��ς�菬����������A�E�g
		if (impactDot < P1Dot) {
			targetPorygon.at(index).isActive = false;
			continue;
		}

		/* ��2�{�� */
		Vec3 P2ToImpactPos = (impactPoint - targetPorygon[index].p2.pos).GetNormal();
		Vec3 P2ToP3 = (targetPorygon[index].p3.pos - targetPorygon[index].p2.pos).GetNormal();
		Vec3 P2ToP1 = (targetPorygon[index].p1.pos - targetPorygon[index].p2.pos).GetNormal();

		// �Փ˓_�ƕ�2�̓���
		impactDot = P2ToImpactPos.Dot(P2ToP3);
		// �_2�Ɠ_1�̓���
		float P2Dot = P2ToP3.Dot(P2ToP1);

		// �Փ˓_�ƕ�2�̓��ς��_2�Ɠ_1�̓��ς�菬����������A�E�g
		if (impactDot < P2Dot) {
			targetPorygon.at(index).isActive = false;
			continue;
		}

		/* ��3�{�� */
		Vec3 P3ToImpactPos = (impactPoint - targetPorygon[index].p3.pos).GetNormal();
		Vec3 P3ToP1 = (targetPorygon[index].p1.pos - targetPorygon[index].p3.pos).GetNormal();
		Vec3 P3ToP2 = (targetPorygon[index].p2.pos - targetPorygon[index].p3.pos).GetNormal();

		// �Փ˓_�ƕ�3�̓���
		impactDot = P3ToImpactPos.Dot(P3ToP1);
		// �_3�Ɠ_2�̓���
		float P3Dot = P3ToP1.Dot(P3ToP2);

		// �Փ˓_�ƕ�3�̓��ς��_3�Ɠ_2�̓��ς�菬����������A�E�g
		if (impactDot < P3Dot) {
			targetPorygon.at(index).isActive = false;
			continue;
		}

		/* �����܂ŗ�����|���S���ɏՓ˂��Ă�I */
		HitPorygonData hitPorygonData;
		hitPorygonData.pos = impactPoint;
		hitPorygonData.distance = impDistance;
		hitPorygonData.normal = targetPorygon[index].p1.normal;
		hitPorygon.push_back(hitPorygonData);
	}

	// hitPorygon�̒l��1�ȏゾ�����狗�����ŏ��̗v�f������
	if (1 <= hitPorygon.size()) {
		// �������ŏ��̗v�f������
		int min = 0;
		float minDistance = 100000;
		int counter = 0;
		for (auto& index : hitPorygon) {
			if (fabs(index.distance) < fabs(minDistance)) {
				minDistance = index.distance;
				min = counter;
				++counter;
			}
		}

		//���������ŏ��l��������return
		ImpactPos = hitPorygon[min].pos;
		Distance = hitPorygon[min].distance;
		HitNormal = hitPorygon[min].normal;
		return true;
	}
	else {
		ImpactPos = Vec3{ -1,-1,-1 };
		Distance = -1;
		HitNormal = Vec3{ -1,-1,-1 };
		return false;
	}
}