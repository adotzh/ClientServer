#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define BUFSIZE 4096

#define FIFO_PATH "fifo.file"

int fifo_client, fifo_server, READ_FILE;

char** split(char * smth_to_split);

void new_exit(int err);

void stopsignal(int);

int main() {
        int child, alive = 1;
        char * filename;
        char * pid;
        char ** temp;
        int rd, wr;
        char * buf = (char *) malloc (BUFSIZE + 1);
    
        umask(0);
    
        if(mknod(FIFO_PATH, 0666|S_IFIFO, 0) < 0) { //создаем fifo сервера
            perror("Error creating client FIFO");
            new_exit(errno);
        }
    
        if((fifo_server = open(FIFO_PATH, O_RDONLY)) < 0) {
                perror("Error opening server FIFO");
                exit(1);
        }
    
        signal(SIGINT, stopsignal);
    
        while(alive) {
                memset(buf, '\0', BUFSIZE);
                if((rd = read(fifo_server, buf, BUFSIZE + 1)) < 0) {
                        perror("Error reading from server FIFO");
                        new_exit(errno);
                }
                if (rd > 0) {
                        if ((child = fork()) < 0) {
                                perror("Error at fork");
                        }
                        if (child == 0) {
                                printf("CHILD\n");
                                alive = 0;
                                temp = split(buf);
                                pid = temp[0];
                                filename = temp[1];
                
                                strcat(pid, ".file");
                                printf("FILENAME: %s\n", filename);
                                printf("Client FIFO name: %s\n", pid);
                                printf("------------------------------------\n");
                            
                                if((fifo_client = open(pid, O_WRONLY)) < 0) {
                                    //открываем fifo клиента для записи
                                        perror("Error opening client FIFO");
                                        new_exit(errno);
                                }
                            
                                if((READ_FILE = open(filename, O_RDONLY, 0666)) < 0) { //открываем fifo сервера
                                        perror("Error opening FILE");
                                        new_exit(errno);
                                }
                            
                                while(1) {
                                        if((rd = read(READ_FILE, buf, BUFSIZE + 1)) < 0){   // читаем из файла
                                                perror("Error reading FILE");
                                                new_exit(errno);
                                        }
                                    
                                        if(rd == 0)
                                                break;
                                    
                                        if((wr = write(fifo_client, buf, rd)) < 0){ // записываем в буффер
                                                perror("Error writing to FIFO");
                                                new_exit(errno);
                                        }
                                }
                                close(fifo_client);
                                close(READ_FILE);
                        }
                }
        }
        return 0;
}



char** split(char * smth_to_split) {
        int flag = 0;
        static char * to_return[2];
        int i;
        int second_cnt = 0;
        int len = strlen(smth_to_split);
        char * first = (char *) malloc (len);
        char * second = (char *) malloc (len);
        for (i = 0; i < len; i++) {
                if (smth_to_split[i] == ' ') {
                        flag = -1;
                        first[i] = '\0';
                }
                if (flag == 0) {
                        first[i] = smth_to_split[i];
                }
                if (flag == 1) {
                        second[second_cnt++] = smth_to_split[i];
                }
                if (smth_to_split[i] == ' ')
                        flag = 1;
        }
    second[second_cnt] = '\0';
    to_return[0] = first;
    to_return[1] = second;
    return to_return;
}

void stopsignal(int sig){
    char c;
    if(sig != SIGINT)
        return;
    else {
        printf("\nStop the server [y/n] : ");
        while((c=getchar()) != 'y')
            return;
        new_exit(1);
    }
};

void new_exit(int err) {
    close(fifo_server);
    close(fifo_client);
    close(READ_FILE);
    remove(FIFO_PATH);
    exit(err);
}
