#include "OneAPIManager.h"

#include <vector>
#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cassert>

#include "HOMLData.h"
#include "DPData.h"

namespace onedal = oneapi::dal;

#define MAXPRINT    10
#define HALFPRINT   5
#define NBROFCAT    10
#define INCOMECAT   7
#define CATCUTOFF   6.0f
#define CATBINSSTEP 1.5f

// std::out overload
std::ostream& operator<<(std::ostream& stream, const onedal::table& table) {
    onedal::array arr = onedal::row_accessor<const float>(table).pull();
    const float* x = arr.get_data();

    if (table.get_row_count() <= MAXPRINT) {
        for (uint64_t i = 0; i < table.get_row_count(); i++) {
            for (uint64_t j = 0; j < table.get_column_count(); j++) {
                std::cout << std::setw(10) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(3) << x[i * table.get_column_count() + j];
            }
            std::cout << std::endl;
        }
    } else {
        for (uint64_t i = 0; i < HALFPRINT; i++) {
            for (uint64_t j = 0; j < table.get_column_count(); j++) {
                std::cout << std::setw(10) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(3) << x[i * table.get_column_count() + j];
            }
            std::cout << std::endl;
        }
        std::cout << "..." << (table.get_row_count() - MAXPRINT) << " lines skipped..." << std::endl;
        for (uint64_t i = table.get_row_count() - HALFPRINT; i < table.get_row_count(); i++) {
            for (uint64_t j = 0; j < table.get_column_count(); j++) {
                std::cout << std::setw(10) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(3) << x[i * table.get_column_count() + j];
            }
            std::cout << std::endl;
        }
    }
    return stream;
};

static auto exceptionHandler = [](sycl::exception_list e_list)->void {
    for (std::exception_ptr const& e : e_list) {
        try {
            std::rethrow_exception(e);
        } catch (std::exception const& e) {
            std::cout << "!!!Failure!!!" << std::endl;
            std::terminate();
        }
    }
};

OneAPIManager::OneAPIManager() {
    m.queues.reserve(2);
    AddDevice(&sycl::cpu_selector_v, exceptionHandler);
    AddDevice(&sycl::gpu_selector_v, exceptionHandler);
}

OneAPIManager::~OneAPIManager() {
}

bool OneAPIManager::Init() {
    if (m.queues.empty()) {
        std::cout << "No compatible device found, exiting." << std::endl;
        return false;
    }
    return true;
}

void OneAPIManager::Run() {
    if (!ListAndSelectDevices()) { // User aborted
        return;
    }

    if (!ListAndRunTasks()) {// User aborted
        return;
    }

    // Restart to device selection if user doesnt wish to exist
    char exitInput;
    std::cout << "Exit?" << "\t[Y]es/[N]o" << std::endl;
    while (std::cin.get(exitInput)) {
        if (std::cin.eof()) {
            std::cout << "User aborted!" << std::endl;
            return;
        }
        else if ((exitInput = tolower(exitInput)) == 'n') {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            Run();
        }
        else if (exitInput == 'y') {
            return;
        }
    }
}

const std::optional<const onedal::table> OneAPIManager::GetTableFromFile(const std::string& name, const std::string& path) {
    std::string tryPath = path + name;
    if (CheckFile(tryPath + ".csv")) {
        tryPath += ".csv";
    }
    else if (CheckFile(tryPath)) {
    }
    else {
        std::cout << "File \"" << name << "\" not found. Please try again:" << std::endl;
        const std::string& userInput = GetUserStringInput();
        if (userInput.empty()) { // User aborted
            return {};
        }
        return GetTableFromFile(userInput);
    }
    onedal::csv::data_source dataSource{ tryPath };
    dataSource.set_delimiter(',');
    dataSource.set_parse_header(true);
    return onedal::read<const onedal::table>(m.queues[m.primaryDevice], dataSource); // Throws exception in debug when running on the gpu. Doesnt seem to cause issue in current testing cases however.
}

