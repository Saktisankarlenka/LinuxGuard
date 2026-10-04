# LinuxGuard Execution Guide

## 1. Environment

LinuxGuard must be built and executed on a Linux system with the matching Linux kernel headers installed.

Test environment:

- Ubuntu Linux
- GCC / G++
- Linux kernel headers
- Make
- Git
- GDB

## 2. Build the Complete Project

From the project root:

cd ~/LinuxGuard
make

This builds:

- Linux kernel module
- C++ monitoring application
- Driver communication test
- IOCTL test

## 3. Load the Kernel Driver

Navigate to the driver directory:

cd ~/LinuxGuard/driver

Load the module:

sudo insmod linuxguard.ko

Verify the module:

lsmod | grep linuxguard

Verify the device:

ls -l /dev/linuxguard

## 4. Test Character Device Communication

Navigate to the tests directory:

cd ~/LinuxGuard/tests

Run:

sudo ./driver_test

The test demonstrates:

- open()
- write()
- read()
- close()

Expected output includes successful device opening, data transmission, data reception, and device closing.

## 5. Test IOCTL Communication

Run:

sudo ./ioctl_test

Expected result:

Driver status: ACTIVE

This verifies communication between user space and the kernel driver using ioctl().

## 6. Run LinuxGuard Monitor

Navigate to the userspace directory:

cd ~/LinuxGuard/userspace

Run:

sudo ./linuxguard_monitor

The application:

1. Opens /dev/linuxguard.
2. Checks driver status using ioctl().
3. Reads process information from /proc.
4. Calculates CPU and memory usage.
5. Applies anomaly thresholds.
6. Displays NORMAL or ANOMALY status.
7. Sends anomaly events to the kernel driver.
8. Records events in the project log.

## 7. Verify Kernel Events

After running the monitor:

dmesg | grep "LinuxGuard:"

This displays driver-side events generated using printk().

## 8. Verify Application Logs

Check the recorded anomaly events:

cat ~/LinuxGuard/logs/linuxguard.log

## 9. Complete Demonstration Sequence

Use the following sequence during the project demonstration:

1. cd ~/LinuxGuard
2. make
3. cd driver
4. sudo insmod linuxguard.ko
5. lsmod | grep linuxguard
6. ls -l /dev/linuxguard
7. cd ../tests
8. sudo ./driver_test
9. sudo ./ioctl_test
10. cd ../userspace
11. sudo ./linuxguard_monitor
12. dmesg | grep "LinuxGuard:"
13. cat ../logs/linuxguard.log

## 10. Unload the Driver

After completing the demonstration, return to the driver directory:

cd ~/LinuxGuard/driver

Unload the module:

sudo rmmod linuxguard

Verify removal:

lsmod | grep linuxguard

The command should return no matching module.

## 11. Rebuild After Source Changes

From the project root:

cd ~/LinuxGuard
make clean
make

Reload the driver before running the tests again.

## 12. Troubleshooting

### Module already loaded

If insmod reports that the file already exists, check:

lsmod | grep linuxguard

If the module is already loaded, do not run insmod again. Use the existing loaded module or unload it first with:

sudo rmmod linuxguard

### Device not found

If /dev/linuxguard does not exist, verify that the kernel module is loaded:

lsmod | grep linuxguard

Then check kernel messages:

dmesg | grep "LinuxGuard:"

### Permission denied

The character device is created with root-only permissions in the current setup. Run the test applications with sudo.
