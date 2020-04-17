// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2020 Darby Johnston
// All rights reserved.

#include "fseq.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printDir(const char* path, const struct FSeqDirOptions* options)
{
    struct FSeqDirEntry* entries = NULL;
    struct FSeqDirEntry* entry   = NULL;
    FSeqBool             error   = FSEQ_FALSE;

    entries = fseqDirList(path, options, &error);
    if (error)
    {
        printf("cannot read %s\n", path);
        return;
    }

    entry = entries;
    while (entry)
    {
        static char buf[FSEQ_STRING_LEN];
        fseqDirEntryToString(entry, buf, FSEQ_FALSE, FSEQ_STRING_LEN);
        printf("%s\n", buf);
        entry = entry->next;
    }

    fseqDirListDel(entries);
}

int main(int argc, char** argv)
{
    struct FSeqDirOptions options;

    fseqDirOptionsInit(&options);

    if (argc > 1)
    {
        // List the input directories.
        for (int i = 1; i < argc; ++i)
        {
            printDir(argv[i], &options);
        }
    }
    else
    {
        // List the current directory.
        printDir(".", &options);
    }

    return 0;
}
