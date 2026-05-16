# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**tarsau** is a command-line archiving utility written in C for Linux/Unix, similar to `tar`/`zip` but without compression. It archives text-only files into a custom `.sau` format and extracts them back, preserving file permissions.

- **Deadline:** May 24, 2026 at 23:59
- **Language:** C (Linux/Unix, POSIX)
- **Submission format:** `b211210095_b211210024.rar/zip`

## Build & Run

```bash
make              # build tarsau executable
make clean        # remove object files and executable
```

Compiler: `gcc -Wall -Wextra -std=c11`

```bash
# Archive mode: combine text files into .sau
./tarsau -b t1.txt t2.txt t3.txt -o output.sau
./tarsau -b t1.txt t2.txt          # defaults to a.sau

# Extract mode: extract .sau archive
./tarsau -a output.sau target_dir  # extracts to target_dir (creates if missing)
./tarsau -a output.sau             # extracts to current directory
```

## Testing

```bash
# Basic round-trip test
echo "hello" > t1.txt && echo "world" > t2.txt
./tarsau -b t1.txt t2.txt -o test.sau
./tarsau -a test.sau out/
diff t1.txt out/t1.txt && diff t2.txt out/t2.txt

# Permission preservation
chmod 755 t1.txt
./tarsau -b t1.txt -o perm.sau
./tarsau -a perm.sau out2/
ls -la out2/t1.txt   # must show rwxr-xr-x

# Error cases
./tarsau -b binary_file.bin -o bad.sau   # must print: "t7 giriş dosyasının formatı uyumsuzdur!"
./tarsau -a corrupted.sau dir            # must print: "Arşiv dosyası uygunsuz veya bozuk!"
```

## Architecture

```
main.c          argument parsing, mode dispatch (-b / -a)
archive.c/.h    -b mode: validate, build header, write .sau
extract.c/.h    -a mode: parse header, create dir, restore files
utils.c/.h      shared: text-file validation, perms <-> string conversion, error exit
```

### `.sau` File Format

```
[10-byte ASCII org-section size]|filename,permissions,size|filename,permissions,size|...[file1 contents][file2 contents]...
```

Example:
```
0000000045|t1,rw-r--r--,120|t2,rwxr-xr-x,85|[t1 raw bytes][t2 raw bytes]
```

- First 10 bytes: zero-padded ASCII integer = byte length of the organization section
- Organization section: `|`-separated records, each `name,perms,size` (comma-separated)
- Content section: raw file bytes concatenated with **no separator** — split using sizes from header

### Key Constraints

| Rule | Value |
|------|-------|
| Max files | 32 |
| Max total size | 200 MB |
| File type | Text only (ASCII: printable + `\n`, `\r`, `\t`; reject bytes > 127 or < 32 otherwise) |
| Permissions | 9-char string, e.g. `rw-r--r--`; use `stat()` to read, `chmod()` to restore |
| Missing output dir | Create with `mkdir()` before extracting |

### Critical Implementation Notes

**Text-file validation** (reject binary files before archiving):
```c
int is_text_file(const char *path) {
    FILE *fp = fopen(path, "rb");
    int ch;
    while ((ch = fgetc(fp)) != EOF)
        if (ch > 127 || (ch < 32 && ch != '\n' && ch != '\r' && ch != '\t'))
            return fclose(fp), 0;
    return fclose(fp), 1;
}
```

**Permission conversion** uses `S_IRUSR/S_IWUSR/S_IXUSR/S_IRGRP/…` bitmasks from `<sys/stat.h>`.

**Header parsing** on extract: read 10 bytes → org size → `fread` org section → `strtok` on `|` then `,`.

### Required Headers

```c
#include <sys/stat.h>   // stat(), chmod(), mkdir(), S_I* constants
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
```

## Submission Checklist

- [ ] Compiles with `make` on Linux
- [ ] `-b`: archives text files into `.sau` with correct header format
- [ ] `-a`: extracts and restores original file permissions
- [ ] Error messages exactly match spec strings (in Turkish)
- [ ] Limits enforced: ≤ 32 files, ≤ 200 MB total
- [ ] Default output name `a.sau` when `-o` is omitted
- [ ] No crashes — all exits clean via `exit(1)` with message
- [ ] GitHub repo with development history
- [ ] Report prepared (code snippets + terminal screenshots)
