#pragma once
#include "BaseManager.h"

#include <stdio.h>

enum LOG_LEVEL { LOG_LEVEL_INFO,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_FATAL
};

class LogManager : public BaseManager {
public:
	bool Init() override;
	void Shutdown() override;
	void Log(LOG_LEVEL& level, const char* fmt, ...);
	
	inline void SetFilePrintLevel(LOG_LEVEL& level) {
		m.filePrintLevel = level;
	};

	inline LOG_LEVEL GetFilePrintLevel() {
		return m.filePrintLevel;
	};

	inline void SetPrintTimeOnInfoLog(bool& printTime) {
		m.printTimeOnInfoLog = printTime;
	};
	
	inline bool GetPrintTimeOnInfoLog() {
		return m.printTimeOnInfoLog;
	};
	
	inline void SetPrintTimeOnInfoLog(LOG_LEVEL& level) {
		m.printTimeOnInfoLog = level;
	};

private:
	struct Members {
		LOG_LEVEL filePrintLevel = LOG_LEVEL_FATAL;
		bool printTimeOnInfoLog = false;
		FILE* logFile = nullptr;
	}m;
};

