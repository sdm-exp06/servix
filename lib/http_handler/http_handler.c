#include "http_handler.h"
#include "http_parser.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>

#define STATUS_LINE_BUFFER_SIZE 512
#define HTTP_RESPONSE_HEADER_BUFFER_SIZE 4096

enum fileType{
    text, 
    image,
    video,
    audio,
    unknown
};

static int write_http_response_header(int cfd, struct stat* fileProperties, char *path){
    enum fileType file_type;
    char date_format[128];
    time_t current_time;
    time(&current_time);
    struct tm* utc = gmtime(&current_time);
    // <day-name>, <day> <month> <year> <hour>:<minute>:<second> GMT
    strftime(date_format, sizeof date_format, "%a, %d %b %Y %H:%M:%S GMT", utc);

    char http_response_header_buffer[HTTP_RESPONSE_HEADER_BUFFER_SIZE];
    char *filetype_pointer = strstr(path, ".");
    filetype_pointer++;
    
    char *file_type_str = filetype_pointer;
    if(filetype_pointer == NULL)
        return -1;
    printf("filetype: %s\n", file_type_str);
    
    printf("start\n");
    file_type = unknown;
    if(strcmp(file_type_str, "html") == 0 || strcmp(file_type_str, "css") == 0 || strcmp(file_type_str, "js") == 0)
        file_type = text;

    else if(strcmp(file_type_str, "png") == 0 || strcmp(file_type_str, "jpg") == 0 || strcmp(file_type_str, "jpeg") == 0 
    || strcmp(file_type_str, "ico") ==0 || strcmp(file_type_str, "webp") ==0 || strcmp(file_type_str, "avif") == 0)
        file_type = image;
    else if(strcmp(file_type_str, "mp4") ==0 || strcmp(file_type_str, "webm") == 0 || strcmp(file_type_str, "ogg") == 0){
        file_type = video;
    }
    else if(strcmp(file_type_str, "mp3") == 0 || strcmp(file_type_str, "wav") == 0 || strcmp(file_type_str, "ogg") == 0 || strcmp(file_type_str, "aac") == 0)
        file_type = audio;
    
      if(file_type == unknown)
        return 0;
    
    snprintf(http_response_header_buffer, HTTP_RESPONSE_HEADER_BUFFER_SIZE, 
        "Content-Type: %s/%s; charset=utf-8\r\n"
        "Content-Length: %ld\r\n"
        "Date: %s\r\n"
        "Connection: keep-alive\r\n\r\n",
        (file_type == text ? "text" : file_type == image ? "image" : file_type == video ? "video" : "audio"),
         strcmp(file_type_str, "mp3") == 0 ? "mpeg" : file_type_str,fileProperties->st_size,
            date_format
        );

    printf("type = %d/%s\n", file_type, file_type_str);

    if(write(cfd, http_response_header_buffer, strlen(http_response_header_buffer)) != (ssize_t)strlen(http_response_header_buffer)){
        fprintf(stderr, "write: %s\n", strerror(errno));
        return -1;
    }
    printf("size: %ld\n", fileProperties->st_size);
    
    return 0;
}

int write_http_response_body(int cfd, struct stat* fileProperties, char *path){
    printf("startstart\n");
    char *response_body = malloc(fileProperties->st_size);
    int file_fd = open(path, O_RDONLY);
    if(file_fd == -1){
        fprintf(stderr, "open: %s\n", strerror(errno));
        return -1;
    }
    printf("end end\n");
    printf("SIZE: %ld\n", fileProperties->st_size);
    
    if((read(file_fd, response_body, fileProperties->st_size)) == -1)
    {
        fprintf(stderr, "readING: %s\n", strerror(errno));
        return -1;
    }

    if(write(cfd, response_body, fileProperties->st_size) == -1){
        fprintf(stderr, "write: %s\n", strerror(errno));
        return -1;
    }
    printf("written\n");
    return 0;
}

int handle_response(int cfd, struct HTTP_REQUEST_STATUSLINE* http_request_statusline, char *request_str, struct http_request_header* http_request_table[])
{
    int parser_status = http_parser(request_str, http_request_statusline, http_request_table);

    if(parser_status != 0){
        return -1;
    }

    // if(strcmp(http_request_statusline->version, "HTTP\1.1") != 0){
    //     fprintf(stderr, "Unsupported http version\n");
    //     return -1;
    // }

    if(strcmp(http_request_statusline->method, "GET") != 0){
        fprintf(stderr, "unsupported http request method please use <GET>\n");
        return -1;
    }
    int handle_get_status = http_get_handler(cfd, strcmp(http_request_statusline->path, "/") == 0 ? "/index.html" : http_request_statusline->path);
    
    if(handle_get_status == -1){
        return -1;
    }
   
    return 0;
}

int http_get_handler(int cfd, char* request_path){
   // #define BUF_SIZE 128
    char client_response[REQUEST_BUF_SIZE];
    //char client_response_buf[BUF_SIZE];
    //ssize_t client_response_bytes_read;


    struct stat statBuf;
    char status_message[256];
    enum status_code {
        OK = 200,
        Not_Found = 404
    };
    enum status_code code;
    ssize_t fileSize;

    (void)cfd;
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "www%s", request_path);
   // printf("path = %s\n", path);
   
   printf("PATH = %s\n", path);
   int file_fd = open(path, O_RDONLY);
    if(file_fd == -1){
        fprintf(stderr, "open: %s\n", strerror(errno));
        file_fd = open("www/404.html", O_RDONLY);
        code = Not_Found;
    }else 
        code = OK;
    
   if((stat(!(code == Not_Found) ? path : "www/404.html", &statBuf) == -1) && code != Not_Found){
        fprintf(stderr, "stat: %s\n", strerror(errno));
        return -1;
    }

//    printf("file_siz: %ld\n", fileSize);

    
    // char response_body[fileSize+1];

    // if(!(read(file_fd, response_body, fileSize) > 0)){
    //     fprintf(stderr, "read: %s\n", strerror(errno));
    //     return -1;
    // }

    // response_body[fileSize] = '\0';

    // // everyting is OK
    
    switch (code)
    {
    case Not_Found:
        strncpy(status_message, "Not Found", 256);
        break;
    case OK:
        strncpy(status_message, "OK", 256);
        break;
    }

    
    char status_line[STATUS_LINE_BUFFER_SIZE];

    snprintf(status_line, STATUS_LINE_BUFFER_SIZE, "HTTP\\1.1 %d %s\r\n", code, status_message);
    
    printf("%s\n", status_line);

    ssize_t status_line_len = strlen(status_line);

    if(write(cfd, status_line, status_line_len) != status_line_len){
        fprintf(stderr, "write: %s\n", strerror(errno));
        return -1;
    }

    if(write_http_response_header(cfd, &statBuf, !(code == Not_Found) ? path : "www/404.html") == -1)
        return -1;
    if(write_http_response_body(cfd, &statBuf, !(code == Not_Found) ? path : "www/404.html") == -1)
        return -1;
    return 0;
}
