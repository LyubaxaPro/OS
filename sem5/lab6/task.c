#include <stdio.h>

#include <stdbool.h>

#include <windows.h>

#define READERS_CNT 5

#define WRITERS_CNT 3

const DWORD sleep_time = 350;

const int iterations = 8;

volatile LONG active_readers = 0;

bool writer_is_active = false;

volatile LONG writers_in_wait = 0;

volatile LONG readers_in_wait = 0;

volatile int current_num = 0;

HANDLE mutex;

HANDLE can_read;

HANDLE can_write;

HANDLE writers[WRITERS_CNT];

HANDLE readers[READERS_CNT];

void start_write()

{

	InterlockedIncrement(&writers_in_wait);

	if ( WaitForSingleObject(can_read, 0) == WAIT_OBJECT_0|| writer_is_active)

	{

		WaitForSingleObject(can_write, INFINITE);

	}

	InterlockedDecrement(&writers_in_wait);

	writer_is_active = true;

	ResetEvent(can_write);

}

void stop_write()

{

	writer_is_active = false;

	if (!writers_in_wait)

	{

		SetEvent(can_read);

	}

	else

	{

		SetEvent(can_write);

	}

}

void start_read()

{

	InterlockedIncrement(&readers_in_wait);

	if (WaitForSingleObject(can_write, 0) == WAIT_OBJECT_0 || writer_is_active)

	{

		WaitForSingleObject(can_read, INFINITE);

	}

	WaitForSingleObject(mutex, INFINITE);

	InterlockedDecrement(&readers_in_wait);

	InterlockedIncrement(&active_readers);

	SetEvent(can_read);

	ReleaseMutex(mutex);

}

void stop_read()

{

	InterlockedDecrement(&active_readers);

	if (readers_in_wait == 0)

	{

		SetEvent(can_write);

	}

}

DWORD WINAPI writer(LPVOID lpParams)

{

	for (int current_it = 0; current_it < iterations; current_it++)

	{

		start_write();

		current_num += 1;

		printf("Writer #%ld write: %d\n", (int) lpParams, current_num);

		stop_write();

		Sleep(sleep_time);

	}

	return EXIT_SUCCESS;

}

DWORD WINAPI reader(LPVOID lpParams)

{

	while (current_num < iterations * WRITERS_CNT)

	{

		start_read();

		printf("Reader #%ld read: %d\n", (int) lpParams, current_num);

		stop_read();

		Sleep(sleep_time);

	}

	return EXIT_SUCCESS;

}

int init_handles()

{

	if ((mutex = CreateMutex(NULL, FALSE, NULL)) == NULL)

	{

		perror("Error. Can not create mutex!");

		return EXIT_FAILURE;

	}

	if ((can_read = CreateEvent(NULL, FALSE, TRUE, NULL)) == NULL)

	{

		perror("Error. Can not create event can_read!");

		return EXIT_FAILURE;

	}

	if ((can_write = CreateEvent(NULL, TRUE, TRUE, NULL)) == NULL)

	{

		perror("Error. Can not create event can_write!");

		return EXIT_FAILURE;

	}

	return EXIT_SUCCESS;

}

int create_threads(HANDLE *threads, int threads_count, DWORD (*on_thread)(LPVOID))

{

	for (int cur_thread_num = 0; cur_thread_num < threads_count; cur_thread_num++)

	{

		if ((threads[cur_thread_num] = CreateThread(NULL, 0, on_thread, (LPVOID) cur_thread_num, 0, NULL)) == NULL)

		{

			perror("Error. Can not create event create thread!");

			return EXIT_FAILURE;

		}

	}

	return EXIT_SUCCESS;

}

int main()

{

	setbuf(stdout, NULL);

	int rc = EXIT_SUCCESS;

	if ((rc = init_handles()) != EXIT_SUCCESS || (rc = create_threads(writers, WRITERS_CNT, writer)) != EXIT_SUCCESS

	|| (rc = create_threads(readers, READERS_CNT, reader)) != EXIT_SUCCESS){ return rc;}

	WaitForMultipleObjects(WRITERS_CNT, writers, TRUE, INFINITE);

	WaitForMultipleObjects(READERS_CNT, readers, TRUE, INFINITE);

	CloseHandle(mutex);

	CloseHandle(can_read);

	CloseHandle(can_write);

	return rc;

}