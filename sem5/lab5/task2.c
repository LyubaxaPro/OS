
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define READERS_CNT 5
#define WRITERS_CNT 3

#define ACTIVE_READERS 0
#define ACTIVE_WRITER 1
#define WAITING_WRITERS 2
#define WAITING_READERS 3
#define TRUE 1

struct sembuf start_writer[5] = {{WAITING_WRITERS,   1, 0}, {ACTIVE_READERS,    0, 0}, {ACTIVE_WRITER,     0, 0},
{ACTIVE_WRITER,     1, 0}, {WAITING_WRITERS,  -1, 0}};

struct sembuf stop_writer[] = {{ACTIVE_WRITER, -1, 0}};

struct sembuf start_reader[5] = {{WAITING_READERS, 1, 0}, {WAITING_WRITERS, 0, 0},
{ACTIVE_WRITER, 0, 0}, {ACTIVE_READERS, 1, 0}, {WAITING_READERS, -1, 0}};

struct sembuf stop_reader[] = {{ACTIVE_READERS, -1, 0}};

void start_read(int sem_id)
{
    semop(sem_id, start_reader, 5);
}

void stop_read(int sem_id)
{
    semop(sem_id, stop_reader, 1);
}

void start_write(int sem_id)
{
    semop(sem_id, start_writer, 5);
}

void stop_write(int sem_id)
{
    semop(sem_id, stop_writer, 1);
}

void reader(int num, int sem_id, int* buf)
{
    while (TRUE) 
    {
        start_read(sem_id);
        printf("Reader №%d read: %d\n", num, *buf);
        stop_read(sem_id);
        sleep(rand() % 3);
    }
}

void writer(int num, int sem_id, int* buf)
{
    while (TRUE) 
    {
        start_write(sem_id);
        printf("Writer №%d write: %d\n", num, ++(*buf));
        stop_write(sem_id);
        sleep(rand() % 3);
    }
}

int create_proc(int sem_id, int* mem_ptr)
{
    int processes_cnt = 0;
    pid_t pid;
    *mem_ptr = 0;
    for (int i = 0; i < WRITERS_CNT; i++) 
    {
        if ((pid = fork())== -1)
        {
            perror("Can’t fork.\n");
            return 1;
        }
        if (!pid)
            writer(i + 1, sem_id, mem_ptr);
        else
            processes_cnt++;
    }

    for (int i = 0; i < READERS_CNT; i++) 
    {
        if ((pid = fork())== -1)
        {
            perror("Can’t fork.\n");
            return 1;
        }
        if (!pid)
            reader(i + 1, sem_id, mem_ptr);
        else
            processes_cnt++;
    }
    return processes_cnt;
}

int main()
{
    int flags = S_IRWXU | S_IRWXG | S_IRWXO;
    int shm_id, sem_id;
    int *mem_ptr;

    if ((shm_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | flags)) == -1)
    {
        perror("Shmget error"); 
        return 1;
    }

    if ((mem_ptr = shmat(shm_id, NULL, 0)) == -1) 
    {
        perror("Shmat error");
        return 1;
    }

    if ((sem_id = semget(IPC_PRIVATE, 4, flags)) == -1) 
    {
        perror("Semget error\n");
        return 1;
    }

    semctl(sem_id, ACTIVE_READERS,  SETVAL, 0); 
    semctl(sem_id, ACTIVE_WRITER,   SETVAL, 0); 
    semctl(sem_id, WAITING_READERS, SETVAL, 0); 
    semctl(sem_id, WAITING_WRITERS, SETVAL, 0); 
    int processes_cnt = create_proc(sem_id, mem_ptr);
    
    int status;
    for (int i = 0; i < processes_cnt; i++)
    {
        wait(&status);
        if (!WIFEXITED(status))
            printf("Error, code = %d\n", status);
    }

    shmdt(mem_ptr);
    semctl(sem_id, 0, IPC_RMID);
    shmctl(shm_id, IPC_RMID, 0);

    return 0;
}


