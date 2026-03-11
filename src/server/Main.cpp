#include "OneAPP.h"

int main(int argc, char* argv[]) {
    OneAPP test;

    if (test.Init()) {
        test.Run();
    }

    return 0;
}