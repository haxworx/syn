/*
    (c) Copyright 2015, 2016. Al Poole <netstar@gmail.com>
    All Rights Reserved.
*/

#include "stdinc.h"
#include "video.h"

void fail(char *fmt, ...)
{
    char buf[8192] = { 0 };
    va_list ap;

    va_start(ap, fmt);

    vsnprintf(buf, sizeof(buf), fmt, ap);

    va_end(ap);

    fprintf(stderr, "error: %s\n", buf);

    exit(1 << 7);
}

#define SYNTH_DATA_DIR_NAME "Recordings"

void set_working_directory(void)
{
    char *homedrive = getenv("HOMEDRIVE");
    if (homedrive) {
        char *homepath = getenv("HOMEPATH");
        if (!homepath)
            fail("getenv: HOME");

        snprintf(working_directory, sizeof(working_directory), "%s%s\\%s",
                 homedrive, homepath, SYNTH_DATA_DIR_NAME);
    } else {
        snprintf(working_directory, sizeof(working_directory), "%c%c%s", '.', SLASH, SYNTH_DATA_DIR_NAME);      // CWD
    }

    struct stat fstats;

    if (stat(working_directory, &fstats) < 0)
#ifdef WINDOWS
        mkdir(working_directory);
#else
        mkdir(working_directory, 0777);
#endif

    current_action("Visit http://haxlab.org for more!");
}


uint32_t hashish(char *s)
{
    char *p = s;
    uint32_t res = 0;

    while (*p) {
        res *= (uint32_t) * p + PRIMORDIAL;
        p++;
    }

    return res % TABLE_SIZE;
}

node_t *cached[CACHE_SIZE] = { NULL };

// we could just use above, however, it's fun not to
node_t *table[TABLE_SIZE] = { NULL };

void table_free(void)
{
    for (int i = 0; i < TABLE_SIZE; i++) {
        node_t *cursor = table[i];
        while (cursor) {
            free(cursor->key);
            free(cursor->value);
            node_t *next = cursor->next;
            free(cursor);
            cursor = next;

        }
    }
}

void table_insert(char *key, char *value, int16_t ival)
{
    uint32_t index = hashish(key);

    node_t *cursor = table[index];

    if (!cursor) {
        table[index] = cursor = calloc(1, sizeof(node_t));
        if (!cursor)
            fail("calloc: %s\n", strerror(errno));

        cursor->key = strdup(key);
        if (value) {
            cursor->value = strdup(value);
            if (!cursor->value)
                fail("strdup: %s\n", strerror(errno));
        } else
            cursor->value = NULL;


        cursor->ival = ival;
        cursor->next = NULL;
        return;
    }

    while (cursor->next)
        cursor = cursor->next;

    if (cursor->next == NULL) {
        cursor->next = calloc(1, sizeof(node_t));
        if (!cursor)
            fail("calloc: %s\n", strerror(errno));

        cursor = cursor->next;
        cursor->key = strdup(key);
        if (value) {
            cursor->value = strdup(value);
            if (!cursor->value)
                fail("strdup: %s\n", strerror(errno));

        } else
            cursor->value = NULL;

        cursor->ival = ival;
        cursor->next = NULL;
    }
}

bool table_delete(char *key)
{
    if ((int) *key < CACHE_SIZE && cached[(int) *key] != NULL) {
        cached[(int) *key] = NULL;
        return true;
    }

    uint32_t idx = hashish(key);
    node_t *tmp = table[idx];
    if (!tmp)
        return false;

    node_t *last = NULL;

    while (tmp) {
        if (tmp->key && !strcmp(tmp->key, key)) {
            if (tmp->value)
                free(tmp->value);
            if (last)
                last->next = tmp->next;

            if (tmp->key)
                free(tmp->key);

            free(tmp);

            return true;
        }

        last = tmp;
        tmp = tmp->next;
    }

    return false;
}

node_t *table_search(char *key)
{
    if ((int) *key < CACHE_SIZE && cached[(int) *key] != NULL) {
        //printf("cache!\n");
        return cached[(int) *key];
    }
    // printf("no cache yet!\n");

    uint32_t idx = hashish(key);
    node_t *tmp = table[idx];
    if (!tmp) {
        return NULL;
    }

    while (tmp) {
        if (tmp->key && !strcmp(tmp->key, key)) {
            if (tmp->use_count >= CACHE_THRESHOLD) {
                cached[(int) *key] = tmp;
            }

            ++tmp->use_count;
            return (node_t *) tmp;
        }

        tmp = tmp->next;
    }

    return NULL;
}

void table_dump(void)
{
    for (int i = 0; i < TABLE_SIZE; i++) {
        node_t *cursor = table[i];
        while (cursor) {
            if (cursor->value) {
                printf("key -> %s val -> %s\n",
                       cursor->key, cursor->value);
            } else {
                printf("key -> %s ival -> %d\n",
                       cursor->key, cursor->ival);
            }
            cursor = cursor->next;
        }
    }
}