void OneAPIManager::PrintBasicTableDescriptor(const onedal::table& table) {
    const onedal::basic_statistics::compute_result result = onedal::compute(m.queues[m.primaryDevice], onedal::basic_statistics::descriptor{}, table);

    std::cout << "Column count: " << table.get_column_count() << std::endl;
    std::cout << "Row count : " << table.get_row_count() << std::endl;

    std::cout << "Minimum:\n" << result.get_min() << std::endl;
    std::cout << "Maximum:\n" << result.get_max() << std::endl;
    std::cout << "Sum:\n" << result.get_sum() << std::endl;
    std::cout << "Sum of squares:\n" << result.get_sum_squares() << std::endl;
    std::cout << "Sum of squared difference from the means:\n"
        << result.get_sum_squares_centered() << std::endl;
    std::cout << "Mean:\n" << result.get_mean() << std::endl;
    std::cout << "Second order raw moment:\n" << result.get_second_order_raw_moment() << std::endl;
    std::cout << "Variance:\n" << result.get_variance() << std::endl;
    std::cout << "Standard deviation:\n" << result.get_standard_deviation() << std::endl;
    std::cout << "Variation:\n" << result.get_variation() << std::endl;
}

bool OneAPIManager::SelectAmongNumOptions(uint64_t& selector, const uint64_t& selectionSize, const std::string& name)
{
    if (!(std::cin >> selector)) {
        if (std::cin.eof()) {
            std::cout << "User aborted!" << std::endl;
            return false;
        }
        std::cout << "Please enter a number!" << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        return SelectAmongNumOptions(selector, selectionSize);
    }
    else if (selector >= selectionSize) {
        // User inputed a number out of range
        std::cout << name << "\"" << selector << "\" not found. Please enter a number between 0 and " << selectionSize - 1 << ":" << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        return SelectAmongNumOptions(selector, selectionSize);
    }
    return true;
}

bool OneAPIManager::ListAndSelectDevices() {
    // List selectable devices
    std::cout << "Enter prefered device index:" << std::endl;
    for (uint64_t i = 0; i < m.queues.size(); i++) {
        std::cout << '\t' << i << ") " << m.queues[i].get_device().get_info<sycl::info::device::name>() << std::endl;
    }

    if (!SelectAmongNumOptions(m.primaryDevice, m.queues.size(), "Device")) { // User aborted
        return false;
    }

    // Device selected successfully, proceeding
    std::cout << "Running on device:" << std::endl;
    std::cout << '\t' << m.queues[m.primaryDevice].get_device().get_info<sycl::info::device::name>() << std::endl;
    return true;
}

bool OneAPIManager::ListAndRunTasks() {
    const uint64_t tasksSize = sizeof(m.tasks) / sizeof(m.tasks[0]);
    uint64_t selectedTask;
    std::cout << "Select among available tasks:" << std::endl;
    for (selectedTask = 0; selectedTask < tasksSize; selectedTask++) {
        std::cout << '\t' << selectedTask << ") " << m.tasks[selectedTask] << std::endl;
    }

    if (!SelectAmongNumOptions(selectedTask, tasksSize, "Task")) { // User aborted
        return false;
    }

    switch (selectedTask) {
    case 1:
        return HOMLTesting();
    case 2:
        return SYCLTesting();
    case 3:
        return SYCLHelloWorld();
    default:
        return true; // NONE option successfully selected
    }
}

