#pragma once
#include <vector>
#include <string>
#include <DirectXMath.h>
#include "Vec.h"

// Blenderで設定した頂点からオブジェクトを生成するクラス。現在はアイテムボックスで使用。
class ConvertGimmickInfoFromBlender {

private:

	/*===== メンバ変数 =====*/

	DirectX::XMMATRIX matTrans_;
	DirectX::XMMATRIX matRot_;
	DirectX::XMMATRIX matScale_;
	std::vector<Vec3> vertex_;


public:

	/*===== メンバ関数 =====*/

	void Convert(std::string ModelPath, std::string ModelName, const DirectX::XMMATRIX& MatTrans, const DirectX::XMMATRIX& MatRot, const DirectX::XMMATRIX& MatScale);

	// 指定したインデックスの頂点を取得。
	Vec3 GetVertex(int Index);

	int GetVertexSize() { return static_cast<int>(vertex_.size()); }

};