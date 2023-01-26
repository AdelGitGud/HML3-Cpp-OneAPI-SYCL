#include "OneAPIManager.h"

int main(int argc, char* argv[]) {
    OneAPIManager test;

    if (test.Init()) {
        test.Run();
    }

    return 0;
}
