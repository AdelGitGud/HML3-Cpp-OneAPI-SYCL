#include "LogManager.h"

#include <time.h>
#include <stdarg.h>
#include <string.h>

bool LogManager::Init() {
	time_t t;
	char dateString[51];
	
	t = time(NULL);
	_tzset();
	strftime(dateString, sizeof(dateString) - 1, "%a %b %d %T %Z %Y.txt", localtime(&t));
	
	m.logFile = fopen(dateString, "w");
	return true;
}

void LogManager::Shutdown() {
	fclose(m.logFile);
}

void LogManager::Log(LOG_LEVEL& level, const char* fmt, ...) {
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
	
	printf("\n");
}