// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2021 Darby Johnston
// All rights reserved.

#include "fseq.h"

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(WIN32) || defined(_WIN32)
#include <direct.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if defined(WIN32) || defined(_WIN32)

void fseqMkdir(const char* fileName)
{
    int wlen = MultiByteToWideChar(CP_UTF8, 0, fileName, -1, NULL, 0);
    wchar_t* wbuf = malloc(wlen * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, fileName, -1, wbuf, wlen);
    _wmkdir(wbuf);
    free(wbuf);
}

void fseqTouch(const char* fileName)
{
    int wlen = MultiByteToWideChar(CP_UTF8, 0, fileName, -1, NULL, 0);
    wchar_t* wbuf = malloc(wlen * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, fileName, -1, wbuf, wlen);
    fclose(_wfopen(wbuf, L"w"));
    free(wbuf);
}

#else

void fseqMkdir(const char* fileName)
{
    mkdir(fileName, 0777);
}

void fseqTouch(const char* fileName)
{
    fclose(fopen(fileName, "w"));
}

#endif

void test0()
{
    struct FSeqFileNameSizes a;
    
    fseqFileNameSizesInit(&a);
    assert(0 == a.path);
    assert(0 == a.base);
    assert(0 == a.number);
    assert(0 == a.extension);
}

