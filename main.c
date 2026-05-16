#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "archive.h"
#include "extract.h"
#include "utils.h"

static void usage(void) {
    fprintf(stderr, "Kullanim:\n");
    fprintf(stderr, "  tarsau -b dosya1 [dosya2 ...] [-o cikti.sau]\n");
    fprintf(stderr, "  tarsau -a arsiv.sau [hedef_dizin]\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) { usage(); return 1; }

    if (strcmp(argv[1], "-b") == 0) {
        if (argc < 3) { usage(); return 1; }

        char *files[32];
        int   file_count = 0;
        char *output     = "a.sau";

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0) {
                if (i + 1 >= argc) {
                    fprintf(stderr, "Hata: -o parametresi icin dosya adi gerekli!\n");
                    return 1;
                }
                output = argv[++i];
            } else {
                if (file_count >= 32)
                    die("Hata: En fazla 32 dosya arsivlenebilir!");
                files[file_count++] = argv[i];
            }
        }

        if (file_count == 0)
            die("Hata: Arsivlenecek dosya belirtilmedi!");

        archive_files(files, file_count, output);

    } else if (strcmp(argv[1], "-a") == 0) {
        if (argc < 3) { usage(); return 1; }

        const char *archive_path = argv[2];
        const char *target       = (argc >= 4) ? argv[3] : NULL;

        extract_archive(archive_path, target);

    } else {
        fprintf(stderr, "Hata: Gecersiz parametre '%s'!\n", argv[1]);
        usage();
        return 1;
    }

    return 0;
}
