#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   //fork
#include <sys/wait.h> //waitpid
#include <errno.h>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for pipe()
#include <string.h>
#include <sys/wait.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

void handler(int signumber)
{
    printf("Signal with number %i has arrived\n", signumber);
}

struct msg
{
    long mtype;
    int sicks;
    int successfullyVaccinated;
};

int send(int msgqueue, int sicks, int successfullyVaccinated)
{
    const struct msg msg = {5, sicks, successfullyVaccinated};
    int status;

    status = msgsnd(msgqueue, &msg, sizeof(int) * 2, 0);
    if (status < 0)
        perror("msgsnd");
    return 0;
}

int receive(int msgqueue)
{
    struct msg msg;
    int status;
    status = msgrcv(msgqueue, &msg, sizeof(int) * 2, 5, 0);

    if (status < 0)
        perror("msgsnd");
    else
        printf("\nSicks:%d -> SUCCESSFULLY VACCINATED:%d\n", msg.sicks, msg.successfullyVaccinated);
    return 0;
}

int countSickPatients(int patients)
{
    int count = 0;
    srand(time(NULL));
    for (int i = 0; i < patients; i++)
    {
        int r = rand() % 100 + 1;
        if (r <= 20)
            count++;
    }
    return count;
}

int main(int argc, char **argv)
{
    int status;
    signal(SIGTERM, handler);
    int pipefd[2];
    int p;

    int msgqueue;
    key_t kulcs;

    if (pipe(pipefd) == -1)
    {
        perror("Hiba a pipe nyitaskor!");
        exit(EXIT_FAILURE);
    }

    kulcs = ftok(argv[0], 1);
    msgqueue = msgget(kulcs, 0600 | IPC_CREAT);
    if (msgqueue < 0)
    {
        perror("msgget");
        return 1;
    }

    pid_t ursula = fork();
    if (ursula < 0)
    {
        perror("The fork calling was not succesful\n");
        exit(1);
    }

    if (ursula > 0)
    {
        pid_t csormester = fork();
        if (csormester < 0)
        {
            perror("The fork calling was not succesful\n");
            exit(1);
        }

        if (csormester > 0) // parent
        {
            pause();
            pause();
            // 2 signals received
            if (argc != 2)
            {
                printf("You need to have 2 args!");
                exit(-1);
            }
            int patients = atoi(argv[1]);
            printf("Currently there are %d patients waiting in queue!", patients);
            int patientsForUrsula = patients / 2;
            int patientsForCsormester;
            if (patients % 2 == 1)
                patientsForCsormester = patients / 2 + 1;
            else
                patientsForCsormester = patients / 2;
            close(pipefd[0]);
            write(pipefd[1], &patientsForUrsula, sizeof(int));     // this one is for Ursula
            write(pipefd[1], &patientsForCsormester, sizeof(int)); // this one is for Csőrmester
            close(pipefd[1]);
            printf("\nPatients number has been sent to children!\n");
            fflush(NULL);
            wait(NULL);
            receive(msgqueue);
            receive(msgqueue);
        }
        else // csormester process
        {
            printf("\nCsőrmester waits 3 seconds, then send a SIGTERM %i signal\n", SIGTERM);
            sleep(3);
            kill(getppid(), SIGTERM);

            close(pipefd[1]);
            printf("\nReading started\n");
            read(pipefd[0], &p, sizeof(int));
            printf("\nCsőrmester read the msg: %d", p);
            int sicks = countSickPatients(p);
            send(msgqueue, sicks, (p - sicks));
            wait(NULL);
            printf("\n");
            close(pipefd[0]);
        }
    }
    else // ursula process
    {
        printf("Ursula waits 3 seconds, then send a SIGTERM %i signal\n", SIGTERM);
        sleep(3);
        kill(getppid(), SIGTERM);

        close(pipefd[1]);
        printf("\nReading started\n");
        read(pipefd[0], &p, sizeof(int));
        printf("\nUrsula read the msg: %d", p);
        int sicks = countSickPatients(p);
        send(msgqueue, sicks, (p - sicks));
        wait(NULL);
        printf("\n");
        close(pipefd[0]);
    }
    return 0;
}

// 2: 56 perc maradt
// 3: 4 perc maradt