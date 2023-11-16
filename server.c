#define _GNU_SOURCE

#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PROJECT_ID 20

int main()
{

    printf("Server started\n");

    key_t ipckey;

    while(1){
        ipckey = ftok("/tmp/lab4", PROJECT_ID);
        if(ipckey == -1){
            printf("[WARNING] There is no any messages here\n");
            sleep(1);
        } else 
            break;
    }

    int mq_id = msgget(ipckey, 0);//get message queue id
    if (mq_id == -1)
    {
        printf("[ERROR] Can't open message queue\n");
        exit(1);
    }
    printf("[INFO] Message queue ID: %d\n", mq_id);

    struct msqid_ds ds;
    msgctl(mq_id, IPC_STAT, &ds);
    printf("[INFO] Messages in queue: %lu\n", ds.msg_qnum);
    printf("[INFO] Max bytes: %lu \n", ds.msg_qbytes);
    printf("[INFO] Last send PID: %d\n", ds.msg_lspid);

    struct 
    { 
        long type; 
        char text[ds.msg_qbytes]; 
    } repo;

    int st = msgrcv(mq_id, &repo, sizeof(repo.text), -1, 0);
    printf("\n[INFO] Bytes read: %d\n", st);

    if (st > 0){
        printf("Message: %s [size=%lu]\n", repo.text, strlen(repo.text));
    }
    
    struct 
    { 
        long type; 
        char text[ds.msg_qbytes]; 
    } files;
    
    st = msgrcv(mq_id, &files, sizeof(files.text), -1, 0);
    printf("\n[INFO] Bytes read = %d\n", st);

    if (st > 0){
        printf("Message: %s [size=%lu]\n", files.text, strlen(files.text));
    }

    char *command;
    int s = asprintf(&command, "ls -lt --time=birth %s", files.text);
    if (s < 0)
    {
        printf("[ERROR] Can't print command");
    }

    if (fork() == 0)       // execute action in parallel process
    {
        // child process
        printf("\nSorted files\n");
        execl("/bin/sh", "sh", "-c", command, NULL);
    } else {
        // wait until child process die
        wait(0);
    }

    msgctl(mq_id, IPC_RMID, NULL);
    printf("\nMessages queue deleted!\n");

}
