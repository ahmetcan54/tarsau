#ifndef UTILS_H
#define UTILS_H

#include <sys/stat.h>

int         is_text_file(const char *path);
void        perms_to_str(mode_t mode, char *buf);   /* buf must be >= 10 bytes */
mode_t      str_to_perms(const char *str);
void        die(const char *msg);
const char *get_basename(const char *path);

#endif
