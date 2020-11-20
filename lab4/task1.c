#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
	int first_child = fork();
	if (first_child == -1) {
		perror("Can't fork");
		exit(1);
	}
	else if (first_child == 0) {
		printf("Child: pid=%d, pidid=%d, groupid=%d\n", getpid(), getppid(), getpgrp());
		sleep(5);
		printf("Child: pid=%d, pidid=%d, groupid=%d\n", getpid(), getppid(), getpgrp());
		return 0;
	}
	printf("Parent: pid=%d, childpid=%d, groupid=%d\n", getpid(), first_child, getpgrp());

	int second_child = fork();
	if (second_child == -1) {
		perror("Can't fork");
		exit(1);
	} else if (second_child == 0) {
	        printf("Child: pid=%d, pidid=%d, groupid=%d\n", getpid(), getppid(), getpgrp());
		sleep(5);
		printf("Child: pid=%d, pidid=%d, groupid=%d\n", getpid(), getppid(), getpgrp());
        	return 0;
	}
	printf("Parent: pid=%d, childpid=%d, groupid=%d\n", getpid(), second_child, getpgrp());
	return 0;
}