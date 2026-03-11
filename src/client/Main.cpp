#include "OneAPP.h"

int main(int argc, char* argv[]) {
    OneAPP test;

    if (test.Init()) {
        test.Run();
        test.Shutdown();
    }

    return 0;
}