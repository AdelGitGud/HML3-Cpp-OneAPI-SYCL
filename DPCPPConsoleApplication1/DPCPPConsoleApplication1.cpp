#include "OneAPIManager.h"
#include "RenderManager.h"

#include <thread>

int main(int argc, char* argv[]) {
    OneAPIManager test;
    RenderManager rendererTest;
    
    if (rendererTest.Init()) {
        rendererTest.OpenWindow(L"Test window", 800, 600);
    }
	bool runGUI = true;
    std::thread testThread([&]{
		while (runGUI) {
			rendererTest.Run();
		}
    });

    if (test.Init()) {
        test.Run();
    }
    
    rendererTest.Shutdown();
    testThread.join();

    return 0;
}
