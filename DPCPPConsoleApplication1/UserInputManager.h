#pragma once
#include "BaseManager.h"

class UserInputManager : public BaseManager {
public:
	bool Init() override;
	void Shutdown() override;

private:
	struct Members {

	}m;
};
