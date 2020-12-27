#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/wait.h>

#define BIN_SEM 0
#define EMPTY_SEM 1
#define FULL_SEM 2
#define PRODUCERS_CNT 3
#define CONSUMERS_CNT 3
#define N 8

#define TRUE 1

struct sembuf p_consumer[] = {{FULL_SEM, -1, 0}, {BIN_SEM, -1, 0}};
struct sembuf v_consumer[] = {{BIN_SEM, 1, 0}, {EMPTY_SEM,  1, 0}};
struct sembuf v_producer[] ={{BIN_SEM, 1, 0}, {FULL_SEM, 1, 0}};
struct sembuf p_producer[] = {{EMPTY_SEM, -1, 0}, {BIN_SEM, -1, 0}};

int create_proc(int sem_id, char* mem_ptr)
{
    int processes_cnt = 0;
    pid_t pid;
    int* producer_n = mem_ptr +  N;
    int* letter = producer_n + 1;
    int* consumer_n = producer_n + 2;

    *producer_n = *consumer_n = *letter = 0;

    for (int i = 0; i < PRODUCERS_CNT; i++) 
    {
        if ((pid = fork())== -1)
        {
            perror("Can’t fork.\n");
            return 1;
        }
        if (!pid)
            produce(i + 1, sem_id, mem_ptr, producer_n, letter);
        else
            processes_cnt++;
    }

    for (int i = 0; i < CONSUMERS_CNT; i++) 
    {
        if ((pid = fork())== -1)
        {
            perror("Can’t fork.\n");
            return 1;
        }
        if (!pid)
            consume(i + 1, sem_id, mem_ptr, consumer_n);
        else
            processes_cnt++;
    }
    return processes_cnt;
}

void produce(int num, int sem_id, char* buf, int* n, int* letter)
{
    while (TRUE) 
    {
        semop(sem_id, p_producer, 2);
        buf[*n] = 'a' + *letter;
        printf("Producer №%d write to buf[%d] = %c\n", num, *n, buf[*n]);
        *n = *n == N - 1 ? 0 : *n + 1;
        *letter = *letter == 25 ? 0 : *letter + 1;
        semop(sem_id, v_producer, 2);
        sleep(rand() % 4);
    }
}

void consume(int num, int sem_id, char* buf, int* n)
{
    while (TRUE) 
    {
        semop(sem_id, p_consumer, 2);
        printf("Consumer №%d read  buf[%d] = %c\n", num, *n, buf[*n]);
        *n = *n == N - 1 ? 0 : *n + 1;
        semop(sem_id, v_consumer, 2);
        sleep(rand() % 5);
    }
}


int main()
{
    int flags = S_IRWXU | S_IRWXG | S_IRWXO;
    int shm_id, sem_id;
    char *mem_ptr = -1;
if ((shm_id = shmget(IPC_PRIVATE, N * sizeof(char) + 3 * sizeof(int), IPC_CREAT | flags)) == -1)
    {
        perror("Shmget error"); 
        return 1;
    }

    if ((mem_ptr = shmat(shm_id, NULL, 0)) == -1) 
    {
        perror("Shmat error");
        return 1;
    }

    if ((sem_id = semget(IPC_PRIVATE, 3, flags)) == -1) 
    {
        perror("Semget error.\n");
        return 1;
    }

    semctl(sem_id, BIN_SEM, SETVAL, 1); 
    semctl(sem_id, EMPTY_SEM, SETVAL, N); 
    semctl(sem_id, FULL_SEM, SETVAL, 0); 
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



