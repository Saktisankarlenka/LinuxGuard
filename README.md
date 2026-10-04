# LinuxGuard

## Kernel-Aware Process & Resource Anomaly Monitor

LinuxGuard is a Linux-based system monitoring project developed using C, C++, and Linux Kernel Module programming.

The system monitors running processes and resource usage, detects abnormal CPU and memory consumption, and demonstrates communication between user space and kernel space through a custom Linux character device driver.

---

## 1. Project Objective

LinuxGuard is designed to:

- Monitor running Linux processes
- Collect CPU and memory usage information
- Detect abnormal resource usage
- Communicate between user space and kernel space
- Implement a custom Linux character device driver
- Demonstrate read(), write(), and ioctl() operations
- Record detected anomaly events

## 2. Technologies Used

- Linux OS
- C
- C++
- Linux Kernel Modules
- Linux Character Device Driver
- /proc filesystem
- ioctl()
- read() / write()
- GCC / G++
- Make
- GDB
- Git / GitHub

---

## 3. System Architecture

```text
                    LINUX OPERATING SYSTEM
                              |
                    +---------v---------+
                    |   /proc Filesystem|
                    +---------+---------+
                              |
                              v
                    +-------------------+
                    | C++ Monitoring     |
                    | Engine             |
                    +---------+---------+
                              |
                    CPU / Memory Analysis
                              |
                    +---------v---------+
                    | Anomaly Detection |
                    +---------+---------+
                              |
                              v
                    +-------------------+
                    | /dev/linuxguard   |
                    | Character Device  |
                    +---------+---------+
                              |
                              v
                    +-------------------+
                    | Linux Kernel      |
                    | Character Driver  |
                    |       C           |
                    +---------+---------+
                              |
                              v
                    Kernel Event Logging
```

---

## 4. Main Features

### Process Monitoring

LinuxGuard reads process information from the Linux /proc filesystem.

### CPU Monitoring

The monitor takes two CPU snapshots one second apart and calculates process CPU usage.

### Memory Monitoring

Process resident memory is obtained from /proc/<PID>/status using the VmRSS value.

### Anomaly Detection

A process is marked as an anomaly when its CPU or memory usage exceeds the defined threshold.

Current thresholds:

CPU     > 20%
Memory  > 300 MB

### Linux Character Device Driver

The project implements:

/dev/linuxguard

The driver supports:

- open()
- read()
- write()
- ioctl()
- release()

### Kernel/User-Space Communication

The C++ monitoring application communicates with the Linux kernel driver through the character device.

Anomaly events are sent to the driver using write().

Driver status is obtained using ioctl().

### Event Logging

Detected anomalies are recorded in:

logs/linuxguard.log

---

## 5. Project Structure

LinuxGuard/
|
+-- driver/
|   +-- linuxguard.c
|   +-- Makefile
|
+-- userspace/
|   +-- monitor.cpp
|
+-- tests/
|   +-- driver_test.c
|   +-- ioctl_test.c
|
+-- config/
+-- docs/
+-- scripts/
+-- logs/
|
+-- .gitignore
+-- README.md

Generated kernel and executable files are excluded from Git using .gitignore.

---

## 6. Requirements

LinuxGuard requires:

- Linux operating system
- GCC
- G++
- Make
- Linux kernel headers
- Git
- GDB

On Ubuntu, install the required packages with:

sudo apt update
sudo apt install build-essential linux-headers-$(uname -r) git gdb make

---

## 7. Build the Linux Kernel Driver

Navigate to the driver directory:

cd ~/LinuxGuard/driver

Build the kernel module:

make

The build generates:

linuxguard.ko

---

## 8. Load the LinuxGuard Kernel Driver

After building the driver, load the kernel module with:

sudo insmod linuxguard.ko

Verify that the module is loaded:

lsmod | grep linuxguard

Verify that the character device has been created:

ls -l /dev/linuxguard

Expected device:

/dev/linuxguard

---

## 9. Test Kernel Driver Communication

The project includes a user-space test program to verify communication with the Linux character device.

Navigate to the tests directory:

cd ~/LinuxGuard/tests

Compile the driver test:

gcc driver_test.c -o driver_test

Run the test:

sudo ./driver_test

The test verifies:

- Device open operation
- Writing data from user space to the kernel
- Reading data from the kernel to user space
- Device close operation

---

## 10. Test IOCTL Communication

LinuxGuard uses an IOCTL interface to query the status of the kernel driver from user space.

Navigate to the tests directory:

cd ~/LinuxGuard/tests

Compile the IOCTL test:

gcc ioctl_test.c -o ioctl_test

Run the test:

sudo ./ioctl_test

Expected output:

Device opened successfully.
Driver status: ACTIVE
Device closed successfully.

---

## 11. Run the C++ Monitoring Engine

