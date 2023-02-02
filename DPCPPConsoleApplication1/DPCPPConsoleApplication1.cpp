#include "OneAPP.h"
#include "RenderManager.h"

#include <thread>

int main(int argc, char* argv[]) {
    OneAPP test;
    RenderManager rendererTest;
    
    if (rendererTest.Init()) {
        rendererTest.OpenWindow(L"Test window", 800, 600);
    }
	bool runGUI = true;
    std::thread testThread([&]{
		while (rendererTest.Run()) {
		}
    });

    if (test.Init()) {
        test.Run();
        test.Shutdown();
    }
    
	rendererTest.CloseWindow();
    testThread.join();

    return 0;
}
