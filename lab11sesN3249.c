#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define __USE_XOPEN
#include <sys/stat.h>
#include <sys/types.h>

#define __USE_XOPEN_EXTENDED
#include <ftw.h>
// #define LAB11DEBUG

const int searchDepth = 20;
const int BUF_SIZE = 1024;
char* target;
int targetLen;

struct foundStr {
    int idx;
    int len;
};

int hexToDec(char a) {
    if(a >= '0' && a <= '9') {
        return a - '0';
    } else if((a >= 'a' && a <= 'f') || (a >= 'A' && a <= 'F')) {
        return 10 + (a - 'a');
    } else {
        fprintf(stderr, "not a hex\n");
        exit(0);
    }
}

int getByte(char* target, int idx) {
    int byte = hexToDec(target[idx]);
    byte <<= 4;
    byte |= hexToDec(target[idx + 1]);
    return byte;
}

void bytesToStr(char* bytes, char* res) {
    int ptr = 0;
    for(int i = 2; i < targetLen * 2; i += 2) {
        res[ptr] = getByte(bytes, i);
        ptr++;
    }

    #ifdef LAB11DEBUG
    printf("target == %s\n", res);
    #endif
}

struct foundStr lookFile(const char *fpath) {
    struct foundStr check;
    check.idx = -1;
    check.len = 0;
    
    FILE* fd = fopen(fpath, "r");

    if(!fd) {
        fprintf(stderr, "Error opening file %s: %s\n", fpath, strerror(errno));
        exit(0);
        return check;
    }

    #ifdef LAB11DEBUG
    printf("reading %s\n", fpath);
    #endif

    int ch;
    int cnt = 0;
    while((ch = fgetc(fd)) != EOF) {
        if(ch == target[check.len]) {
            check.len++;
            if(check.idx == -1) {
                check.idx = cnt;
            }
        } else {
            check.idx = -1;
            check.len = 0;
        }
        if(target[check.len] == '\0') {
            check.len++;
            break;
        }
        cnt++;
    }

    fclose(fd);
    return check;
}

int fn(const char *fpath, const struct stat *sb,
             int tflag, struct FTW *ftwbuf) {
    //ограничение на глубину поиска
    // if(ftwbuf->level >= searchDepth) {
    //     #ifdef LAB11DEBUG
    //     printf("search depth exceeded\n");
    //     #endif
    //     return 0;
    // }
    ftwbuf = ftwbuf;
    if (tflag == FTW_F && ((sb->st_mode & S_IRWXU) & S_IRUSR) && ((sb->st_mode & S_IFMT) & S_IFREG)
        && ((sb->st_mode & S_IRWXG) & S_IRGRP) && ((sb->st_mode & S_IRWXO) & S_IROTH)) {
        struct foundStr ans = lookFile(fpath);
        if(ans.len == targetLen) {
            printf("found %s\n", fpath);
            #ifdef LAB11DEBUG
            printf("your substr found at index %d\n", ans.idx);
            #endif
        }
    } else {
        #ifdef LAB11DEBUG
        printf("skip %s\n", fpath);
        #endif
    }
    return 0;   
}

int main(int argc, char **argv) {
    int res = 0, optIndex = -1;
    const char* shortOpt = "hv";
    const struct option longOpt[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'v'},
    };

    #ifdef LAB11DEBUG
    printf("argc == %d\n", argc);
    #endif

    if(argc == 2) {
        while((res = getopt_long(argc, argv, shortOpt, longOpt, &optIndex)) != -1) {
            #ifdef LAB11DEBUG
            printf("processing opts\n");
            #endif

            switch (res)
            {
            case 'v': printf("Version 1.1\nAuthor: Stanbekov Sultan Ernisovich\nGroup: N3249\nVariant: 4\n"); break;
            case 'h': 
                printf("-v --version \t print version information in console\n-h --help \t open this menu\n"); 
                printf("Usage: ./lab11sesN3249 [directory] [target]\n");
                break;
            default: printf("%s", argv[optIndex]); break;
            }
        }
        exit(0);
    }

    else if(argc != 3) {
        errno = 1;
        perror("Missing options");
    }

    char* dir = argv[argc - 2];
    char* targetBytes = argv[argc - 1];

    if(strlen(targetBytes) < 2) {
        perror("invalid target");
        exit(0);
    }

    if(targetBytes[0] != '0' || targetBytes[1] != 'x') {
        perror("invalid target");
        exit(0);
    }

    targetLen = strlen(targetBytes) / 2;
    target = malloc(targetLen);
    target[targetLen - 1] = '\0';

    bytesToStr(targetBytes, target);

    nftw(dir, fn, 20, FTW_DEPTH || FTW_PHYS);

    free(target);

    return 0;
}