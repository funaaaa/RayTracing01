#pragma once
#include "Singleton.h"

class BrightnessParam : public Singleton<BrightnessParam> {

public:

	/*===== メンバ変数 =====*/

	bool isBright_;

};