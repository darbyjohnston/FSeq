// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2020 Darby Johnston
// All rights reserved.

#include "fseq.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32)
#define FSEQ_SNPRINTF sprintf_s
FILE* _fopen(const char* fileName, const char* mode)
{
    FILE* out = NULL;
    fopen_s(&out, fileName, mode);
    return out;
}
#else
#define FSEQ_SNPRINTF snprintf
FILE* _fopen(const char* fileName, const char* mode)
{
    return fopen(fileName, mode);
}
#endif

static const char fileNames[][FSEQ_STRING_LEN] =
{
    "shot%d_scene%d_normals",
    "shot%d_scene%d_rgba",
    "shot%d_scene%d_z"
};
static const size_t fileNamesSize = sizeof(fileNames) / sizeof(fileNames[0]);

int randInt(int max)
{
    return (int)(max * (rand() / (float)RAND_MAX));
}

int main(int argc, char** argv)
{
    int fileCount = 0;
    int seqCount  = 0;

    if (argc != 4)
    {
        printf("usage: fseqCreateRandom (directory) (total file count) (sequence count)\n");
        return 1;
    }
    fileCount = atoi(argv[2]);
    seqCount  = atoi(argv[3]);

    for (size_t i = 0; i < seqCount; ++i)
    {
        static char seq[FSEQ_STRING_LEN];
        int frame = 0;
        FSEQ_SNPRINTF(seq, FSEQ_STRING_LEN, fileNames[randInt((int)fileNamesSize - 1)], randInt(100), randInt(100));
        frame = randInt(1000);
        for (size_t j = 0; j < fileCount / seqCount; ++j, ++frame)
        {
            static char buf[FSEQ_STRING_LEN];
            FILE* f = NULL;
            FSEQ_SNPRINTF(buf, FSEQ_STRING_LEN, "%s/%s.%d.tif", argv[1], seq, frame);
            f = _fopen(buf, "w");
            if (!f)
            {
                printf("cannot create %s\n", buf);
                return 1;
            }
            fclose(f);
        }
    }
    return 0;
}
