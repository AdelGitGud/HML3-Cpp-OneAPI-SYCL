#pragma once

#ifndef ONEDAL_DATA_PARALLEL
#define ONEDAL_DATA_PARALLEL
#endif

#include <filesystem>

#include "oneapi/dal.hpp"

#include "BaseManager.h"

class OneAPP : public BaseManager {
public:
    OneAPP();
    ~OneAPP() override;

	bool Init() override;
	void Shutdown() override;
    void Run();

private:
    size_t PrintDirectoryEntries(const std::string& dir, std::string& lastEntry);
    std::optional<const oneapi::dal::table> GetTableFromFile(const std::string& name, const std::string& path = "data/"); // Embarrassing test function
    void PrintBasicTableDescriptor(const oneapi::dal::table& table);

    bool SelectAmongNumOptions(uint64_t& selector, const uint64_t& selectionSize, const std::string& name = "Option");
    bool ListAndSelectDevices();
    bool ListAndRunTasks();

    bool HOMLTesting();
    bool SYCLTesting();
    bool SYCLHelloWorld();
    bool SYCLCount();

private:
    struct Members {
		class LogManager* logManager = nullptr;
		class ComputeManager* computeManager = nullptr;

        const std::string tasks[5] = {"NONE",

            "SYCL Hello World",
            "SYCL Learn to count",

            "HOML Testing (EXP)",
            "SYCL Testing (EXP)"
        };
    }m;

public:

private:
    inline bool CheckFile(const std::string& name) { return std::ifstream{ name }.good(); }

    inline const std::string GetUserStringInput() {
        std::string tmp;
        std::cin >> tmp;
        if (std::cin.eof()) {
            std::cout << "User aborted!" << std::endl;
            return {};
        }
        else {
            return tmp;
        }
    }
};

