#include "OneAPP.h"

#include <vector>
#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cassert>

#include "HOMLData.h"
#include "DPData.h"

namespace onedal = oneapi::dal;

enum TASKS {
    NONE,

    SYCLHW,
    SYCLCOUNT,

    HOMLEXP,
    SYCLEXP
};

// std::out overload
std::ostream& operator<<(std::ostream& stream, const onedal::table& table) {
    constexpr uint64_t  MAXPRINT    = 10;
    constexpr uint64_t  HALFPRINT   = 5;

    onedal::array arr = onedal::row_accessor<const float>(table).pull();
    const float* x = arr.get_data();

    if (table.get_row_count() <= MAXPRINT) {
        for (size_t i = 0; i < table.get_row_count(); i++) {
            for (size_t j = 0; j < table.get_column_count(); j++) {
                std::cout << std::setw(10) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(3) << x[i * table.get_column_count() + j];
            }
            std::cout << std::endl;
        }
    } else {
        for (size_t i = 0; i < HALFPRINT; i++) {
            for (size_t j = 0; j < table.get_column_count(); j++) {
                std::cout << std::setw(10) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(3) << x[i * table.get_column_count() + j];
            }
            std::cout << std::endl;
        }
        std::cout << "..." << (table.get_row_count() - MAXPRINT) << " lines skipped..." << std::endl;
        for (size_t i = table.get_row_count() - HALFPRINT; i < table.get_row_count(); i++) {
            for (size_t j = 0; j < table.get_column_count(); j++) {
                std::cout << std::setw(10) << std::setiosflags(std::ios::fixed)
                    << std::setprecision(3) << x[i * table.get_column_count() + j];
            }
            std::cout << std::endl;
        }
    }
    return stream;
};

OneAPP::OneAPP() {
    m.computeManager = new ComputeManager;
}

OneAPP::~OneAPP() {
    delete m.computeManager;
}

bool OneAPP::Init() {
    if (!m.computeManager->Init()) {
        return false;
    }
    return true;
}

void OneAPP::Shutdown() {
    m.computeManager->Shutdown();
}

void OneAPP::Run() {
    if (!ListAndSelectDevices()) { // User aborted
        return;
    }

    if (!ListAndRunTasks()) {// User aborted or error in selecting task
        return;
    }

    // Restart to device selection if user doesn't wish to exist
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
            return;
        }
        else if (exitInput == 'y') {
            return;
        }
    }
}

size_t OneAPP::PrintDirectoryEntries(const std::string& dir, std::string& lastEntry)
{
    std::cout << "Select among available data: " << std::endl;
    size_t i = 0;
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(dir)) {
        ++i;
        lastEntry = entry.path().filename().string();
        std::cout << '\t' << lastEntry << std::endl;
    }
    return i;
}

std::optional<const onedal::table> OneAPP::GetTableFromFile(const std::string& name, const std::string& path) {
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
    return onedal::read<const onedal::table>(m.computeManager->GetPrimaryQueue(), dataSource); // Throws exception in debug when running on the gpu. Doesn't seem to cause issue in current testing cases however.
}

void OneAPP::PrintBasicTableDescriptor(const oneapi::dal::table& table) {
    const onedal::basic_statistics::compute_result result = onedal::compute(m.computeManager->GetPrimaryQueue(), onedal::basic_statistics::descriptor{}, table);

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

bool OneAPP::SelectAmongNumOptions(uint64_t& selector, const uint64_t& selectionSize, const std::string& name) {
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
        // User inputted a number out of range
        std::cout << name << "\"" << selector << "\" not found. Please enter a number between 0 and " << selectionSize - 1 << ":" << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        return SelectAmongNumOptions(selector, selectionSize);
    }
    return true;
}

bool OneAPP::ListAndSelectDevices() {
    std::cout << "Enter preferred device index:" << std::endl;

    for (size_t i = 0; i < m.computeManager->GetQueueCount(); i++) {
        std::cout << '\t' << i << ") " << m.computeManager->GetQueue(i).get_device().get_info<sycl::info::device::name>() << std::endl;
    }

    if (!SelectAmongNumOptions(m.computeManager->GetPrimaryDevice(), m.computeManager->GetQueueCount(), "Device")) { // User aborted
        return false;
    }

    // Device selected successfully, proceeding
    std::cout << "Running on device:" << std::endl;
    std::cout << '\t' << m.computeManager->GetPrimaryQueue().get_device().get_info<sycl::info::device::name>() << std::endl;
    return true;
}

