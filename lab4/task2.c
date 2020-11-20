 #include <stdio.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <unistd.h>


int main() {
    int first_child = fork();
    int second_child;

    if ( first_child == -1 ) {
        perror("Couldn't fork.");
        exit(1);
    }
    else if ( first_child == 0 ) {
        printf( "Child: pid=%d; group=%d; parent=%d\n", getpid(), getpgrp(), getppid() );
        return 0;
    }
    printf( "Parent: pid=%d; group=%d; child=%d\n", getpid(), getpgrp(), first_child );
    
    second_child = fork();
    if ( second_child == -1 ) {
        perror("Couldn't fork.");
        exit(1);
    }
    else if ( second_child == 0 ) {
        printf( "Child: pid=%d; group=%d; parent=%d\n", getpid(), getpgrp(), getppid() );
        return 0;
        }
    printf( "Parent: pid=%d; group=%d; child=%d\n", getpid(), getpgrp(), second_child );
        

    if (first_child != 0 && second_child != 0) {
        int status1;
        int status2;
        pid_t ret_value1 = wait(&status1);

        if (WIFEXITED(status1))
            printf("Parent: child %d finished with %d code.\n", ret_value1, WEXITSTATUS(status1) );
        else if (WIFSIGNALED(status1))
            printf( "Parent: child %d finished from signal with %d code.\n", ret_value1, WTERMSIG(status1));
        else if (WIFSTOPPED(status1))
            printf("Parent: child %d finished from signal with %d code.\n", ret_value1, WSTOPSIG(status1));

        pid_t ret_value2 = wait(&status2);

        if (WIFEXITED(status2))
                printf("Parent: child %d finished with %d code.\n", ret_value2, WEXITSTATUS(status2) );
        else if (WIFSIGNALED(status2))
            printf( "Parent: child %d finished from signal with %d code.\n", ret_value2, WTERMSIG(status2));
        else if (WIFSTOPPED(status2))
            printf("Parent: child %d finished from signal with %d code.\n", ret_value2, WSTOPSIG(status2));
    }
    return 0;
}