#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H
#include <stdio.h>
#include <sys/types.h>
#include "http_parser.h"

int handle_response(int cfd, struct HTTP_REQUEST_STATUSLINE* http_request_statusline, char *request_str, struct http_request_header* http_request_table[]);
int http_get_handler(int cfd, char* request_path);

#endif