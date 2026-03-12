#pragma once
#include "BaseManager.h"

#include "OneMath.h"

class ComputeManager : public BaseManager {
public:
	ComputeManager();
	~ComputeManager() override;
    bool Init() override;
    void Shutdown() override;

private:

private:
    struct Members {
        std::vector<sycl::queue> queues = {};
        uint64_t primaryDevice = 0;
    }m;
    
public:
	inline sycl::queue& GetPrimaryQueue() { return m.queues[m.primaryDevice]; };
	inline sycl::queue& GetQueue(uint64_t& index) { return m.queues[index]; };
	inline size_t GetQueueCount() { return m.queues.size(); };
	inline uint64_t& GetPrimaryDevice() { return m.primaryDevice; };

private:
    inline void AddDevice(int (*selector)(const sycl::device&), const std::function<void(sycl::exception_list)>& AsyncHandler) {
        try {
            sycl::queue queue(selector, AsyncHandler);
            const sycl::device device = queue.get_device();

            std::cout << "[ComputeManager] Added"
                    << ": " << device.get_info<sycl::info::device::name>()
                    << '\n';
            m.queues.push_back(sycl::queue{ sycl::ext::oneapi::detail::select_device(selector), AsyncHandler });
        }
        catch (const sycl::exception& e) {
            std::cerr << "[ComputeManager] Skipped"
                    << ": " << e.what() << '\n';
        }
        catch (const std::exception& e) {
            std::cerr << "[ComputeManager] Skipped"
                    << ": " << e.what() << '\n';
        }
        catch (...) {
            std::cerr << "[ComputeManager] Skipped"
                    << ": unknown exception\n";
        }
    };
};

