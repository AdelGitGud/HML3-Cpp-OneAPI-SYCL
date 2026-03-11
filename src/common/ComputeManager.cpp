#include "ComputeManager.h"
#include <iostream>

static auto exceptionHandler = [](sycl::exception_list e_list)->void {
    for (std::exception_ptr const& e : e_list) {
        try {
            std::rethrow_exception(e);
        }
        catch (std::exception const& e) {
            std::cout << "!!!Failure!!!" << std::endl;
            std::terminate();
        }
    }
};

ComputeManager::ComputeManager() {
    m.queues.reserve(2);
    AddDevice(&sycl::cpu_selector_v, exceptionHandler);
    AddDevice(&sycl::gpu_selector_v, exceptionHandler);
}

ComputeManager::~ComputeManager() {
}

bool ComputeManager::Init() {
    if (m.queues.empty()) {
        std::cout << "No compatible device found, exiting." << std::endl;
        return false;
    }
    return true;
}

void ComputeManager::Shutdown() {
}
