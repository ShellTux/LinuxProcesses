#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#define CHILD_ID 0
#define FORK_INVALID_ID -1

void printForkID(void)
{
	const pid_t id = fork();

	if (id == CHILD_ID) {
		printf("Child Process: %d\n", id);
	} else if (id == FORK_INVALID_ID) {
		fprintf(stderr, "Error forking process\n");
	} else {
		printf("Main Process: %d\n", id);
	}
}

void forkMainProcessOnly(void)
{
	int id = fork();

	if (id == FORK_INVALID_ID) {
		return;
	}

	if (id != CHILD_ID) {
		fork();
	}

	printf("Fork\n");
}

void createNChildProcesses(const size_t number)
{
	int id;
	for (size_t i = 0; i < number; ++i) {
		id = fork();

		if (id == CHILD_ID) {
			break;
		}
	}

	printf("Fork: %d\n", id);
}

void printHalf(const size_t number)
{
	const pid_t id = fork();
	const int start = (id == CHILD_ID)
		? number / 2
		: 0;
	const int end = start + number / 2;

	if (id != CHILD_ID) {
		wait(NULL);
	}

	for (int i = start; i < end; ++i) {
		printf("%d ", i);
		fflush(stdout);
	}

	if (id != CHILD_ID) {
		printf("\n");
	}
}

void parentPID(void) {
	int id = fork();

	if (id == FORK_INVALID_ID) {
		printf("Error Forking process\n");
		return;
	} else if (id == CHILD_ID) {
		printf("Child, ID: %d, PID: %d, PPID: %d\n", id, getpid(), getppid());
	} else {
		printf("Main, ID: %d, PID: %d, PPID: %d\n", id, getpid(), getppid());
	}
}

void createZombieProcess(void) {
	int id = fork();

	if (id == FORK_INVALID_ID) {
		printf("Error Forking process\n");
		return;
	} else if (id == CHILD_ID) {
		// NOTE: Waiting 1 second in Child process so that the parent
		// process terminates first. The Child process with a dead
		// parent process is an orphan process
		sleep(1);
		// NOTE: Another process is going to take over as the parent
		// process
		printf("Child, ID: %d, PID: %d, PPID: %d\n", id, getpid(), getppid());
	} else {
		printf("Main, ID: %d, PID: %d, PPID: %d\n", id, getpid(), getppid());
	}

	// NOTE: Good practice to way for child processes to prevent memory
	// leaks. A process not acknowledge by the parent process is a zombie
	// process
	pid_t res;
	// NOTE: wait returns the PID of the waited process
	if ((res = wait(NULL)) == -1) {
		fprintf(stderr, "No children to wait for...\n");
	} else {
		printf("Finished execution of PID: %d\n", res);
	}
}

#define FUNCTION 6

int main(void)
{
#if FUNCTION == 1
	printForkID();
#elif FUNCTION == 2
	forkMainProcessOnly();
#elif FUNCTION == 3
	createNChildProcesses(5);
#elif FUNCTION == 4
	printHalf(10);
#elif FUNCTION == 5
	parentPID();
#elif FUNCTION == 6
	createZombieProcess();
#endif

	return 0;
}
