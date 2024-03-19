#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

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

void createOrphanProcess(void) {
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

typedef struct {
	int id;
	int *result;
	pthread_t thread;
} Thread;

#define N_THREADS 10
#define RANDOM_SLEEP 500

void* threadRoutine(void *threadID) {
	int id = *(int *) threadID;
	printf("[%02d] Thread, start working...\n", id);
	usleep(rand() % RANDOM_SLEEP);
	printf("[%02d] Thread, end work. Leaving...\n", id);
	int *result = malloc(sizeof(int));
	*result = rand() % 20;
	pthread_exit((void *) result);
}

int simpleThreads() {
	srand(time(NULL));
	Thread threads[N_THREADS] = {0};

	for (int i = 0; i < N_THREADS; ++i) {
		threads[i].id = i + 1;
		if (pthread_create(
					&threads[i].thread,
					NULL,
					threadRoutine,
					&threads[i].id
				  ) != 0) {
			return 1;
		}
	}

	// NOTE: All threads created, if we don't wait for them the process will
	// end before the threads finish. So we want to join them
	for (int i = 0; i < N_THREADS; ++i) {
		if (pthread_join(
					threads[i].thread,
					(void **) &(threads[i].result)
				) != 0) {
			return 1;
		}
	}

	printf("-------Final List-------\n");
	for (int i = 0; i < N_THREADS; ++i) {
		Thread thread = threads[i];
		printf("Thread %03d returned %03d\n", thread.id, *thread.result);
		free(thread.result);
	}

	return 0;
}

int globalRaceConditionVariable = 0;

#define NO_RACE_CONDITION_ITERATIONS 100
#define RACE_CONDITION_ITERATIONS 1000000

// NOTE: There is also some probability for race condition, but it less likelier
// when the number of iterations is bigger
void *noRaceCondition()
{
	for (int i = 0; i < NO_RACE_CONDITION_ITERATIONS; ++i) {
		globalRaceConditionVariable++;
	}
	pthread_exit(NULL);
}

void *raceCondition()
{
	for (int i = 0; i < RACE_CONDITION_ITERATIONS; ++i) {
		globalRaceConditionVariable++;
	}
	pthread_exit(NULL);
}

int raceConditionExample(void)
{
	Thread threads[N_THREADS] = {0};

	for (int i = 0; i < N_THREADS; ++i) {
		threads[i].id = i + 1;
		if (pthread_create(
					&threads[i].thread,
					NULL,
					noRaceCondition,
					&threads[i].id
				  ) != 0) {
			return 1;
		}
	}

	for (int i = 0; i < N_THREADS; ++i) {
		if (pthread_join(
					threads[i].thread,
					NULL
				) != 0) {
			return 1;
		}
	}

	printf("Global Race Condition Variable:\n");
	printf(
			"Iterations %d: %d, Expected: %d\n",
			NO_RACE_CONDITION_ITERATIONS,
			globalRaceConditionVariable,
			globalRaceConditionVariable == (N_THREADS * NO_RACE_CONDITION_ITERATIONS)
	      );

	globalRaceConditionVariable = 0;

	for (int i = 0; i < N_THREADS; ++i) {
		threads[i].id = i + 1;
		if (pthread_create(
					&threads[i].thread,
					NULL,
					raceCondition,
					&threads[i].id
				  ) != 0) {
			return 1;
		}
	}

	for (int i = 0; i < N_THREADS; ++i) {
		if (pthread_join(
					threads[i].thread,
					NULL
				) != 0) {
			return 1;
		}
	}

	printf(
			"Iterations %d: %d, Expected: %d\n",
			RACE_CONDITION_ITERATIONS,
			globalRaceConditionVariable,
			globalRaceConditionVariable == (N_THREADS * RACE_CONDITION_ITERATIONS)
	      );


	return 0;
}

#define FUNCTION 8

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
	createOrphanProcess();
#elif FUNCTION == 7
	return simpleThreads();
#elif FUNCTION == 8
	return raceConditionExample();
#endif

	return 0;
}
