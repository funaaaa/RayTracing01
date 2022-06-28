#pragma once
#include "BaseSprite.h"

class Sprite : public BaseSprite {

private:

	// 共通の生成処理
	void CommonGenerate(Vec3 centerPos, DirectX::XMFLOAT2 size, int projectionID, int piplineID);

public:

	// 生成処理
	void GenerateForTexture(Vec3 centerPos, DirectX::XMFLOAT2 size, int projectionID, int piplineID, LPCWSTR fileName);
	void GenerateForColor(Vec3 centerPos, DirectX::XMFLOAT2 size, int projectionID, int piplineID, DirectX::XMFLOAT4 color);
	void GenerateSpecifyTextureID(Vec3 centerPos, DirectX::XMFLOAT2 size, int projectionID, int piplineID, int textureID);

	// 描画処理
	void Draw();

	// 色を変える処理
	inline void SetColor(const DirectX::XMFLOAT4& color) { constBufferDataB0.color = color; }

};