#include "OneDALManager.h"

#include <vector>
#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cassert>

namespace onedal = oneapi::dal;

#define MAXPRINT    10
#define HALFPRINT   5
#define INCOMECAT   7
#define CATCUTOFF   6.0
#define CATBINSSTEP 1.5
typedef sycl::usm_allocator<float, sycl::usm::alloc::shared> TABLEALLOC;

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

OneDALManager::OneDALManager() {
    m.queues.reserve(2);
    AddDevice(&sycl::gpu_selector_v, exceptionHandler);
    AddDevice(&sycl::cpu_selector_v, exceptionHandler);
}

OneDALManager::~OneDALManager() {
}

void OneDALManager::Run() {
    if (m.queues.empty()) {
        std::cout << "No compatible device found, exiting." << std::endl;
        return;
    }

start:
    if (!ListAndSelectDevices()) { // User aborted
        return;
    }

    // Prints and loads selected data
    PrintDirectoryEntries("data");
    const std::optional<const onedal::table>& data = GetTableFromFile(GetUserStringInput());
    if (!data.has_value()) { // User aborted
        return;
    }
    PrintBasicTableDescriptor(data.value());

    // ------ WARNING: 69IQ CODING BELOW ------
    // Create allocator for device
    TABLEALLOC myAlloc(m.queues[m.selectedDevice]);

    // Create std vectors with the allocator and onedal array to play with data
    std::vector<float, TABLEALLOC> incomeCat(data.value().get_row_count(), myAlloc);
    onedal::array arr = onedal::row_accessor<const float>(data.value()).pull();

    // Get pointer to vector data for access in device and another for copying the housing income data
    float* devicePtr = incomeCat.data();
    const float* hostPtr = arr.get_data();

    // Copy income house data to income category array (in the dirtiest way possible)
    for (uint64_t i = 0; i < data.value().get_row_count(); i++) {
        if (hostPtr[i * data.value().get_column_count() + INCOMECAT] < CATCUTOFF) {
            incomeCat[i] = hostPtr[i * data.value().get_column_count() + INCOMECAT];
        } else {
            incomeCat[i] = CATCUTOFF;
        }
    }

    // Probably the most useless hardware accelleration ever
    m.queues[m.selectedDevice].submit([&](sycl::handler& h) {
        h.parallel_for(sycl::range<1>(data.value().get_row_count()),
        [=](sycl::id<1> idx) {
                devicePtr[idx] /= CATBINSSTEP;
            });
    }).wait();

    // Followed by this ugly adjustment because I have not learned how get the correct values from above without crashing yet...
    uint64_t cat1 = 0, cat2 = 0, cat3 = 0, cat4 = 0, cat5 = 0;
    for (uint64_t i = 0; i < data.value().get_row_count(); i++) {
        switch ((uint64_t)incomeCat[i]) {
        case 0:
            cat1++;
            break;
        case 1:
            cat2++;
            break;
        case 2:
            cat3++;
            break;
        case 3:
            cat4++;
            break;
        case 4:
            cat5++;
            break;
        }
    }
    std::cout << "Housing income categories:" << std::endl;
    std::cout << '\t' << "cat1" << '\t' << "cat2" << '\t' << "cat3" << '\t' << "cat4" << '\t' << "cat5" << std::endl;
    std::cout << '\t' << cat1 << '\t' << cat2 << '\t' << cat3 << '\t' << cat4 << '\t' << cat5 << std::endl;
    std::cout << std::fixed << std::setprecision(2)
        << '\t' <<(float)cat1 / data.value().get_row_count() * 100.0 << "%"
        << '\t' << (float)cat2 / data.value().get_row_count() * 100.0 << "%"
        << '\t' << (float)cat3 / data.value().get_row_count() * 100.0 << "%"
        << '\t' << (float)cat4 / data.value().get_row_count() * 100.0 << "%"
        << '\t' << (float)cat5 / data.value().get_row_count() * 100.0 << "%" << std::endl;
    // ------ Back 420IQ coding ------

    // Restart to device selection if user doesnt wish to exist
    char exitInput;
    std::cout << "Exit?" << "\t[Y]es/[N]o" << std::endl;
    while (std::cin.get(exitInput)) {
        if (std::cin.eof()) {
            std::cout << "User aborted!" << std::endl;
            return;
        } else if ((exitInput = tolower(exitInput)) == 'n') {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            goto start;
        } else if (exitInput == 'y') {
            return;
        }
    }
}

bool OneDALManager::ListAndSelectDevices() {
    // List selectable devices
    std::cout << "Enter prefered device index:" << std::endl;
    for (uint64_t i = 0; i < m.queues.size(); i++) {
        std::cout << '\t' << i << ") " << m.queues[i].get_device().get_info<sycl::info::device::name>() << std::endl;
    }

    // Select device
    if (!(std::cin >> m.selectedDevice)) {
        if (std::cin.eof()) {
            std::cout << "User aborted!" << std::endl;
            return false;
        }
        std::cout << "Please enter a number!" << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        return ListAndSelectDevices();
    } else if (m.selectedDevice >= m.queues.size()) {
        // User inputed a number out of range
        std::cout << "Device \"" << m.selectedDevice << "\" not found. Please enter a number between 0 and " << m.queues.size() - 1 << ":" << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        return ListAndSelectDevices();
    }
    // Device selected successfully, proceeding
    std::cout << "Running on device:" << std::endl;
    std::cout << '\t' << m.queues[m.selectedDevice].get_device().get_info<sycl::info::device::name>() << std::endl;
    return true;
}

const std::optional<const onedal::table> OneDALManager::GetTableFromFile(const std::string& name, const std::string& path) {
    std::string tryPath = path + name;
    if (CheckFile(tryPath + ".csv")) {
        tryPath += ".csv";
    } else if (CheckFile(tryPath)) {
    } else {
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
    return onedal::read<const onedal::table>(m.queues[m.selectedDevice], dataSource); // Throws exception in debug when running on the gpu. Doesnt seem to cause issue in current testing cases however.
}

void OneDALManager::PrintBasicTableDescriptor(const onedal::table& table) {
    const onedal::basic_statistics::compute_result result = onedal::compute(m.queues[m.selectedDevice], onedal::basic_statistics::descriptor{}, table);

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