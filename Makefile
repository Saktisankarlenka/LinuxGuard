DRIVER_DIR := driver
USER_DIR := userspace
TEST_DIR := tests

.PHONY: all driver userspace tests clean

all: driver userspace tests

driver:
	$(MAKE) -C $(DRIVER_DIR)

userspace:
	g++ $(USER_DIR)/monitor.cpp -o $(USER_DIR)/linuxguard_monitor

tests:
	gcc $(TEST_DIR)/driver_test.c -o $(TEST_DIR)/driver_test
	gcc $(TEST_DIR)/ioctl_test.c -o $(TEST_DIR)/ioctl_test

clean:
	$(MAKE) -C $(DRIVER_DIR) clean
	rm -f $(USER_DIR)/linuxguard_monitor
	rm -f $(TEST_DIR)/driver_test $(TEST_DIR)/ioctl_test
