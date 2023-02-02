#pragma once
class BaseManager {
public:
	virtual ~BaseManager() = 0;
	virtual bool Init() = 0;
	virtual void Shutdown() = 0;
};