// ------ EXPERIMENTAL: Dont understand what I'm doing ------
bool OneAPIManager::HOMLTesting() {
    // Prints and loads selected data
    PrintDirectoryEntries("data");
    const std::optional<const onedal::table>& data = GetTableFromFile(GetUserStringInput());
    if (!data.has_value()) { // User aborted
        return false;
    }
    PrintBasicTableDescriptor(data.value());

    // Create allocator for device
    sycl::usm_allocator<uint64_t, sycl::usm::alloc::shared> myAlloc(m.queues[m.primaryDevice]);

    // Create std vectors with the allocator and onedal array to play with data
    std::vector<uint64_t, sycl::usm_allocator<uint64_t, sycl::usm::alloc::shared>> incomeSplit(data.value().get_row_count(), myAlloc);
    onedal::array<float> mutArray = onedal::row_accessor<const float>(data.value()).pull();

    std::cout << mutArray.has_mutable_data() << std::endl;

    mutArray.need_mutable_data();

    std::cout << mutArray.has_mutable_data() << std::endl;

    // Haha data go brrr
    m.queues[m.primaryDevice].submit([&](sycl::handler& h) {
        uint64_t* incomeSplitPtr = incomeSplit.data();
    const float* rawIncomePtr = mutArray.get_mutable_data();
    h.parallel_for(sycl::range<1>(data.value().get_row_count()), [=](sycl::id<1> idx) {
        incomeSplitPtr[idx] = rawIncomePtr[idx * NBROFCAT + INCOMECAT] / CATBINSSTEP;
        });
    }).wait();

    // Count and print income category split
    uint64_t cat[5] = { 0 };
    for (uint64_t i = 0; i < data.value().get_row_count(); i++) {
        switch (incomeSplit[i]) {
        case 0:
            cat[incomeSplit[i]]++;
            break;
        case 1:
            cat[incomeSplit[i]]++;
            break;
        case 2:
            cat[incomeSplit[i]]++;
            break;
        case 3:
            cat[incomeSplit[i]]++;
            break;
        default:
            cat[4]++;
            break;
        }
    }
    std::cout << "Housing income split:" << std::endl;
    std::cout << '\t' << "cat0" << '\t' << "cat1" << '\t' << "cat2" << '\t' << "cat3" << '\t' << "cat4" << std::endl;
    for (uint64_t i = 0; i < 5; i++) {
        std::cout << '\t' << cat[i];
    }
    std::cout << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    for (uint64_t i = 0; i < 5; i++) {
        std::cout << '\t' << (float)cat[i] / data.value().get_row_count() * 100.0 << "%";
    }
    std::cout << std::endl;
    return true;
}

bool OneAPIManager::SYCLTesting() {
    return true;
}

bool OneAPIManager::SYCLHelloWorld() {
    const DPHelloWorld data;
    char* result = sycl::malloc_shared<char>(data.sz, m.queues[m.primaryDevice]);
    std::memcpy(result, data.secret.data(), data.sz);

    m.queues[m.primaryDevice].parallel_for(data.sz, [=](auto& i) {
        result[i] -= 1;
    }).wait();

    std::cout << result << std::endl;
    free(result, m.queues[m.primaryDevice]);
    return true;
}

// Fake Code still working on this
/*template <typename T>
void Cut() {

}

template <typename T>
void StratifiedShuffledSplit(const onedal::table& table, float ratio, int seed,
    onedal::table& first, onedal::table& second) {
    // Get the number of rows in the table
    const int num_rows = table.get_row_count();

    // Get the class labels and the number of unique labels
    std::vector<T> labels;
    labels.reserve(num_rows);
    for (int i = 0; i < num_rows; i++) {
        //labels.push_back(table.get_column<T>("label")[i]);
    }
    const int num_classes = static_cast<int>(std::set<T>(labels.begin(), labels.end()).size());

    // Initialize the output tables
    first = onedal::table(num_rows);
    second = onedal::table(num_rows);

    // Shuffle the rows
    std::mt19937 rng(seed);
    std::shuffle(labels.begin(), labels.end(), rng);

    // Split the rows into two tables based on the ratio
    int split_index = static_cast<int>(num_rows * ratio);
    for (int i = 0; i < num_rows; i++) {
        onedal::table& split = (i < split_index) ? first : second;
        //split.set_column("label", labels[i]);
        // Set other columns as needed
    }
} */