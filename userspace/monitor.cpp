#include <fcntl.h>
#include <sys/ioctl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <sstream>
#include <map>
#include <thread>
#include <chrono>
#include <unistd.h>
#define DEVICE "/dev/linuxguard"

#define LINUXGUARD_IOCTL_MAGIC 'L'
#define LINUXGUARD_GET_STATUS _IOR(LINUXGUARD_IOCTL_MAGIC, 1, int)

namespace fs = std::filesystem;

struct ProcessInfo
{
    std::string name;
    unsigned long long memory_kb;
    unsigned long long cpu_time;
};

bool is_number(const std::string& s)
{
    if (s.empty())
        return false;

    for (char c : s)
    {
        if (c < '0' || c > '9')
            return false;
    }

    return true;
}

std::string get_value(const std::string& file,
                      const std::string& key)
{
    std::ifstream input(file);
    std::string line;

    while (std::getline(input, line))
    {
        if (line.rfind(key, 0) == 0)
            return line.substr(key.length());
    }

    return "";
}

unsigned long long get_memory_kb(const std::string& pid)
{
    std::string value =
        get_value("/proc/" + pid + "/status", "VmRSS:");

    if (value.empty())
        return 0;

    std::istringstream stream(value);

    unsigned long long memory = 0;
    stream >> memory;

    return memory;
}

unsigned long long get_process_cpu(const std::string& pid)
{
    std::ifstream file("/proc/" + pid + "/stat");

    if (!file.is_open())
        return 0;

    std::string line;

    if (!std::getline(file, line))
        return 0;

    /*
     * Process name can contain spaces.
     * Therefore, find the final ')' before parsing
     * the remaining fields.
     */

    size_t closing_bracket = line.rfind(')');

    if (closing_bracket == std::string::npos)
        return 0;

    std::string remaining =
        line.substr(closing_bracket + 2);

    std::istringstream stream(remaining);

    std::string value;

    unsigned long long utime = 0;
    unsigned long long stime = 0;

    /*
     * remaining starts at field 3.
     *
     * field 14 = utime
     * field 15 = stime
     */

    for (int field = 3; field <= 15; field++)
    {
        if (!(stream >> value))
            return 0;

        if (field == 14)
            utime = std::stoull(value);

        if (field == 15)
            stime = std::stoull(value);
    }

    return utime + stime;
}

unsigned long long get_total_cpu()
{
    std::ifstream file("/proc/stat");

    if (!file.is_open())
        return 0;

    std::string line;

    if (!std::getline(file, line))
        return 0;

    std::istringstream stream(line);

    std::string cpu;

    unsigned long long user = 0;
    unsigned long long nice = 0;
    unsigned long long system = 0;
    unsigned long long idle = 0;
    unsigned long long iowait = 0;
    unsigned long long irq = 0;
    unsigned long long softirq = 0;
    unsigned long long steal = 0;

    stream >> cpu
           >> user
           >> nice
           >> system
           >> idle
           >> iowait
           >> irq
           >> softirq
           >> steal;

    return user + nice + system + idle +
           iowait + irq + softirq + steal;
}

std::map<std::string, ProcessInfo> take_snapshot()
{
    std::map<std::string, ProcessInfo> processes;

    for (const auto& entry :
         fs::directory_iterator("/proc"))
    {
        std::string pid =
            entry.path().filename().string();

        if (!is_number(pid))
            continue;

        std::string status_file =
            "/proc/" + pid + "/status";

        std::string name =
            get_value(status_file, "Name:");

        if (name.empty())
            continue;

        ProcessInfo info;

        info.name = name;
        info.memory_kb = get_memory_kb(pid);
        info.cpu_time = get_process_cpu(pid);

        processes[pid] = info;
    }

    return processes;
}

int main()
{      int driver_fd = open(DEVICE, O_RDWR);

    if (driver_fd < 0)
    {
        perror("LinuxGuard driver");
        return 1;
    }

    int driver_status = 0;

    if (ioctl(driver_fd, LINUXGUARD_GET_STATUS, &driver_status) < 0)
    {
        perror("LinuxGuard ioctl");
        close(driver_fd);
        return 1;
    }

    std::cout << "Kernel Driver: "
              << (driver_status ? "ACTIVE" : "INACTIVE")
              << "\n\n";
    const double CPU_THRESHOLD = 20.0;
    const unsigned long long MEMORY_THRESHOLD_KB =
        300000;

    std::cout << "============================================\n";
    std::cout << "          LinuxGuard Monitor\n";
    std::cout << "============================================\n\n";

    std::cout << "Collecting system statistics...\n";
    std::cout << "Please wait 1 second...\n\n";

    /*
     * First system snapshot
     */

    unsigned long long total_cpu_1 =
        get_total_cpu();

    auto processes_1 =
        take_snapshot();

    /*
     * Wait before taking second snapshot.
     */

    std::this_thread::sleep_for(
        std::chrono::milliseconds(1000));

    /*
     * Second system snapshot
     */

    unsigned long long total_cpu_2 =
        get_total_cpu();

    auto processes_2 =
        take_snapshot();

    unsigned long long total_cpu_delta =
        total_cpu_2 - total_cpu_1;

    long cpu_count =
        sysconf(_SC_NPROCESSORS_ONLN);

    if (cpu_count < 1)
        cpu_count = 1;

    std::cout << "CPU cores: "
              << cpu_count << "\n\n";

    std::cout << "PID\tProcess\t\t\tMemory\tCPU\tStatus\n";

    std::cout
        << "------------------------------------------------------------"
        << "----------------\n";

    int anomaly_count = 0;

    for (const auto& [pid, info2] :
         processes_2)
    {
        auto it = processes_1.find(pid);

        if (it == processes_1.end())
            continue;

        const ProcessInfo& info1 =
            it->second;

        unsigned long long process_delta =
            info2.cpu_time - info1.cpu_time;

        double cpu_usage = 0.0;

        if (total_cpu_delta > 0)
        {
            /*
             * Multiply by number of CPU cores so that
             * approximately one fully utilized core
             * represents 100%.
             */

            cpu_usage =
                ((double)process_delta /
                 (double)total_cpu_delta)
                * 100.0
                * cpu_count;
        }

        double memory_mb =
            info2.memory_kb / 1024.0;

        bool anomaly =
            (cpu_usage > CPU_THRESHOLD) ||
            (info2.memory_kb >
             MEMORY_THRESHOLD_KB);

        std::cout << pid << "\t"
                  << info2.name << "\t\t"
                  << memory_mb << " MB\t"
                  << cpu_usage << "%\t";

    if (anomaly)
{
    std::cout
        << "ANOMALY";

    std::string event =
        "ANOMALY PID=" + pid +
        " PROCESS=" + info2.name;

    if (write(driver_fd, event.c_str(), event.size()) < 0)
    {
        perror("Driver write");
    }

    std::ofstream log_file(
        "../logs/linuxguard.log",
        std::ios::app
    );

    if (log_file.is_open())
    {
        log_file << event << "\n";
        log_file.close();
    }

    anomaly_count++;
}
        else
        {
            std::cout << "NORMAL";
        }

        std::cout << "\n";
    }

    std::cout
        << "\n============================================================\n";

    std::cout
        << "Anomaly Thresholds:\n";

    std::cout
        << "CPU     > "
        << CPU_THRESHOLD
        << "%\n";

    std::cout
        << "Memory  > 300 MB\n";

    std::cout
        << "Detected anomalies: "
        << anomaly_count
        << "\n";

    std::cout
        << "============================================================\n";
    close(driver_fd);
    return 0;
}
