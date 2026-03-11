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
            m.queues.push_back(sycl::queue{ sycl::ext::oneapi::detail::select_device(selector), AsyncHandler });
        }
        catch (...) {
            return;
        }
    };
};

