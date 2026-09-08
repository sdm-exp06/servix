#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H
#include <linux/limits.h>

#define METHOD_SIZE 36
#define VERSION_SIZE 16
#define REQUEST_BUF_SIZE 8192

struct HTTP_REQUEST_STATUSLINE{
    char method[METHOD_SIZE];
    char path[PATH_MAX];
    char version[VERSION_SIZE];
};
struct http_request_header{
    char name[256];
    char value[256];
};

#define TABLE_SIZE 60

int http_parser(const char *request_str, struct HTTP_REQUEST_STATUSLINE* http_request, struct http_request_header* http_request_table[]);
#endif