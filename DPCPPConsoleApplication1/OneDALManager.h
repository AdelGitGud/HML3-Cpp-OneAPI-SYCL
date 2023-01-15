#pragma once

#ifndef ONEDAL_DATA_PARALLEL
#define ONEDAL_DATA_PARALLEL
#endif

#include "oneapi/dal.hpp"
#include <sycl/sycl.hpp>

#include <filesystem>

#include "HOMLData.h"

class OneDALManager
{
public:
    OneDALManager();
    ~OneDALManager();

    void Run();

private:
    const std::optional<const oneapi::dal::table> GetTableFromFile(const std::string& name, const std::string& path = "data/");
    void PrintBasicTableDescriptor(const oneapi::dal::table& table);

    bool SelectAmongNumOptions(uint64_t& selector, const uint64_t& selectionSize, const std::string& name = "Option");
    bool ListAndSelectDevices();
    bool ListAndSelectTasks(const std::optional<const oneapi::dal::v1::table>& data);

    void TestFunction(const std::optional<const oneapi::dal::v1::table>& data);

    inline void AddDevice(int (*selector)(const sycl::device&), const std::function<void(sycl::exception_list)>& AsyncHandler) {
        try {
            m.queues.push_back(sycl::queue{ sycl::ext::oneapi::detail::select_device(selector), AsyncHandler });
        } catch (...) {
            return;
        }
    };

    inline bool CheckFile(const std::string& name) {
        return std::ifstream{ name }.good();
    };

    inline void PrintDirectoryEntries(const std::string& dir) {
        std::cout << "Select among available data: " << std::endl;
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(dir)) {
            std::cout << '\t' << entry.path().filename() << std::endl;
        }
    };

    inline const std::string GetUserStringInput() {
        std::string tmp;
        std::cin >> tmp;
        if (std::cin.eof()) {
            std::cout << "User aborted!" << std::endl;
            return {};
        } else {
            return tmp;
        }
    };

private:
    struct Members {
        std::vector<sycl::queue> queues = {};
        uint64_t selectedDevice = 0;

        const std::string tasks[2] = {"NONE", "EXP"};
    }m;
};

