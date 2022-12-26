#pragma once

#ifndef ONEDAL_DATA_PARALLEL
#define ONEDAL_DATA_PARALLEL
#endif

#include <vector>
#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <filesystem>

#include <sycl/sycl.hpp>

#include "oneapi/dal.hpp"

class OneDALManager
{
public:
    OneDALManager();
    ~OneDALManager();

    void Run();

private:
    // Get dal::table from .csv file
    oneapi::dal::table GetTableFromFile(const std::string& name);

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
            m_devices.push_back(sycl::ext::oneapi::detail::select_device(selector));
        }
        catch (...) {
            return;
        }
    };

private:
    std::vector<sycl::device> m_devices;
    std::vector<sycl::queue> m_queues;
    uint64_t m_selectedDevice;
};

