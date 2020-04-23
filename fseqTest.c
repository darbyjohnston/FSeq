// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2020 Darby Johnston
// All rights reserved.

#include "fseq.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static void _mkdir(const char* fileName)
{
    mkdir(fileName, 0777);
}

static void touch(const char* fileName)
{
    fclose(fopen(fileName, "w"));
}

int main(int argc, char** argv)
{
    {
        struct FSeqFileNameSizes a;
        
        fseqFileNameSizesInit(&a);
        assert(0 == a.path);
        assert(0 == a.base);
        assert(0 == a.number);
        assert(0 == a.extension);
    }
    {
        static const char testData[][5][FSEQ_STRING_LEN] =
        {
            { "", "", "", "", "" },
            { "/", "/", "", "", "" },
            { ".", "", ".", "", "" },
            { "/.", "/", ".", "", "" },
            { "\\", "\\", "", "", "" },
            { "\\\\", "\\\\", "", "", "" },
            { "\\\\.", "\\\\", ".", "", "" },
            { "\\\\.\\", "\\\\.\\", "", "", "" },
            { "0", "", "", "0", "" },
            { "/0", "/", "", "0", "" },
            { "/a/b/c1.ext", "/a/b/", "c", "1", ".ext" },
            { "/a/b/c10.ext", "/a/b/", "c", "10", ".ext" },
            { "/a/b/c100.ext", "/a/b/", "c", "100", ".ext" },
            { "/a/b/c0100.ext", "/a/b/", "c", "0100", ".ext" },
            { "/a/b/c-0100.ext", "/a/b/", "c-", "0100", ".ext" },
            { "C:\\a\\b\\c-0100.ext", "C:\\a\\b\\", "c-", "0100", ".ext" }
        };
        static const size_t testDataSize = sizeof(testData) / sizeof(testData[0]);
        
        for (size_t i = 0; i < testDataSize; ++i)
        {
            struct FSeqFileNameSizes sizes;
            struct FSeqFileName fileName;

            fseqFileNameSizesInit(&sizes);
            fseqFileNameParseSizes(testData[i][0], &sizes, FSEQ_STRING_LEN);

            fseqFileNameInit(&fileName);
            fseqFileNameSplit2(testData[i][0], &sizes, &fileName);

            printf("\"%s\": \"%s\" \"%s\" \"%s\" \"%s\"\n",
                testData[i][0],
                fileName.path,
                fileName.base,
                fileName.number,
                fileName.extension);

            assert(strlen(testData[i][1]) == sizes.path && 0 == memcmp(fileName.path, testData[i][1], sizes.path));
            assert(strlen(testData[i][2]) == sizes.base && 0 == memcmp(fileName.base, testData[i][2], sizes.base));
            assert(strlen(testData[i][3]) == sizes.number && 0 == memcmp(fileName.number, testData[i][3], sizes.number));
            assert(strlen(testData[i][4]) == sizes.extension && 0 == memcmp(fileName.extension, testData[i][4], sizes.extension));

            fseqFileNameDel(&fileName);
        }
    }
    {
        struct FSeqFileNameSizes a;
        struct FSeqFileNameSizes b;
        
        fseqFileNameSizesInit(&a);
        fseqFileNameSizesInit(&b);
        assert(fseqFileNameSizesCompare(&a, &b) != 0);
        a.path = 1;
        assert(fseqFileNameSizesCompare(&a, &b) == 0);
    }
    {
        struct FSeqFileName a;
        
        fseqFileNameInit(&a);
        assert(NULL == a.path);
        assert(NULL == a.base);
        assert(NULL == a.number);
        assert(NULL == a.extension);
    }
    {
        struct FSeqFileName a;
        
        fseqFileNameInit(&a);
        fseqFileNameSplit("/tmp/seq.1.tif", &a, FSEQ_STRING_LEN);
        assert(0 == memcmp("/tmp/", a.path, strlen(a.path)));
        assert(0 == memcmp("seq.", a.base, strlen(a.base)));
        assert(0 == memcmp("1", a.number, strlen(a.number)));
        assert(0 == memcmp(".tif", a.extension, strlen(a.extension)));
        fseqFileNameDel(&a);
        assert(NULL == a.path);
        assert(NULL == a.base);
        assert(NULL == a.number);
        assert(NULL == a.extension);
    }
    {
        char fileNameA[] = "/tmp/seq.1.tif";
        char fileNameB[] = "/tmp/seq.2.tif";
        struct FSeqFileNameSizes sizesA;
        struct FSeqFileNameSizes sizesB;
        int match = 0;
        
        fseqFileNameSizesInit(&sizesA);
        fseqFileNameParseSizes(fileNameA, &sizesA, FSEQ_STRING_LEN);
        assert(5 == sizesA.path);
        assert(4 == sizesA.base);
        assert(1 == sizesA.number);
        assert(4 == sizesA.extension);
        
        fseqFileNameSizesInit(&sizesB);
        fseqFileNameParseSizes(fileNameB, &sizesB, FSEQ_STRING_LEN);
        assert(5 == sizesB.path);
        assert(4 == sizesB.base);
        assert(1 == sizesB.number);
        assert(4 == sizesB.extension);
        
        match = fseqFileNameMatch(fileNameA, &sizesA, fileNameB, &sizesB);
        assert(match != 0);
    }
    {
        struct FSeqDirEntry entry;
        char buf[FSEQ_STRING_LEN];
        
        fseqDirEntryInit(&entry);
        fseqDirEntryToString(&entry, buf, FSEQ_FALSE, FSEQ_STRING_LEN);
        assert(0 == strlen(buf));
        fseqDirEntryDel(&entry);
    }
    {
        struct FSeqDirEntry entry;
        char buf[FSEQ_STRING_LEN];

        fseqDirEntryInit(&entry);
        fseqFileNameSplit("/tmp/seq.exr", &entry.fileName, FSEQ_STRING_LEN);
        
        fseqDirEntryToString(&entry, buf, FSEQ_FALSE, FSEQ_STRING_LEN);
        assert(0 == strcmp(buf, "seq.exr"));
        fseqDirEntryToString(&entry, buf, FSEQ_TRUE, FSEQ_STRING_LEN);
        assert(0 == strcmp(buf, "/tmp/seq.exr"));

        fseqDirEntryDel(&entry);
    }
    {
        struct FSeqDirEntry entry;
        char buf[FSEQ_STRING_LEN];

        fseqDirEntryInit(&entry);
        fseqFileNameSplit("/tmp/seq.1.exr", &entry.fileName, FSEQ_STRING_LEN);

        fseqDirEntryToString(&entry, buf, FSEQ_FALSE, FSEQ_STRING_LEN);
        assert(0 == strcmp(buf, "seq.0.exr"));
        fseqDirEntryToString(&entry, buf, FSEQ_TRUE, FSEQ_STRING_LEN);
        assert(0 == strcmp(buf, "/tmp/seq.0.exr"));

        entry.frameMin = 1000;
        entry.frameMax = 10000;
        fseqDirEntryToString(&entry, buf, FSEQ_FALSE, FSEQ_STRING_LEN);
        assert(0 == strcmp(buf, "seq.1000-10000.exr"));
        fseqDirEntryToString(&entry, buf, FSEQ_TRUE, FSEQ_STRING_LEN);
        assert(0 == strcmp(buf, "/tmp/seq.1000-10000.exr"));

        entry.hasFramePadding = FSEQ_TRUE;
        entry.framePadding = 5;
        fseqDirEntryToString(&entry, buf, FSEQ_FALSE, FSEQ_STRING_LEN);
        assert(0 == strcmp(buf, "seq.01000-10000.exr"));

        fseqDirEntryDel(&entry);
    }
    {
        struct FSeqDirEntry* entry = NULL;
        
        _mkdir("tests");
        _mkdir("tests/dir0");
        entry = fseqDirList("tests/dir0", NULL, NULL);
        assert(NULL == entry);
    }
    {
        struct FSeqDirEntry* entry = NULL;
        char buf[FSEQ_STRING_LEN];
        
        _mkdir("tests");
        _mkdir("tests/dir2");
        touch("tests/dir2/file");
        touch("tests/dir2/seq.1.exr");
        touch("tests/dir2/seq.2.exr");
        touch("tests/dir2/seq.3.exr");
        touch("tests/dir2/seq.0001.tiff");
        touch("tests/dir2/seq.0002.tiff");
        touch("tests/dir2/seq.0003.tiff");
        entry = fseqDirList("tests/dir2", NULL, NULL);
        assert(entry != NULL);

        size_t matches = 0;
        for (struct FSeqDirEntry* i = entry; i != NULL; i = i->next)
        {
            fseqDirEntryToString(i, buf, FSEQ_FALSE, FSEQ_STRING_LEN);
            if (0 == strcmp(buf, "file"))
            {
                ++matches;
            }
            else if (0 == strcmp(buf, "seq.1-3.exr"))
            {
                ++matches;
            }
            else if (0 == strcmp(buf, "seq.0001-0003.tiff"))
            {
                ++matches;
            }
        }
        assert(3 == matches);
        
        fseqDirListDel(entry);
    }
    {
        struct FSeqDirEntry* entry = NULL;
        struct FSeqDirOptions options;
        char buf[FSEQ_STRING_LEN];
        
        fseqDirOptionsInit(&options);
        options.dotAndDotDotDirs = FSEQ_TRUE;
        options.dotFiles = FSEQ_TRUE;
        options.sequence = FSEQ_FALSE;
        _mkdir("tests");
        _mkdir("tests/dir3");
        touch("tests/dir3/.dotfile");
        touch("tests/dir3/seq.1.exr");
        touch("tests/dir3/seq.2.exr");
        touch("tests/dir3/seq.3.exr");
        
        entry = fseqDirList("tests/dir3", &options, NULL);
        assert(entry != NULL);
        
        size_t matches = 0;
        for (struct FSeqDirEntry* i = entry; i != NULL; i = i->next)
        {
            fseqDirEntryToString(i, buf, FSEQ_FALSE, FSEQ_STRING_LEN);
            if (0 == strcmp(buf, "."))
            {
                ++matches;
            }
            else if (0 == strcmp(buf, ".."))
            {
                ++matches;
            }
            else if (0 == strcmp(buf, ".dotfile"))
            {
                ++matches;
            }
            else if (0 == strcmp(buf, "file"))
            {
                ++matches;
            }
            else if (0 == strcmp(buf, "seq.1.exr"))
            {
                ++matches;
            }
            else if (0 == strcmp(buf, "seq.2.exr"))
            {
                ++matches;
            }
            else if (0 == strcmp(buf, "seq.3.exr"))
            {
                ++matches;
            }
        }
        assert(7 == matches);
        
        fseqDirListDel(entry);
    }
    {
        struct FSeqDirEntry* entry = NULL;
        FSeqBool error = FSEQ_FALSE;

        entry = fseqDirList("tests/dir4", NULL, &error);
        assert(NULL == entry);
        assert(FSEQ_TRUE == error);
    }
    return 0;
}

