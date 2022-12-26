#include "OneDALManager.h"
#include <cassert>

// std::out overload
std::ostream& operator<<(std::ostream& stream, const oneapi::dal::table& table) {
    oneapi::dal::array arr = oneapi::dal::row_accessor<const float>(table).pull();
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

static auto ExceptionHandler = [](sycl::exception_list e_list) {
    for (std::exception_ptr const& e : e_list) {
        try {
            std::rethrow_exception(e);
        }
        catch (std::exception const& e) {
            std::cout << "Failure" << std::endl;
            std::terminate();
        }
    }
};

OneDALManager::OneDALManager() {
    AddDevice(&sycl::gpu_selector_v);
    AddDevice(&sycl::cpu_selector_v);

    if (m_devices.empty()) {
        std::cout << "No compatible device found, exiting." << std::endl;
        std::terminate();
    }

    std::cout << "Select device:" << std::endl;
    for (sycl::device device : m_devices) {
        std::cout << '\t' << m_queues.size() << ") ";
        m_queues.push_back(sycl::queue{ device, ExceptionHandler});
        std::cout << m_queues.back().get_device().get_info<sycl::info::device::name>() << std::endl;
    }
}

OneDALManager::~OneDALManager() {
}

void OneDALManager::Run() {
    // Select device
    std::cin >> m_selectedDevice;
    std::cout << "Running on:" << std::endl;
    std::cout << '\t' << m_queues[m_selectedDevice].get_device().get_info<sycl::info::device::name>() << std::endl;

    // Load and prints selected data
    PrintDirectoryEntries("data");
    std::string tmp;
    std::cin >> tmp;
    oneapi::dal::table data = GetTableFromCSV(tmp);
    PrintBasicTableDescriptor(data);

    return;
}

oneapi::dal::table OneDALManager::GetTableFromCSV(const std::string& name) {
    const std::string path = "data/";

    const std::string tryPath = path + name;
    if (CheckFile(tryPath)) {
        oneapi::dal::csv::data_source dataSource{tryPath};
        dataSource.set_delimiter(',');
        dataSource.set_parse_header(true);

        return oneapi::dal::read<oneapi::dal::table>(m_queues[m_selectedDevice], dataSource); // Throws exception in debug when running on the gpu. Doesnt seem to cause issue in current testing cases however.
    }

    std::cout << "File \"" << name << "\" not found. Please try again:" << std::endl;
    std::string tmp;
    std::cin >> tmp;
    return GetTableFromCSV(tmp);
}

void OneDALManager::PrintBasicTableDescriptor(const oneapi::dal::table& table) {
    const oneapi::dal::basic_statistics::compute_result result = oneapi::dal::compute(m_queues[m_selectedDevice], oneapi::dal::basic_statistics::descriptor{}, table);

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
void StratifiedShuffledSplit(const oneapi::dal::table& table, float ratio, int seed,
    oneapi::dal::table& first, oneapi::dal::table& second) {
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
    first = oneapi::dal::table(num_rows);
    second = oneapi::dal::table(num_rows);

    // Shuffle the rows
    std::mt19937 rng(seed);
    std::shuffle(labels.begin(), labels.end(), rng);

    // Split the rows into two tables based on the ratio
    int split_index = static_cast<int>(num_rows * ratio);
    for (int i = 0; i < num_rows; i++) {
        oneapi::dal::table& split = (i < split_index) ? first : second;
        //split.set_column("label", labels[i]);
        // Set other columns as needed
    }
} */