#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

#include "utils.h"

void print_to_center(const char* string, struct winsize *ws){
    int str_len = strlen(string);

    for(int i = 0; i < (ws->ws_col - str_len) / 2; i++){
        putc('-', stdout);
    }
    printf("\033[1;4;35m%s\033[0m", string);

    for(int i = 0; i < (ws->ws_col - str_len) / 2; i++){
        putc('-', stdout);
    }

}
ssize_t compute_filesize(int fd){
    ssize_t fileSize = 0;
    ssize_t numRead = 0;

    #define BUF_SIZE 500
    char buf[BUF_SIZE];
    
    while((numRead = read(fd, buf, BUF_SIZE))  > 0 ){
            fileSize+=numRead;
    }
    
    return (numRead != -1) ? fileSize : -1;
}
