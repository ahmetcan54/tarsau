#include "archive.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_FILES      32
#define MAX_TOTAL_SIZE (200LL * 1024 * 1024)
/* Per-entry max: '|' + 512-char name + ',' + 9-char perms + ',' + 10-digit size */
#define ORG_BUF_SIZE   (MAX_FILES * 534 + 2)

void archive_files(char **files, int count, const char *output) {
    if (count > MAX_FILES)
        die("Hata: En fazla 32 dosya arsivlenebilir!");

    struct stat st;
    char   perms[MAX_FILES][10];
    long   sizes[MAX_FILES];
    long long total_size = 0;

    for (int i = 0; i < count; i++) {
        /* Existence / readability check */
        FILE *fp = fopen(files[i], "rb");
        if (!fp) {
            fprintf(stderr, "Hata: %s dosyasi acilamadi!\n", files[i]);
            exit(1);
        }
        fclose(fp);

        if (!is_text_file(files[i])) {
            fprintf(stderr, "%s giris dosyasinin formati uyumsuzdur!\n",
                    get_basename(files[i]));
            exit(1);
        }

        if (stat(files[i], &st) != 0) {
            fprintf(stderr, "Hata: %s dosya bilgisi alinamadi!\n", files[i]);
            exit(1);
        }

        perms_to_str(st.st_mode, perms[i]);
        sizes[i]    = (long)st.st_size;
        total_size += st.st_size;
    }

    if (total_size > MAX_TOTAL_SIZE)
        die("Hata: Toplam dosya boyutu 200 MB sinirini asiyor!");

    /* Build org section: |name,perms,size|name,perms,size|...| */
    char org[ORG_BUF_SIZE];
    int  org_len = 0;

    for (int i = 0; i < count; i++) {
        const char *name = get_basename(files[i]);
        int written = snprintf(org + org_len, ORG_BUF_SIZE - org_len,
                               "|%s,%s,%ld", name, perms[i], sizes[i]);
        if (written < 0 || org_len + written >= ORG_BUF_SIZE)
            die("Hata: Organizasyon bolumu cok buyuk!");
        org_len += written;
    }
    if (org_len + 1 >= ORG_BUF_SIZE)
        die("Hata: Organizasyon bolumu cok buyuk!");
    org[org_len++] = '|';
    org[org_len]   = '\0';

    /* Write archive */
    FILE *out = fopen(output, "wb");
    if (!out) {
        fprintf(stderr, "Hata: %s arsiv dosyasi olusturulamadi!\n", output);
        exit(1);
    }

    /* 10-byte zero-padded org section size */
    fprintf(out, "%010d", org_len);

    /* Org section */
    fwrite(org, 1, org_len, out);

    /* Raw file contents in order */
    for (int i = 0; i < count; i++) {
        FILE *fp = fopen(files[i], "rb");
        if (!fp) {
            fclose(out);
            fprintf(stderr, "Hata: %s dosyasi acilamadi!\n", files[i]);
            exit(1);
        }
        char   buf[8192];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
            fwrite(buf, 1, n, out);
        fclose(fp);
    }

    fclose(out);
    printf("Dosyalar birlestirildi.\n");
}
