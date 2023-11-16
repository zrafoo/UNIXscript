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
    printf("Client started\n");
    system("touch /tmp/lab4");
    key_t ipckey = ftok("/tmp/lab4", PROJECT_ID);

    int mq_id;
    if ((mq_id = msgget(ipckey, IPC_EXCL | IPC_CREAT | 0666)) == -1)
        if ((mq_id = msgget(ipckey, 0)) == -1)
        {
            printf("Can't create/open message queue\n");
            exit(1);
        }

    printf("[INFO] Message queue identifier is %d\n", mq_id);

    FILE *proc = popen("grep -I -rnl '.' -e 'define' | cut -f1 -d':' | cut -c3- | xargs", "r");
    char buf[256];
    size_t read_chars;
    size_t current_lenght = 256;
    size_t result_lenght = 0;
    char  *files   = malloc(current_lenght);
    while ((read_chars = fread(buf, 1, sizeof(buf), proc)) != 0) {
        if (result_lenght + read_chars >= result_lenght) {
            current_lenght *= 2;
            files = realloc(files, current_lenght);
        }
        memmove(files + result_lenght, buf, read_chars);
        result_lenght += read_chars;
    }
    pclose(proc);
    
    struct {
        long type;
        char message[1024];
    } repo;
    getcwd(repo.message,sizeof(repo.message)); //current working directory
    repo.type = 1;
    
    printf("[INFO] Sending message with directory: %s [size=%lu]\n", repo.message, strlen(repo.message));
    msgsnd(mq_id, &repo, strlen(repo.message), 0);


    struct {
        long type;
        char message[strlen(files)];
    } files_define;
    files[strlen(files)-1] = 0; // delete \n in the end
    files_define.type = 1;
    strcpy(&files_define.message, files);

    printf("[INFO] Sending message with files: %s [size=%lu]\n", files_define.message, strlen(files_define.message));
    msgsnd(mq_id, &files_define, strlen(files_define.message), 0);

    free(files);
    printf("Done.\n");
    return 0;
}
