#include "Platform.h"

#include <iostream>

#ifdef __linux    
#include <unistd.h>
#endif

void MsgBox(const char* s) {
#ifdef _WIN32
    MessageBoxA(NULL, s, "Vulkan Error!", 0);
#elif defined __linux
    char cmd[1024];
    sprintf(cmd, "xmessage -center \"%s\"", s);
    if (fork() == 0) {
        close(1); close(2);
        system(cmd);
        exit(0);
    }
#endif
}
