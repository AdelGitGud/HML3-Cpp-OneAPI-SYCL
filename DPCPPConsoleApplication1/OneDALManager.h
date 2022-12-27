#pragma once

#ifndef ONEDAL_DATA_PARALLEL
#define ONEDAL_DATA_PARALLEL
#endif

#include "oneapi/dal.hpp"
#include <sycl/sycl.hpp>

#include <filesystem>

class OneDALManager
{
public:
    OneDALManager();
    ~OneDALManager();

    void Run();

private:
    // Get dal::table from .csv file
    std::optional<oneapi::dal::table> GetTableFromFile(const std::string& name);

    // Prints basic informations about a given dal::table
    void PrintBasicTableDescriptor(const oneapi::dal::table& table);

    inline bool CheckFile(const std::string& name) {
        return std::ifstream{ name }.good();
    };

    inline void PrintDirectoryEntries(const std::string& dir) {
        std::cout << "Select among available data: " << std::endl;
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(dir)) {
            std::cout << '\t' << entry.path().filename() << std::endl;
        }
    };

    inline void AddDevice(int (*selector)(const sycl::device&)) {
        try {
            m.devices.push_back(sycl::ext::oneapi::detail::select_device(selector));
        }
        catch (...) {
            return;
        }
    };

private:
    struct M {
        std::vector<sycl::device> devices = {};
        std::vector<sycl::queue> queues = {};
        uint64_t selectedDevice = 0;
    }m;
};

