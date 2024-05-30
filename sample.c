#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#define ERR(source)(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

void usage(char *progname)
{
    fprintf(stderr,"USAGE: %s 2<=N<=5 5<=M<=10\n",progname);
    exit(EXIT_FAILURE);
}

void wait_children()
{
    while(1)
    {
        pid_t pid;
        pid = waitpid(0,NULL,WNOHANG);
        if(pid == 0)
            break;
        if(pid < 0)
        {
            if(errno == ECHILD) break;
            ERR("waitpid");
        }
    }
}

void child_work(int readfd,int writefd,int m)
{
    srand(getpid());
    while(1)
    {
        char buf[16];
        if(read(readfd,&buf,16) == -1)
            ERR("read");
        if(strcmp(buf,"new_round") != 0) break;
        int n = 1 + rand() % m;
        if(write(writefd,&n,sizeof(int)) == -1)
            ERR("write");
    }
    if(close(readfd) == -1 || close(writefd) == -1)
        ERR("close");
}

void parent_work(int **pipefd,int n,int m)
{
    printf("parent\n");
    for(int i=0;i<n;i++)
    {
        if(close(pipefd[2*i][0]) == -1 || close(pipefd[2*i+1][1]))
            ERR("close");
    }
    for(int j=0;j<m;j++)
    {
        char buf[16];
        memset(&buf,0,16);
        sprintf(buf,"new_round");
        printf("NEW ROUND\n");
        for(int i=0;i<n;i++)
        {
            if(write(pipefd[2*i][1],&buf,16) == -1)
                ERR("write");
        }
        for(int i=0;i<n;i++)
        {
            int buf;
            if(read(pipefd[2*i+1][0],&buf,sizeof(int)) == -1)
                ERR("read");
            printf("Got number %d from player %d\n",buf,i);
        }
        sleep(1);
    }
    for(int i=0;i<n;i++)
    {
        if(close(pipefd[2*i][1]) == -1 || close(pipefd[2*i+1][0]))
            ERR("close");
    }
    wait_children();
}

void create_n_children_and_pipes(int n,int m)
{
    int **pipefd;
    pipefd = (int**)malloc(2*n*sizeof(int*));
    for(int i=0;i<2*n;i++)
    {
        pipefd[i] = (int*)malloc(2*sizeof(int));
        if(pipe(pipefd[i]) == -1)
            ERR("pipe");
    }
    for(int i=0;i<n;i++)
    {
        pid_t pid = fork();
        switch (pid)
        {
            case 0:
                for(int j=0;j<n;j++)
                {
                    if(j != 2*i && j != 2*i+1)
                        if(close(pipefd[j][0]) == -1 || close(pipefd[j][1]))
                            ERR("close");
                }
                if(close(pipefd[2*i][1]) == -1 || close(pipefd[2*i+1][0]))
                    ERR("close");
                child_work(pipefd[2*i][0],pipefd[2*i+1][1],m);
                exit(EXIT_SUCCESS);
            case -1:
                ERR("fork");
            default:
                break;
        }
    }
    parent_work(pipefd,n,m);
}

int main(int argc,char **argv)
{
    int n,m;
    if(argc != 3) usage(argv[0]);
    n=atoi(argv[1]);
    m=atoi(argv[2]);
    if(n < 2 || n > 6 || m < 5 || m > 10) usage(argv[0]);
    create_n_children_and_pipes(n,m);
    return EXIT_SUCCESS;
}