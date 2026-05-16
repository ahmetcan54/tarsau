#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int is_text_file(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return 0;
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        if (ch > 127 || (ch < 32 && ch != '\n' && ch != '\r' && ch != '\t')) {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    return 1;
}

void perms_to_str(mode_t mode, char *buf) {
    buf[0] = (mode & S_IRUSR) ? 'r' : '-';
    buf[1] = (mode & S_IWUSR) ? 'w' : '-';
    buf[2] = (mode & S_IXUSR) ? 'x' : '-';
    buf[3] = (mode & S_IRGRP) ? 'r' : '-';
    buf[4] = (mode & S_IWGRP) ? 'w' : '-';
    buf[5] = (mode & S_IXGRP) ? 'x' : '-';
    buf[6] = (mode & S_IROTH) ? 'r' : '-';
    buf[7] = (mode & S_IWOTH) ? 'w' : '-';
    buf[8] = (mode & S_IXOTH) ? 'x' : '-';
    buf[9] = '\0';
}

mode_t str_to_perms(const char *str) {
    mode_t mode = 0;
    if (str[0] == 'r') mode |= S_IRUSR;
    if (str[1] == 'w') mode |= S_IWUSR;
    if (str[2] == 'x') mode |= S_IXUSR;
    if (str[3] == 'r') mode |= S_IRGRP;
    if (str[4] == 'w') mode |= S_IWGRP;
    if (str[5] == 'x') mode |= S_IXGRP;
    if (str[6] == 'r') mode |= S_IROTH;
    if (str[7] == 'w') mode |= S_IWOTH;
    if (str[8] == 'x') mode |= S_IXOTH;
    return mode;
}

void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

const char *get_basename(const char *path) {
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}
