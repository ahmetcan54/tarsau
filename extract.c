#include "extract.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_FILES 32
#define MAX_NAME  512

/* Build a human-readable list: "a, b ve c" */
static void build_names_list(char *buf, size_t buf_size,
                              char names[][MAX_NAME], int count) {
    buf[0] = '\0';
    for (int i = 0; i < count; i++) {
        size_t remaining = buf_size - strlen(buf) - 1;
        if (i == 0) {
            strncat(buf, names[i], remaining);
        } else if (i == count - 1) {
            strncat(buf, " ve ", buf_size - strlen(buf) - 1);
            strncat(buf, names[i], buf_size - strlen(buf) - 1);
        } else {
            strncat(buf, ", ", remaining);
            strncat(buf, names[i], buf_size - strlen(buf) - 1);
        }
    }
}

void extract_archive(const char *archive, const char *target) {
    FILE *in = fopen(archive, "rb");
    if (!in) {
        fprintf(stderr, "Hata: %s arsiv dosyasi acilamadi!\n", archive);
        exit(1);
    }

    /* Read 10-byte org section size */
    char size_buf[11] = {0};
    if (fread(size_buf, 1, 10, in) != 10) {
        fclose(in);
        die("Arsiv dosyasi uygunsuz veya bozuk!");
    }
    for (int i = 0; i < 10; i++) {
        if (size_buf[i] < '0' || size_buf[i] > '9') {
            fclose(in);
            die("Arsiv dosyasi uygunsuz veya bozuk!");
        }
    }

    long org_size = atol(size_buf);
    if (org_size <= 0) {
        fclose(in);
        die("Arsiv dosyasi uygunsuz veya bozuk!");
    }

    /* Read org section */
    char *org = malloc((size_t)org_size + 1);
    if (!org) die("Hata: Bellek tahsisi basarisiz!");

    if ((long)fread(org, 1, (size_t)org_size, in) != org_size) {
        free(org);
        fclose(in);
        die("Arsiv dosyasi uygunsuz veya bozuk!");
    }
    org[org_size] = '\0';

    /* Format: |name,perms,size|...| */
    if (org[0] != '|' || org[org_size - 1] != '|') {
        free(org);
        fclose(in);
        die("Arsiv dosyasi uygunsuz veya bozuk!");
    }

    /* Parse org section with nested strtok_r */
    char names[MAX_FILES][MAX_NAME];
    char perms[MAX_FILES][10];
    long file_sizes[MAX_FILES];
    int  file_count = 0;

    char *outer_save, *inner_save;
    char *token = strtok_r(org, "|", &outer_save);
    while (token && file_count < MAX_FILES) {
        char *tok_copy = strdup(token);
        if (!tok_copy) die("Hata: Bellek tahsisi basarisiz!");

        char *name_s = strtok_r(tok_copy, ",", &inner_save);
        char *perm_s = strtok_r(NULL,     ",", &inner_save);
        char *size_s = strtok_r(NULL,     ",", &inner_save);

        if (!name_s || !perm_s || !size_s || strlen(perm_s) != 9) {
            free(tok_copy); free(org); fclose(in);
            die("Arsiv dosyasi uygunsuz veya bozuk!");
        }

        strncpy(names[file_count], name_s, MAX_NAME - 1);
        names[file_count][MAX_NAME - 1] = '\0';
        strncpy(perms[file_count], perm_s, 9);
        perms[file_count][9] = '\0';

        file_sizes[file_count] = atol(size_s);
        if (file_sizes[file_count] < 0) {
            free(tok_copy); free(org); fclose(in);
            die("Arsiv dosyasi uygunsuz veya bozuk!");
        }

        free(tok_copy);
        file_count++;
        token = strtok_r(NULL, "|", &outer_save);
    }
    free(org);

    if (file_count == 0) {
        fclose(in);
        die("Arsiv dosyasi uygunsuz veya bozuk!");
    }

    /* Create target directory if it doesn't exist */
    if (target) {
        struct stat st;
        if (stat(target, &st) != 0) {
            if (mkdir(target, 0755) != 0) {
                fclose(in);
                fprintf(stderr, "Hata: %s dizini olusturulamadi!\n", target);
                exit(1);
            }
        } else if (!S_ISDIR(st.st_mode)) {
            fclose(in);
            fprintf(stderr, "Hata: %s bir dizin degil!\n", target);
            exit(1);
        }
    }

    /* Extract each file */
    for (int i = 0; i < file_count; i++) {
        char path[MAX_NAME + 520];
        if (target)
            snprintf(path, sizeof(path), "%s/%s", target, names[i]);
        else
            snprintf(path, sizeof(path), "%s", names[i]);

        FILE *out = fopen(path, "wb");
        if (!out) {
            fclose(in);
            fprintf(stderr, "Hata: %s dosyasi olusturulamadi!\n", path);
            exit(1);
        }

        long remaining = file_sizes[i];
        char buf[8192];
        while (remaining > 0) {
            long to_read = (remaining < (long)sizeof(buf))
                           ? remaining : (long)sizeof(buf);
            long n = (long)fread(buf, 1, (size_t)to_read, in);
            if (n <= 0) {
                fclose(out); fclose(in);
                die("Arsiv dosyasi uygunsuz veya bozuk!");
            }
            fwrite(buf, 1, (size_t)n, out);
            remaining -= n;
        }
        fclose(out);

        /* Restore original permissions */
        (void)chmod(path, str_to_perms(perms[i]));
    }

    fclose(in);

    /* Success message */
    char list[MAX_FILES * (MAX_NAME + 5)];
    build_names_list(list, sizeof(list), names, file_count);

    if (target)
        printf("%s dizininde %s dosyalari acildi.\n", target, list);
    else
        printf("%s dosyalari acildi.\n", list);
}
