#include "http_parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


void init_table(struct http_request_header* http_request_table[]){
    for(int i = 0; i < TABLE_SIZE; i++){
        http_request_table[i] = NULL;
    }
}

int hash(const char* field_name){
    int len = strlen(field_name);
    int hash_value = 0;
    for(int i = 0; i < len; i++){
        hash_value+=field_name[i];
        hash_value = (hash_value + field_name[i]) % TABLE_SIZE;
    }
    return hash_value;
}

bool insert_to_table(struct http_request_header* field, struct http_request_header* http_request_table[]){
    if(field == NULL) return false;
    int index = hash(field->name);

    for(int i =0; i < TABLE_SIZE; i++){
        int try = (i + index) % TABLE_SIZE;
        if(http_request_table[try] == NULL){
            http_request_table[try] = field;
            return true;
        }
    }
    return false;
}


struct http_request_header* table_lookup(const char* field_name, struct http_request_header* http_request_table[]){
    int index = hash(field_name);
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        int try = (i + index ) % TABLE_SIZE;
        if(http_request_table[try] != NULL && strncmp(http_request_table[try]->name, field_name, TABLE_SIZE) == 0){
            return http_request_table[try];
        }
    }

    return NULL;
}


int http_parser(const char *request_str, struct HTTP_REQUEST_STATUSLINE* http_request, struct http_request_header* http_request_table[]){
    // printf("REQUESTING: %s\n", request_str);
    (void)http_request;
    const char* end_startline = strstr(request_str, "\r\n");
    if(!end_startline){
        fprintf(stderr, "strstr(): error while parsing");
        return -1;
    }

    // GET / HTTP/1.1\r
    int starline_len = end_startline - request_str;
    
    char starline_string[starline_len + 1];

    memcpy(starline_string, request_str, starline_len);

    starline_string[starline_len] = '\0';

    // printf("start line : %s\n", starline_string);
    sscanf(starline_string, "%s %s %s", http_request->method, http_request->path, http_request->version);
    
    init_table(http_request_table);

    const char* header_line_begin = end_startline + 2;
    // printf("head: %s\n", header_line_begin);
    const char * header_line_end;
    int header_line_len;

    header_line_len = 0;

    // printf("start\n");
    
    while ((header_line_end = strstr(header_line_begin, "\r\n")) != NULL)
    {
        
        header_line_len = header_line_end - header_line_begin + 1;

        if(header_line_len == 1)
            break;
        struct http_request_header *http_request_header = malloc(sizeof(struct http_request_header));
        
        char header_line_str[header_line_len + 1];

        // printf("len: %d\n", header_line_len);
        
        memmove(header_line_str, header_line_begin, header_line_len);
        header_line_str[header_line_len] = '\0';

        sscanf(header_line_str, "%s %s", http_request_header->name, http_request_header->value);
        http_request_header->name[strlen(http_request_header->name) - 1] = '\0';
        
        if(!insert_to_table(http_request_header, http_request_table)){
            fprintf(stderr, "error while inserting to hash table");
        }
        
        // printf("%s\n", header_line_str);
        
        //printf("%s = %s\n", http_request_header->name, http_request_header->value);
        
        header_line_begin = header_line_end + 2;
        

        // printf("%s = %s\n", http_request_header.name, http_request_header.value);
    }  
   // printf("end\n");

    // for(int i = 0; i < TABLE_SIZE; i++){
    //     if(hash_request_table[i] != NULL){
    //         printf("%s=%s\n", hash_request_table[i]->name, hash_request_table[i]->value);
    //     }
    // }

    // struct http_request_header* req = table_lookup("Host");
    
    // printf("H ==== %s\n", req->value);


    return 0;
}