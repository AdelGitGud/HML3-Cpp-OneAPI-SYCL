#ifndef ONEDAL_DATA_PARALLEL
#define ONEDAL_DATA_PARALLEL
#endif

#include <vector>
#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cassert>

#include <sycl/sycl.hpp>

#include "oneapi/dal.hpp"

// std::out overload
std::ostream& operator<<(std::ostream& stream, const oneapi::dal::table& table) {
    auto arr = oneapi::dal::row_accessor<const float>(table).pull();
    const auto x = arr.get_data();

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
}

inline bool CheckFile(const std::string& name) {
    return std::ifstream{ name }.good();
}

// Prints basic informations about a given dal::table
inline void PrintBasicTableDescriptor(sycl::queue& q, const oneapi::dal::table& table) {
    const auto result = oneapi::dal::compute(q, oneapi::dal::basic_statistics::descriptor{}, table);

    std::cout << "housing.csv column count: "
        << table.get_column_count() << std::endl
        << "housing.csv row count : "
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

// Create an exception handler for async SYCL exceptions
static auto ExceptionHandler = [](sycl::exception_list e_list) {
    for (std::exception_ptr const& e : e_list) {
        try {
            std::rethrow_exception(e);
        }
        catch (std::exception const& e) {
#if _DEBUG
            std::cout << "Failure" << std::endl;
#endif
            std::terminate();
        }
    }
};

// Get dal::table from .csv file
const auto GetTableFromCSV = [](sycl::queue& q, const std::string& name) {
    const std::string path = "data/";
    const std::string ext = ".csv";

    const std::string tryPath = path + name + ext;
    if (CheckFile(tryPath)) {
        auto dataSource = oneapi::dal::csv::data_source(tryPath);
        dataSource.set_delimiter(',');
        dataSource.set_parse_header(true);

        return oneapi::dal::read<oneapi::dal::table>(q, dataSource);
    }

    // Terminate if file not found
    std::cout << "File " << name << ".csv not found" << std::endl;
    std::terminate();
};

// Real main tbh
void Run(sycl::queue& q) {
    std::cout << "Running on device: "
        << q.get_device().get_info<sycl::info::device::name>() << std::endl;

    const auto housing = GetTableFromCSV(q, "housing");
    
    PrintBasicTableDescriptor(q, housing);

    return;
}

int main(int argc, char* argv[]) {
    sycl::queue q(sycl::default_selector_v, ExceptionHandler);

    Run(q);

    return 0;
}