bool OneAPP::ListAndRunTasks() {
    constexpr uint64_t  TASKSCOUNT   = sizeof(m.tasks) / sizeof(m.tasks[0]);

    uint64_t selectedTask;

    std::cout << "Select among available tasks:" << std::endl;
    for (selectedTask = 0; selectedTask < TASKSCOUNT; selectedTask++) {
        std::cout << '\t' << selectedTask << ") " << m.tasks[selectedTask] << std::endl;
    }

    if (!SelectAmongNumOptions(selectedTask, TASKSCOUNT, "Task")) { // User aborted
        return false;
    }

    switch (selectedTask) {
    case NONE:
        std::cout << "No task selected." << std::endl;
        return true; // No task selected
        
    case SYCLHW:
        return SYCLHelloWorld();
    case SYCLCOUNT:
        return SYCLCount();
        
    case HOMLEXP:
        return HOMLTesting();
    case SYCLEXP:
        return SYCLTesting();
    default:
        std::cout << "Error task \"" << m.tasks[selectedTask] << "\" is listed but not implemented!!!" << std::endl;
        return false; 
    }
}

// ------ EXPERIMENTAL ------
bool OneAPP::HOMLTesting() {
	/*constexpr size_t	TESTSIZE = 20640;
    constexpr uint64_t  NBROFCAT        = 10;
    constexpr uint64_t  INCOMESPLITS    = 5;
    constexpr uint64_t  INCOMECAT       = 7;
    constexpr float     CATBINSSTEP     = 1.5f;
    constexpr float     SPLITSCUTOFF    = 6.f;*/

    std::cout << "Running task: " << m.tasks[HOMLEXP] << '.' << std::endl;

    // Prints and loads selected data
    std::string selectedData = {};
    if (PrintDirectoryEntries("data", selectedData) > 1) {
		selectedData = GetUserStringInput();
    }
    const std::optional<const onedal::table> data = GetTableFromFile(selectedData);
    if (!data.has_value()) { // User aborted
        return false;
    }
	PrintBasicTableDescriptor(data.value());
    
    return true;
}

// ------ EXPERIMENTAL ------
bool OneAPP::SYCLTesting() {
    constexpr size_t N = 69;

    std::cout << "Running task: " << m.tasks[SYCLEXP] << '.' << std::endl;

    std::array<uint64_t, N> data = { 0 };

    {
        sycl::buffer buffer(data);

        m.computeManager->GetPrimaryQueue().submit([&](sycl::handler& h) {
            sycl::accessor access(buffer, h);

            h.parallel_for(N, [=](sycl::id<1> i) {
                access[i] += i + 1;
            });
        });

        sycl::host_accessor hostAccess(buffer);

        for (size_t i = 0; i < N; i++) {
            std::cout << hostAccess[i] << " ";
        }
        std::cout << "\n";
    }

    // myData is updated when myBuffer is
    // destroyed upon exiting scope
    for (size_t i = 0; i < N; i++) {
        std::cout << data[i] << " ";
    }
    std::cout << std::endl;
    return true;
}

bool OneAPP::SYCLHelloWorld() {
    std::cout << "Running task: " << m.tasks[SYCLHW] << '.' << std::endl;

    const DPHelloWorld data;
    char* result = sycl::malloc_shared<char>(data.sz, m.computeManager->GetPrimaryQueue());

    std::memcpy(result, data.secret.data(), data.sz);

    m.computeManager->GetPrimaryQueue().submit([&](sycl::handler& h) {
        h.parallel_for(data.sz, [=](auto& i) {
            result[i] -= 1;
        });
    }).wait();

    std::cout << result << std::endl;
    free(result, m.computeManager->GetPrimaryQueue());
    return true;
}

bool OneAPP::SYCLCount() {
    constexpr size_t  SIZE    = 16;

    std::cout << "Running task: " << m.tasks[SYCLCOUNT] << '.' << std::endl;

    std::array<uint64_t, SIZE> data;

    // Create buffer using host allocated "data" array
    sycl::buffer B{ data };

    m.computeManager->GetPrimaryQueue().submit([&](sycl::handler& h) {
        sycl::accessor A{ B, h };
        h.parallel_for(SIZE, [=](auto& idx) {
                A[idx] = idx;
        });
    });

    // Obtain access to buffer on the host
    // Will wait for device kernel to execute to generate data
    sycl::host_accessor A{ B };
    for (size_t i = 0; i < SIZE; i++) {
        std::cout << "data[" << i << "] = " << A[i] << "\n";
    }
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
