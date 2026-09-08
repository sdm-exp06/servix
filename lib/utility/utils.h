#ifndef UTILS_H
#define UTILS_H
#include <sys/ioctl.h>
void print_to_center(const char* string, struct winsize *ws);
ssize_t compute_filesize(int cfd);
#endif