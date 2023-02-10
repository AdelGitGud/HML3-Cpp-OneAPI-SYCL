#include "LogManager.h"

#ifdef _WIN32
#pragma warning (disable : 4996) // disable deprecated warning
#endif

#include <time.h>
#include <stdarg.h>
#include <string.h>

LogManager::LogManager() {

}

LogManager::~LogManager() {

}

bool LogManager::Init() {
	
	return true;
}

void LogManager::Shutdown() {
	
}

void LogManager::Log(LOG_LEVEL level, const char* fmt, ...) {
	va_list ap;
	time_t t;
	char dateString[51];

	if (level > LOG_LEVEL_INFO || m.printTimeOnInfoLog == true) {
		t = time(NULL);
		_tzset();
		strftime(dateString, sizeof(dateString) - 1, "%a %b %d %T %Z %Y", localtime(&t));
		printf("%s: ", dateString);
	}
	
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}