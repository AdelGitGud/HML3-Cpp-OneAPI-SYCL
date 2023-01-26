#include "OneAPIManager.h"
#include "RenderManager.h"

#include <thread>

int main(int argc, char* argv[]) {
    OneAPIManager test;
    
    RenderManager renderer;
	renderer.Init();
    renderer.OpenWindow(L"Test window", 800, 600);

    std::thread testThread([&]{
        while(renderer.Run());
    });

    if (test.Init()) {
        test.Run();
    }
    
    testThread.join();
    renderer.Shutdown();

    return 0;
}
