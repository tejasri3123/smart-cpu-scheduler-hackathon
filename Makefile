CC = gcc
CFLAGS = -Wall -Wextra -std=c11
PTHREAD_FLAGS = -pthread

BINARIES = \
	scheduler \
	modules/co1_os_services/co1_demo \
	modules/co2_process_control/co2_demo \
	modules/co3_ipc/scheduler_pipe \
	modules/co3_ipc/fifo_demo \
	modules/co3_ipc/unix_socket_demo \
	modules/co3_ipc/signal_demo \
	modules/co3_ipc/shared_memory_demo \
	modules/co4_virtual_memory/memory_demo \
	modules/co5_file_system/scheduler_file_io \
	modules/co6_concurrency/concurrency_demo

.PHONY: all clean

all: $(BINARIES)

scheduler: src/scheduler.c
	$(CC) $(CFLAGS) $< -o $@

modules/co1_os_services/co1_demo: modules/co1_os_services/co1_demo.c
	$(CC) $(CFLAGS) $< -o $@

modules/co2_process_control/co2_demo: modules/co2_process_control/co2_demo.c
	$(CC) $(CFLAGS) $< -o $@

modules/co3_ipc/scheduler_pipe: modules/co3_ipc/scheduler_pipe.c
	$(CC) $(CFLAGS) $< -o $@

modules/co3_ipc/fifo_demo: modules/co3_ipc/fifo_demo.c
	$(CC) $(CFLAGS) $< -o $@

modules/co3_ipc/unix_socket_demo: modules/co3_ipc/unix_socket_demo.c
	$(CC) $(CFLAGS) $< -o $@

modules/co3_ipc/signal_demo: modules/co3_ipc/signal_demo.c
	$(CC) $(CFLAGS) $< -o $@

modules/co3_ipc/shared_memory_demo: modules/co3_ipc/shared_memory_demo.c
	$(CC) $(CFLAGS) $< -o $@

modules/co4_virtual_memory/memory_demo: modules/co4_virtual_memory/memory_demo.c
	$(CC) $(CFLAGS) $< -o $@

modules/co5_file_system/scheduler_file_io: modules/co5_file_system/scheduler_file_io.c
	$(CC) $(CFLAGS) $< -o $@

modules/co6_concurrency/concurrency_demo: modules/co6_concurrency/concurrency_demo.c
	$(CC) $(CFLAGS) $(PTHREAD_FLAGS) $< -o $@

clean:
	rm -f $(BINARIES)
