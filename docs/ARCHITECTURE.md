# LinuxGuard Architecture

## 1. Overview

LinuxGuard is divided into two major execution layers: a C++ user-space monitoring application and a C Linux kernel character device driver.

The user-space application collects process information from the Linux /proc filesystem, analyzes CPU and memory usage, detects anomalies using configured thresholds, and communicates anomaly events to the kernel driver.

## 2. Architecture

```text
                    LINUX OPERATING SYSTEM
                             |
              +--------------v--------------+
              |       /proc Filesystem      |
              | Process / CPU / Memory Data |
              +--------------+--------------+
                             |
                             v
              +-----------------------------+
              |     C++ Monitoring Engine    |
              |-----------------------------|
              | Process Discovery            |
              | CPU Usage Calculation        |
              | Memory Monitoring            |
              | Anomaly Detection            |
              +--------------+--------------+
                             |
                       ioctl() / write()
                             |
                             v
              +-----------------------------+
              |      /dev/linuxguard         |
              |    Character Device Node     |
              +--------------+--------------+
                             |
                             v
              +-----------------------------+
              | LinuxGuard Kernel Driver     |
              |            C                 |
              |-----------------------------|
              | open()                      |
              | read()                      |
              | write()                     |
              | ioctl()                     |
              | release()                   |
              +--------------+--------------+
                             |
                             v
                    Kernel Event Logging
                         dmesg / printk
```

## 3. User-Space Layer

The user-space monitoring engine is implemented in C++.

Responsibilities:

- Discover running processes using /proc.
- Read process memory usage from /proc/<PID>/status.
- Read process CPU time from /proc/<PID>/stat.
- Read total CPU time from /proc/stat.
- Calculate CPU usage using two snapshots taken one second apart.
- Compare resource usage against configured thresholds.
- Classify processes as NORMAL or ANOMALY.
- Send anomaly events to the kernel driver.
- Record anomaly events in the project log.

## 4. Kernel-Space Layer

The kernel-space component is implemented as a Linux Kernel Module written in C.

The driver creates the character device:

/dev/linuxguard

The driver implements the following file operations:

- open()
- read()
- write()
- ioctl()
- release()

The driver uses copy_to_user() and copy_from_user() for safe data transfer between user space and kernel space.

## 5. Kernel/User-Space Communication

LinuxGuard uses two communication mechanisms.

### IOCTL

The C++ application uses ioctl() to request the current driver status.

The driver returns an ACTIVE status when the module is available.

### write()

When an anomaly is detected, the C++ monitoring application writes an event message to /dev/linuxguard.

The driver receives the message using its write() operation and records the event using printk().

## 6. Data Flow

The complete data flow is:

1. Linux exposes process information through /proc.
2. The C++ application reads process information.
3. CPU and memory usage are calculated.
4. Resource usage is compared with configured thresholds.
5. Anomalous processes are identified.
6. Anomaly information is sent to /dev/linuxguard.
7. The Linux character driver receives the event.
8. The driver records the event in the kernel log.
9. The event is also stored in logs/linuxguard.log.

## 7. Threshold-Based Detection

LinuxGuard currently uses two thresholds:

- CPU usage greater than 20 percent.
- Memory usage greater than 300 MB.

If either threshold is exceeded, the process is classified as ANOMALY.

## 8. Technologies and Concepts

- Linux OS
- C
- C++
- Linux Kernel Modules
- Character Device Drivers
- /proc filesystem
- ioctl()
- read() / write()
- copy_to_user()
- copy_from_user()
- printk()
- GCC / G++
- Make
- Git / GitHub

## 9. Design Goal

The architecture is intentionally lightweight so that the project demonstrates fundamental Linux system programming and kernel development concepts without depending on Python, Java, hardware devices, or external services.
