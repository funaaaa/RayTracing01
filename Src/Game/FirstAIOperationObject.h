#pragma once
#include "BaseOperationObject.h"
#include <memory>

class FirstAIWayPointMgr;

class FirstAIOperationObject : public BaseOperationObject {

private:

	/*===== メンバ変数 =====*/

	std::shared_ptr<FirstAIWayPointMgr> waypointMgr_;


public:

	/*===== メンバ関数 =====*/

	FirstAIOperationObject(const int& WayPointOffset);

	BaseOperationObject::Operation Input(const BaseOperationObject::OperationInputData& InputData)override;

	void OnGoal()override;


};