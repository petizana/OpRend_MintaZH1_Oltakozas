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

void handler(int signumber)
{
    printf("Signal with number %i has arrived\n", signumber);
}

int main(int argc, char **argv)
{
    int status;
    signal(SIGTERM, handler);
    int pipefd[2];
    int p;

    if (pipe(pipefd) == -1)
    {
        perror("Hiba a pipe nyitaskor!");
        exit(EXIT_FAILURE);
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
            write(pipefd[1], &patientsForUrsula, sizeof(int)); // this one is for Ursula
            write(pipefd[1], &patientsForCsormester, sizeof(int)); // this one is for Csőrmester
            close(pipefd[1]);
            printf("Patients number has been sent to children!");
            fflush(NULL);
            wait(NULL);
        }
        else // csormester process
        {
            printf("Csőrmester waits 3 seconds, then send a SIGTERM %i signal\n", SIGTERM);
            sleep(3);
            kill(getppid(), SIGTERM);

            close(pipefd[1]);
            printf("\nReading started\n");
            read(pipefd[0], &p, sizeof(int));
            printf("\nCsőrmester read the msg: %d", p);
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
        printf("\n");
        close(pipefd[0]);
    }
    return 0;
}

// 2: 56 perc maradt