The C++ monitoring engine collects process and resource information from the Linux /proc filesystem.

It monitors:

- Running processes
- Process memory usage
- Process CPU usage
- Number of available CPU cores

Navigate to the userspace directory:

cd ~/LinuxGuard/userspace

Compile the monitor:

g++ monitor.cpp -o linuxguard_monitor

Run the monitor:

sudo ./linuxguard_monitor

---

## 12. Anomaly Detection

LinuxGuard uses threshold-based anomaly detection.

Current thresholds:

- CPU usage greater than 20%
- Memory usage greater than 300 MB

If a process exceeds either threshold, LinuxGuard marks the process as ANOMALY.

Otherwise, the process is marked as NORMAL.

Detected anomaly events are sent from the C++ monitoring engine to the Linux kernel driver through the character device.

---

## 13. Event Logging

LinuxGuard records detected anomaly events in the project log file:

logs/linuxguard.log

Each detected anomaly records the process ID and process name.

Example:

ANOMALY PID=3061 PROCESS=gnome-shell
ANOMALY PID=4099 PROCESS=ptyxis

The same anomaly event is also sent to the kernel driver through the Linux character device.

---

## 14. Complete Execution Flow

1. Build the Linux kernel driver.
2. Load the LinuxGuard kernel module using insmod.
3. The driver creates the /dev/linuxguard character device.
4. The C++ monitoring engine opens the character device.
5. The monitoring engine uses IOCTL to verify that the driver is active.
6. Process information is collected from the Linux /proc filesystem.
7. CPU and memory usage are calculated for running processes.
8. Each process is compared against the configured anomaly thresholds.
9. Normal processes are marked as NORMAL.
10. Processes exceeding a threshold are marked as ANOMALY.
11. Anomaly events are written to the kernel driver through the character device.
12. The anomaly events are also recorded in logs/linuxguard.log.
13. Kernel-side events can be verified using dmesg.

---

## 15. Verification Commands

Check the loaded kernel module:

lsmod | grep linuxguard

Check the character device:

ls -l /dev/linuxguard

Check kernel messages:

dmesg | grep "LinuxGuard:"

Check recorded anomaly events:

cat ~/LinuxGuard/logs/linuxguard.log

---

## 16. Current Limitations

LinuxGuard is a lightweight prototype for demonstrating Linux monitoring and kernel-user space communication.

Current limitations include:

- Thresholds are currently defined in the C++ source code.
- Monitoring uses periodic snapshots of the /proc filesystem.
- The project does not automatically terminate or block anomalous processes.
- The current driver provides basic character-device communication and event reception.
- The anomaly detection mechanism is rule-based rather than machine-learning based.

---

## 17. Future Improvements

Possible future improvements include:

- Runtime configuration of CPU and memory thresholds.
- More detailed process-state and resource monitoring.
- Improved kernel-side event management.
- Multithreaded monitoring for larger process sets.
- More advanced anomaly detection techniques.
- Additional Linux kernel interfaces for deeper system observability.

---

## 18. Author

**Sakti Sankar Lenka**

B.Tech Computer Science and Engineering (IoT)
SOA University, Bhubaneswar

This project was developed as part of a Linux, C/C++, and Linux Device Driver capstone project.

---

## 19. Demonstration

The project can be demonstrated using the following sequence:

1. Build the Linux kernel driver using make.
2. Load the LinuxGuard kernel module using insmod.
3. Verify the /dev/linuxguard character device.
4. Run driver_test to demonstrate open, read, write, and close operations.
5. Run ioctl_test to demonstrate kernel status communication using ioctl().
6. Start the C++ monitoring engine.
7. Observe CPU and memory usage of running processes.
8. Observe processes classified as NORMAL or ANOMALY.
9. Check linuxguard.log for recorded anomaly events.
10. Use dmesg to verify that anomaly events reached the kernel driver.

---

## 20. Technical Concepts Demonstrated

LinuxGuard demonstrates the following concepts:

- Linux operating system fundamentals
- Linux kernel module development
- Linux character device drivers
- Kernel-space and user-space communication
- File operations in Linux drivers
- read() and write() system interfaces
- IOCTL-based communication
- copy_to_user() and copy_from_user()
- /proc filesystem monitoring
- Process and resource monitoring
- CPU and memory analysis
- Threshold-based anomaly detection
- Kernel event logging using printk and dmesg
- C and C++ programming
- Make-based build system
- Git and GitHub based project management

---

## 21. Conclusion

LinuxGuard demonstrates a complete Linux-based monitoring workflow combining a C++ user-space application with a custom Linux character device driver.

The project monitors process resource usage, identifies threshold-based anomalies, communicates anomaly events to the Linux kernel, and records the resulting events for verification.

The implementation provides practical experience with Linux system programming, kernel modules, device drivers, /proc filesystem monitoring, IOCTL communication, and C/C++ development.
