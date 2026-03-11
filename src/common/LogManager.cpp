#include "LogManager.h"

#ifdef _WIN32
    #pragma warning (disable : 4996) // disable deprecated warning

    #include <time.h>
    #define tzset _tzset
#else
    #include <time.h>
#endif

#include <stdarg.h>
#include <string.h>

bool LogManager::Init() {
	time_t t;
	char dateString[51];
	
	t = time(NULL);
	tzset();
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
		tzset();
		strftime(dateString, sizeof(dateString) - 1, "%a %b %d %T %Z %Y", localtime(&t));
		printf("%s: ", dateString);
	}
	
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	
	printf("\n");
}