void test1()
{
    const char testData[][5][FSEQ_STRING_LEN] =
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
        { "/a/b/c-0100.ext", "/a/b/", "c", "-0100", ".ext" },
        { "C:\\a\\b\\c-0100.ext", "C:\\a\\b\\", "c", "-0100", ".ext" }
    };
    const size_t testDataSize = sizeof(testData) / sizeof(testData[0]);
    
    for (size_t i = 0; i < testDataSize; ++i)
    {
        struct FSeqFileNameSizes sizes;
        struct FSeqFileName fileName;

        fseqFileNameSizesInit(&sizes);
        fseqFileNameParseSizes(testData[i][0], &sizes, FSEQ_STRING_LEN, NULL);

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

void test2()
{
    const char test[FSEQ_STRING_LEN] = "9223372036854775807.ext";
    struct FSeqFileNameOptions options;
    struct FSeqFileNameSizes sizes;
    struct FSeqFileName fileName;

    fseqFileNameOptionsInit(&options);

    fseqFileNameSizesInit(&sizes);
    fseqFileNameParseSizes(test, &sizes, FSEQ_STRING_LEN, NULL);

    fseqFileNameInit(&fileName);
    fseqFileNameSplit2(test, &sizes, &fileName);

    assert(0 == memcmp(fileName.path, "", sizes.path));
    assert(0 == memcmp(fileName.base, "", sizes.base));
    assert(0 == memcmp(fileName.number, "9223372036854775807", sizes.number));
    assert(0 == memcmp(fileName.extension, ".ext", sizes.extension));

    fseqFileNameSizesInit(&sizes);
    fseqFileNameParseSizes(test, &sizes, FSEQ_STRING_LEN, &options);

    fseqFileNameInit(&fileName);
    fseqFileNameSplit2(test, &sizes, &fileName);

    assert(0 == memcmp(fileName.path, "", sizes.path));
    assert(0 == memcmp(fileName.base, "9223372036854775807", sizes.base));
    assert(0 == memcmp(fileName.number, "", sizes.number));
    assert(0 == memcmp(fileName.extension, ".ext", sizes.extension));
}

void test3()
{
    struct FSeqFileNameSizes a;
    struct FSeqFileNameSizes b;
    
    fseqFileNameSizesInit(&a);
    fseqFileNameSizesInit(&b);
    assert(fseqFileNameSizesCompare(&a, &b) != 0);
    a.path = 1;
    assert(fseqFileNameSizesCompare(&a, &b) == 0);
}

void test4()
{
    struct FSeqFileName a;
    
    fseqFileNameInit(&a);
    assert(NULL == a.path);
    assert(NULL == a.base);
    assert(NULL == a.number);
    assert(NULL == a.extension);
}

void test5()
{
    struct FSeqFileName a;

    fseqFileNameInit(&a);
    fseqFileNameSplit("/tmp/seq.1.tif", &a, FSEQ_STRING_LEN, NULL);
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

void test6()
{
    char fileNameA[] = "/tmp/seq.1.tif";
    char fileNameB[] = "/tmp/seq.2.tif";
    struct FSeqFileNameSizes sizesA;
    struct FSeqFileNameSizes sizesB;
    int match = 0;
    
    fseqFileNameSizesInit(&sizesA);
    fseqFileNameParseSizes(fileNameA, &sizesA, FSEQ_STRING_LEN, NULL);
    assert(5 == sizesA.path);
    assert(4 == sizesA.base);
    assert(1 == sizesA.number);
    assert(4 == sizesA.extension);
    
    fseqFileNameSizesInit(&sizesB);
    fseqFileNameParseSizes(fileNameB, &sizesB, FSEQ_STRING_LEN, NULL);
    assert(5 == sizesB.path);
    assert(4 == sizesB.base);
    assert(1 == sizesB.number);
    assert(4 == sizesB.extension);
    
    match = fseqFileNameMatch(fileNameA, &sizesA, fileNameB, &sizesB);
    assert(match != 0);
}

void test7()
{
    struct FSeqDirEntry entry;
    char buf[FSEQ_STRING_LEN];
    
    fseqDirEntryInit(&entry);
    fseqDirEntryToString(&entry, buf, FSEQ_FALSE, FSEQ_STRING_LEN);
    assert(0 == strlen(buf));
    fseqDirEntryDel(&entry);
}

void test8()
{
    struct FSeqDirEntry entry;
    char buf[FSEQ_STRING_LEN];

    fseqDirEntryInit(&entry);
    fseqFileNameSplit("/tmp/seq.exr", &entry.fileName, FSEQ_STRING_LEN, NULL);
    
    fseqDirEntryToString(&entry, buf, FSEQ_FALSE, FSEQ_STRING_LEN);
    assert(0 == strcmp(buf, "seq.exr"));
    fseqDirEntryToString(&entry, buf, FSEQ_TRUE, FSEQ_STRING_LEN);
    assert(0 == strcmp(buf, "/tmp/seq.exr"));

    fseqDirEntryDel(&entry);
}

void test9()
{
    struct FSeqDirEntry entry;
    char buf[FSEQ_STRING_LEN];
    char buf2[FSEQ_STRING_LEN];

    fseqDirEntryInit(&entry);
    fseqFileNameSplit("/tmp/seq.1.exr", &entry.fileName, FSEQ_STRING_LEN, NULL);

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

    entry.framePadding = 5;
    fseqDirEntryToString(&entry, buf, FSEQ_FALSE, FSEQ_STRING_LEN);
    assert(0 == strcmp(buf, "seq.01000-10000.exr"));
    
    entry.frameMin = 0;
    entry.frameMax = INT64_MAX;
    entry.framePadding = 0;
    fseqDirEntryToString(&entry, buf, FSEQ_FALSE, FSEQ_STRING_LEN);
    snprintf(buf2, FSEQ_STRING_LEN, "seq.%" PRId64 "-%" PRId64 ".exr", (int64_t)0, INT64_MAX);
    assert(0 == strcmp(buf, buf2));
    fseqDirEntryToString(&entry, buf, FSEQ_TRUE, FSEQ_STRING_LEN);
    snprintf(buf2, FSEQ_STRING_LEN, "/tmp/seq.%" PRId64 "-%" PRId64 ".exr", (int64_t)0, INT64_MAX);
    assert(0 == strcmp(buf, buf2));

    fseqDirEntryDel(&entry);
}

void test10()
{
    struct FSeqDirEntry* entry = NULL;
    
    fseqMkdir("tests");
    fseqMkdir("tests/test9");
    entry = fseqDirList("tests/test9", NULL, NULL);
    assert(NULL == entry);
    
    fseqDirListDel(entry);
}

void test11()
{
    struct FSeqDirEntry* entry = NULL;
    char buf[FSEQ_STRING_LEN];
    
    fseqMkdir("tests");
    fseqMkdir("tests/test10");
    fseqTouch("tests/test10/file");
    fseqTouch("tests/test10/seq.1.exr");
    fseqTouch("tests/test10/seq.2.exr");
    fseqTouch("tests/test10/seq.3.exr");
    fseqTouch("tests/test10/seq.0001.tiff");
    fseqTouch("tests/test10/seq.0002.tiff");
    fseqTouch("tests/test10/seq.0003.tiff");
    entry = fseqDirList("tests/test10", NULL, NULL);
    assert(entry != NULL);

    size_t matches = 0;
    for (const struct FSeqDirEntry* i = entry; i != NULL; i = i->next)
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

void test12()
{
    struct FSeqDirEntry* entry = NULL;
    struct FSeqDirOptions options;
    char buf[FSEQ_STRING_LEN];
    char buf2[FSEQ_STRING_LEN];

    fseqDirOptionsInit(&options);
    options.dotAndDotDotDirs = FSEQ_TRUE;
    options.dotFiles = FSEQ_TRUE;
    options.sequence = FSEQ_FALSE;
    fseqMkdir("tests");
    fseqMkdir("tests/test11");
    fseqTouch("tests/test11/.dotfile");
    fseqTouch("tests/test11/seq.1.exr");
    fseqTouch("tests/test11/seq.2.exr");
    fseqTouch("tests/test11/seq.3.exr");
    snprintf(buf2, FSEQ_STRING_LEN, "tests/test11/large.%" PRId64 ".exr", INT64_MAX);
    fseqTouch(buf2);
    snprintf(buf2, FSEQ_STRING_LEN, "large.%" PRId64 ".exr", INT64_MAX);

    entry = fseqDirList("tests/test11", &options, NULL);
    assert(entry != NULL);

    size_t matches = 0;
    for (const struct FSeqDirEntry* i = entry; i != NULL; i = i->next)
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
        else if (0 == strcmp(buf, buf2))
        {
            ++matches;
        }
    }
    assert(7 == matches);

    fseqDirListDel(entry);
}

void test13()
{
    const int count = 65536;
    struct FSeqDirEntry* entry = NULL;
    struct FSeqDirOptions options;
    char buf[FSEQ_STRING_LEN];
    char buf2[FSEQ_STRING_LEN];

    fseqDirOptionsInit(&options);
    options.dotAndDotDotDirs = FSEQ_TRUE;
    options.dotFiles = FSEQ_FALSE;
    options.sequence = FSEQ_TRUE;
    fseqMkdir("tests");
    fseqMkdir("tests/test12");
    for (int i = 0; i < count; ++i)
    {
        snprintf(buf2, FSEQ_STRING_LEN, "tests/test12/large.%08d.exr", i);
        fseqTouch(buf2);
    }
    snprintf(buf2, FSEQ_STRING_LEN, "large.%08d-%08d.exr", 0, count - 1);

    entry = fseqDirList("tests/test12", &options, NULL);
    assert(entry != NULL);

    size_t matches = 0;
    for (const struct FSeqDirEntry* i = entry; i != NULL; i = i->next)
    {
        fseqDirEntryToString(i, buf, FSEQ_FALSE, FSEQ_STRING_LEN);
        if (0 == strcmp(buf, buf2))
        {
            ++matches;
        }
    }
    assert(1 == matches);

    fseqDirListDel(entry);
}

void test14()
{
    struct FSeqDirEntry* entry = NULL;
    FSeqBool error = FSEQ_FALSE;

    entry = fseqDirList("tests/dir4", NULL, &error);
    assert(NULL == entry);
    assert(FSEQ_TRUE == error);

    fseqDirListDel(entry);
}

void test15()
{
    struct FSeqDirEntry* entry = NULL;
    char buf[FSEQ_STRING_LEN];

    fseqMkdir("tests");
    fseqMkdir("tests/测试文件夹");
    fseqTouch("tests/测试文件夹/大平原_1.jpg");
    fseqTouch("tests/测试文件夹/大平原_2.jpg");

    entry = fseqDirList("tests/测试文件夹", NULL, NULL);
    assert(entry != NULL);

    size_t matches = 0;
    for (const struct FSeqDirEntry* i = entry; i != NULL; i = i->next)
    {
        fseqDirEntryToString(i, buf, FSEQ_FALSE, FSEQ_STRING_LEN);
        if (0 == strcmp(buf, "大平原_1-2.jpg"))
        {
            ++matches;
        }
    }
    assert(1 == matches);

    fseqDirListDel(entry);
}

int main(int argc, char** argv)
{
    test0();
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
    test9();
    test10();
    test11();
    test12();
    test13();
    test14();
    test15();
    return 0;
}

