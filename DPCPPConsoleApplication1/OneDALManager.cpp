#include "OneDALManager.h"

#include <vector>
#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cassert>

namespace onedal = oneapi::dal;

// std::out overload
std::ostream& operator<<(std::ostream& stream, const onedal::table& table) {
    onedal::array arr = onedal::row_accessor<const float>(table).pull();
    const float* x = arr.get_data();

    if (table.get_row_count() <= 10) {
        for (std::int64_t i = 0; i < table.get_row_count(); i++) {
            for (std::int64_t j = 0; j < table.get_column_count(); j++) {
                std::cout << std::setw(10) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(3) << x[i * table.get_column_count() + j];
            }
            std::cout << std::endl;
        }
    }
    else {
        for (std::int64_t i = 0; i < 5; i++) {
            for (std::int64_t j = 0; j < table.get_column_count(); j++) {
                std::cout << std::setw(10) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(3) << x[i * table.get_column_count() + j];
            }
            std::cout << std::endl;
        }
        std::cout << "..." << (table.get_row_count() - 10) << " lines skipped..." << std::endl;
        for (std::int64_t i = table.get_row_count() - 5; i < table.get_row_count(); i++) {
            for (std::int64_t j = 0; j < table.get_column_count(); j++) {
                std::cout << std::setw(10) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(3) << x[i * table.get_column_count() + j];
            }
            std::cout << std::endl;
        }
    }
    return stream;
};

static auto ExceptionHandler = [](sycl::exception_list e_list) -> void {
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

OneDALManager::OneDALManager() {
    AddDevice(&sycl::gpu_selector_v);
    AddDevice(&sycl::cpu_selector_v);
    m.queues.reserve(m.devices.size());
}

OneDALManager::~OneDALManager() {
}

void OneDALManager::Run() {
    if (m.devices.empty()) {
        std::cout << "No compatible device found, exiting." << std::endl;
        return;
    }

    // List selectable devices
    std::cout << "Enter prefered device index:" << std::endl;
    for (sycl::device device : m.devices) {
        std::cout << '\t' << m.queues.size() << ") ";
        m.queues.push_back(sycl::queue{ device, ExceptionHandler });
        std::cout << m.queues.back().get_device().get_info<sycl::info::device::name>() << std::endl;
    }

start:
    // Select device
    if (!(std::cin >> m.selectedDevice)) {
        // Invalid input
        if (std::cin.eof()) {
            std::cout << "User aborted!" << std::endl;
            return;
        }
        std::cout << "Please enter a number!";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        goto start;
    }
    else if (m.selectedDevice >= m.devices.size()) {
        // User inputed a number out of range
        std::cout << "Device \"" << m.selectedDevice << "\" not found. Please enter a number between 0 and " << m.devices.size() - 1 << ":" << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        goto start;
    }
    else {
        // Device selected successfully
        std::cout << "Running on device:" << std::endl;
        std::cout << '\t' << m.queues[m.selectedDevice].get_device().get_info<sycl::info::device::name>() << std::endl;
    }

    // Load and prints selected data
    PrintDirectoryEntries("data");
    std::string tmp;
    std::cin >> tmp;
    if (std::cin.eof()) {
        std::cout << "User aborted!" << std::endl;
        return;
    }
    std::optional<onedal::table> data = GetTableFromFile(tmp);
    if (data.has_value()) {
        // Everything is good, proceeding
        PrintBasicTableDescriptor(data.value());
    }
    else {
        // User aborted
        return;
    }
}

std::optional<onedal::table> OneDALManager::GetTableFromFile(const std::string& name) {
    const std::string path = "data/";

    const std::string tryPath = path + name;
    if (CheckFile(tryPath)) {
        onedal::csv::data_source dataSource{tryPath};
        dataSource.set_delimiter(',');
        dataSource.set_parse_header(true);

        return onedal::read<onedal::table>(m.queues[m.selectedDevice], dataSource); // Throws exception in debug when running on the gpu. Doesnt seem to cause issue in current testing cases however.
    }

    std::cout << "File \"" << name << "\" not found. Please try again:" << std::endl;
    std::string tmp;
    std::cin >> tmp;
    if (std::cin.eof()) {
        std::cout << "User aborted!" << std::endl;
        return {};
    }
    return GetTableFromFile(tmp);
}

void OneDALManager::PrintBasicTableDescriptor(const onedal::table& table) {
    const onedal::basic_statistics::compute_result result = onedal::compute(m.queues[m.selectedDevice], onedal::basic_statistics::descriptor{}, table);

    std::cout << "Column count: "
        << table.get_column_count() << std::endl
        << "Row count : "
        << table.get_row_count() << std::endl;

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