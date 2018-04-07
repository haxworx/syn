#ifndef __STDINC_H__
#define __STDINC_H__
#define _XOPEN_SOURCE 501

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include "audio.h"

#define VERSION "[v0.6.0.0]  "

#ifndef PATH_MAX
#define PATH_MAX 8192
#endif

#ifdef Windows
#define SLASH '\\'
#else
#define SLASH '/'
#endif

#define pi 3.14159265358979323846

void fail(char *fmt, ...);

#define PRIMORDIAL 101
#define TABLE_SIZE 32768 * PRIMORDIAL

typedef struct node_t node_t;
struct node_t {
    char *key;
    char *value;
    int16_t ival;
    uint8_t use_count;
    node_t *next;
};

uint32_t hashish(char *s);

#define CACHE_THRESHOLD 16
#define CACHE_SIZE 1024

void table_insert(char *key, char *value, int16_t ival);
bool table_delete(char *key);
node_t *table_search(char *key);
void table_free(void);
void table_dump(void);

#endif
