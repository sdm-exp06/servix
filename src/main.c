// <>
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include "utils.h"
#include "http_handler.h"
#include "http_parser.h"

#define BUF_SIZE 500
#define SERVICE "50002"

#define BACKLOG 5
extern int h_errno;

// #define TABLE_SIZE 60
static volatile int worker = 0;
void zombieProcessHandler(int sig)
{
    (void)sig;
    while ((waitpid(-1, NULL, WNOHANG)) > 0)
    {
           printf("executing zeombie process handler: worker: %d\n", worker);
           worker--;
    }
}

int main(void)
{

    struct http_request_header *http_request_table[TABLE_SIZE];

    int http_response_status;
    struct winsize ws;
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == -1)
    {
        fprintf(stderr, "ioctl: %s\n", strerror(errno));
    }

    // printf("%d:%d\n", ws.ws_row, ws.ws_col);
    struct HTTP_REQUEST_STATUSLINE http_request_statusline;

    int sfd, s, cfd, optval;
    socklen_t len;
    // struct HTTP_REQUEST_STATUSLINE http_request_statusline;

    struct sockaddr_storage client_addr;
    struct addrinfo hints;
    struct addrinfo *result, *rp;

    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    char request_buf[REQUEST_BUF_SIZE];

    hints.ai_next = NULL;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
    hints.ai_protocol = 0;

    s = getaddrinfo(NULL, SERVICE, &hints, &result);
    if (s == -1)
    {
        fprintf(stderr, "\033[1;4;31;0m%s\n", gai_strerror(h_errno));
        exit(EXIT_FAILURE);
    }

    rp = result;

    optval = 1;

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;
        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        {
            fprintf(stderr, "\033[1;4;31;0m%s", strerror(errno));
        }

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
        {
            s = getnameinfo(rp->ai_addr, rp->ai_addrlen, host, NI_MAXHOST, service, NI_MAXSERV, 0);
            if (s != 0)
            {
                fprintf(stderr, "\033[1;4;31;0mgetnameinfo: %s\n", gai_strerror(h_errno));
            }
            break;
        }

        close(sfd);
        fprintf(stderr, "\033[1;4;31;0mbind: %s\n", strerror(errno));
    }

    if (rp == NULL)
    {
        fprintf(stderr, "\033[1;4;31;0munable to get an address\n");
        freeaddrinfo(result);
        exit(EXIT_FAILURE);
    }

    if (listen(sfd, BACKLOG) == -1)
    {
        fprintf(stderr, "\033[1;4;31;0mlisten: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // inform the user

    printf("listening on: \033[1;32;4m%s:\033[1;34;4m%s\033[0m\n", host, service);

    // concurrent function that handle one connection at a time
    int numConnection = 0;
    sigset_t blocked_set, empty_set;
    struct sigaction sa;

    
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = zombieProcessHandler;

    if(sigaction(SIGCLD, &sa, NULL) == -1){
        fprintf(stderr, "sigaction: %s\n", strerror(errno));
        exit(0);
    }
    sigemptyset(&empty_set);
    sigemptyset(&blocked_set);
    sigaddset(&blocked_set, SIGCLD);

    
    for (;;)
    {
        len = sizeof(struct sockaddr_storage);
        cfd = accept(sfd, (struct sockaddr *)&client_addr, &len);
        if (cfd == -1)
        {
            fprintf(stderr, "\033[1;4;31;0mfailed while accepting connexion\n");
            continue;
        }

        s = getnameinfo((struct sockaddr *)&client_addr, len, host, NI_MAXHOST, service, NI_MAXSERV, 0);
        if (s != 0)
        {
            fprintf(stderr, "\033[1;4;31;0mgetnameinfo: %s\n", gai_strerror(h_errno));
        }

        printf("accepting connection from \033[1;32;4m%s:\033[1;34;4m%s\033[0m\n", host, service);

        // craete a separate process to handle connected file descriptor
        if(sigprocmask(SIG_BLOCK, &empty_set, NULL) == -1){
            fprintf(stderr, "sigprocmask: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        print_to_center("FORKING\n", &ws);
        switch (fork())
        {
        case -1:
            fprintf(stderr, "FORK: (%s)\n", strerror(errno));
            exit(EXIT_FAILURE);
            break;
        case 0:
            for (;;)
            {
                memset(request_buf, 0, REQUEST_BUF_SIZE);
                errno = 0;
                ssize_t bytesRead;
                printf("startwars\n");

                bytesRead = read(cfd, request_buf, REQUEST_BUF_SIZE);

                printf("end end end end end\n");
                if (bytesRead <= 0)
                {
                    printf("read: %ld\n", bytesRead);
                    close(cfd);
                    exit(0);
                    break;
                }

                print_to_center("REQUEST", &ws);

                printf("%s\n", request_buf);
                http_response_status = handle_response(cfd, &http_request_statusline, request_buf, http_request_table);
                printf("sssssssss\n");
                printf("code = %dd\n", http_response_status);
                if (http_response_status == -1)
                {
                    close(cfd);
                    exit(0);
                    break;
                }
            }

            close(cfd);
            
            break;
        default:
        // DEBLOQUE 
            close(cfd);
            break;
        }

        // parent SI IL N A PAS TERMINER IL VA DEVENIR ZOMBIE DONC LORSQUE UNE ATREU CONNECTION TERMINER IL VA LE TUER AVEC WAITPID
        worker++;
        // if(sigprocmask(SIG_BLOCK, &empty_set, NULL) == -1){
        //     fprintf(stderr, "sigprocmask: %s\n", strerror(errno));
        //     exit(EXIT_FAILURE);
        // }

        printf("WORKER: %d\n", worker);
    }

    int sigCnt = 0;
    while (worker > 0)
    {
        if(sigsuspend(&empty_set) == -1 && errno == EINTR){
            fprintf(stderr, "sigsuspend: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        sigCnt++;
    }
    
    printf("killed %d zombies process\n", sigCnt);
    return 0;
}