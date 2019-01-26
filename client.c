#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define BUFSIZE 4096

char pid_str[50];
char pid_filename[50];

int fifo_client, fifo_server;

void new_exit(int err);

int main(int argc, char ** argv) {
        char * buf = (char *) malloc (BUFSIZE + 1);
        char * filename = argv[1];
        int fifo_server, fifo_client;
        int rd, wr;
        char pid_str[100];
        char pid_filename[100];
        int pid = getpid();
    
        sprintf(pid_str, "%d", pid);
        sprintf(pid_filename, "%d", pid);
    
        strcat(pid_filename, " ");
        strcat(pid_filename, filename); //создаем строку, которую передаем в fifo
    
        strcat(pid_str, ".file"); //создаем имя для fifo клиента
    
        if((fifo_server = open("fifo.file", O_WRONLY, 0666)) < 0) { //открываем fifo сервера
                perror("Error opening server FIFO");
                new_exit(errno);
        }
        if((write(fifo_server, pid_filename, strlen(pid_filename))) < 0){
                perror("Error writing to server FIFO");
                new_exit(errno);
        }
         //записываем на сервер
        if(mknod(pid_str, 0666|S_IFIFO, 0) < 0) { //создаем fifo клиента
                perror("Error creating client FIFO");
                new_exit(errno);
        }
        if((fifo_client = open(pid_str, O_RDONLY)) < 0) {//открываем fifo клиента для чтения
                perror("Error opening client FIFO");
                new_exit(errno);
        }
        do {
                if((rd = read(fifo_client, buf, BUFSIZE + 1)) < 0){
                    perror("Error reading from client FIFO");
                }
                if(rd == 0)
                        break;
                sleep(1);
                if((wr = write(1, buf, rd)) < 0){
                        perror("Error writing to output");
                }
        } while(1);
        close(fifo_client);
        close(fifo_server);
        remove(pid_str);
        return 0;
}


void new_exit(int err) {
        close(fifo_server);
        close(fifo_client);
        remove(pid_str);
        exit(err);
}
