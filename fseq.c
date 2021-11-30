// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2021 Darby Johnston
// All rights reserved.

#include "fseq.h"

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#include <windows.h>
#else
#include <sys/types.h>
#include <dirent.h>
#endif

void fseqFileNameSizesInit(struct FSeqFileNameSizes* value)
{
    value->path      = 0;
    value->base      = 0;
    value->number    = 0;
    value->extension = 0;
}

#define _IS_PATH_SEPARATOR(V) \
	('/' == V || \
     '\\' == V)
#define _IS_DOT(V) \
	('.' == V)
#define _IS_NUMBER(V) \
	('0' == V || \
     '1' == V || \
     '2' == V || \
     '3' == V || \
     '4' == V || \
     '5' == V || \
     '6' == V || \
     '7' == V || \
     '8' == V || \
     '9' == V)

unsigned short fseqFileNameParseSizes(
    const char*               in,
    struct FSeqFileNameSizes* out,
    size_t                    max)
{
    unsigned short len               = 0;
    int            lastPathSeparator = -1;
    int            lastDot           = -1;
    int            start             = 0;
    int            end               = 0;
    const char*    p                 = in;

    // Iterate over the characters to find the last path separator,
    // the last dot, and the end of the string.
    for (; *p && len < max; ++p, ++len)
    {
        if (_IS_PATH_SEPARATOR(*p))
        {
            lastPathSeparator = (int)(p - in);
        }
        else if (_IS_DOT(*p))
        {
            lastDot = (int)(p - in);
        }
    }

    if (len > 0)
    {
        // Extract the path.
        if (lastPathSeparator != -1)
        {
            start = lastPathSeparator + 1;
            out->path = (unsigned short)(start);
        }

        // Extract the extension.
        end = len - 1;
        if (lastDot > 0 && lastDot > lastPathSeparator + 1)
        {
            out->extension = (unsigned short)(end - lastDot + 1);
            end = lastDot - 1;
        }

        // Extract the number or wildcard.
        p = in + end;
        if (_IS_NUMBER(*p))
        {
            while (p > in && _IS_NUMBER(*(p - 1)))
            {
                --p;
            }
            if (p > in && '-' == *(p - 1))
            {
                --p;
            }
            out->number = (unsigned short)(end - (p - in) + 1);
            end = (int)(p - in - 1);
        }
        else if ('#' == *p)
        {
            while (p > in && '#' == *(p - 1))
            {
                --p;
            }
            out->number = (unsigned short)(end - (p - in) + 1);
            end = (int)(p - in - 1);
        }

        // Whatever is leftover is the base.
        out->base = end - start + 1;
    }
    return len;
}

FSeqBool fseqFileNameSizesCompare(
    const struct FSeqFileNameSizes* a,
    const struct FSeqFileNameSizes* b)
{
    return (
        a->path      == b->path      &&
        a->base      == b->base      &&
        a->number    == b->number    &&
        a->extension == b->extension) ?
        FSEQ_TRUE :
        FSEQ_FALSE;
}

void fseqFileNameInit(struct FSeqFileName* value)
{
    value->path      = NULL;
    value->base      = NULL;
    value->number    = NULL;
    value->extension = NULL;
}

void fseqFileNameDel(struct FSeqFileName* value)
{
    free(value->path);
    free(value->base);
    free(value->number);
    free(value->extension);

    value->path      = NULL;
    value->base      = NULL;
    value->number    = NULL;
    value->extension = NULL;
}

void fseqFileNameSplit(const char* value, struct FSeqFileName* out, size_t max)
{
    struct FSeqFileNameSizes sizes;
    fseqFileNameSizesInit(&sizes);
    fseqFileNameParseSizes(value, &sizes, max);
    fseqFileNameSplit2(value, &sizes, out);
}

FSeqBool fseqFileNameSplit2(
    const char*                     value,
    const struct FSeqFileNameSizes* sizes,
    struct FSeqFileName*            out)
{
    out->path = (char*)malloc((size_t)sizes->path + 1);
    if (!out->path)
    {
        return FSEQ_FALSE;
    }
    memcpy(out->path, value, sizes->path);
    out->path[sizes->path] = 0;

    out->base = (char*)malloc((size_t)sizes->base + 1);
    if (!out->base)
    {
        return FSEQ_FALSE;
    }
    memcpy(out->base, value + sizes->path, sizes->base);
    out->base[sizes->base] = 0;

    out->number = (char*)malloc((size_t)sizes->number + 1);
    if (!out->number)
    {
        return FSEQ_FALSE;
    }
    memcpy(out->number, value + sizes->path + sizes->base, sizes->number);
    out->number[sizes->number] = 0;

    out->extension = (char*)malloc((size_t)sizes->extension + 1);
    if (!out->extension)
    {
        return FSEQ_FALSE;
    }
    memcpy(out->extension, value + sizes->path + sizes->base + sizes->number, sizes->extension);
    out->extension[sizes->extension] = 0;

    return FSEQ_TRUE;
}

FSeqBool fseqFileNameMatch(
    const char*                     a,
    const struct FSeqFileNameSizes* as,
    const char*                     b,
    const struct FSeqFileNameSizes* bs)
{
    return (
        as->path == bs->path &&
        as->base == bs->base &&
        as->number &&
        bs->number &&
        as->extension == bs->extension &&
        0 == memcmp(a, b, as->path) &&
        0 == memcmp(a + as->path, b + bs->path, as->base) &&
        0 == memcmp(a + as->path + as->base + as->number, b + bs->path + bs->base + bs->number, as->extension)) ?
        FSEQ_TRUE :
        FSEQ_FALSE;
}

void fseqDirEntryInit(struct FSeqDirEntry* value)
{
    fseqFileNameInit(&value->fileName);
    value->frameMin        = 0;
    value->frameMax        = 0;
    value->framePadding    = 0;
    value->next            = NULL;
}

void fseqDirEntryDel(struct FSeqDirEntry* value)
{
    fseqFileNameDel(&value->fileName);
    value->next = NULL;
}

void fseqDirOptionsInit(struct FSeqDirOptions* value)
{
    value->dotAndDotDotDirs = FSEQ_FALSE;
    value->dotFiles         = FSEQ_FALSE;
    value->sequence         = FSEQ_TRUE;
}

void fseqDirEntryToString(
    const struct FSeqDirEntry* value,
    char*                      out,
    FSeqBool                   path,
    size_t                     max)
{
    char format[FSEQ_STRING_LEN];

    assert(value);
    assert(value->framePadding < 10);
    assert(max > 0);
    out[0] = 0;
    const size_t l = strlen(PRId64);
    if (path)
    {
        if (value->fileName.path &&
            value->fileName.base &&
            value->fileName.number &&
            value->fileName.extension)
        {
            if (value->fileName.number[0] &&
                (value->frameMin != value->frameMax))
            {
                size_t i = 0;
                format[i++] = '%';
                format[i++] = 's';
                format[i++] = '%';
                format[i++] = 's';
                format[i++] = '%';
                format[i++] = '0';
                format[i++] = '0' + value->framePadding;
                for (size_t j = 0; j < l; ++j)
                {
                    format[i++] = PRId64[j];
                }
                format[i++] = '-';
                format[i++] = '%';
                format[i++] = '0';
                format[i++] = '0' + value->framePadding;
                for (size_t j = 0; j < l; ++j)
                {
                    format[i++] = PRId64[j];
                }
                format[i++] = '%';
                format[i++] = 's';
                format[i++] = 0;
                snprintf(
                    out,
                    max,
                    format,
                    value->fileName.path,
                    value->fileName.base,
                    value->frameMin,
                    value->frameMax,
                    value->fileName.extension);
            }
            else if (value->fileName.number[0])
            {
                size_t i = 0;
                format[i++] = '%';
                format[i++] = 's';
                format[i++] = '%';
                format[i++] = 's';
                format[i++] = '%';
                format[i++] = '0';
                format[i++] = '0' + value->framePadding;
                for (size_t j = 0; j < l; ++j)
                {
                    format[i++] = PRId64[j];
                }
                format[i++] = '%';
                format[i++] = 's';
                format[i++] = 0;
                snprintf(
                    out,
                    max,
                    format,
                    value->fileName.path,
                    value->fileName.base,
                    value->frameMin,
                    value->fileName.extension);
            }
            else
            {
                size_t i = 0;
                format[i++] = '%';
                format[i++] = 's';
                format[i++] = '%';
                format[i++] = 's';
                format[i++] = '%';
                format[i++] = 's';
                format[i++] = 0;
                snprintf(
                    out,
                    max,
                    format,
                    value->fileName.path,
                    value->fileName.base,
                    value->fileName.extension);
            }
        }
    }
    else
    {
        if (value->fileName.base &&
            value->fileName.number &&
            value->fileName.extension)
        {
            if (value->fileName.number[0] &&
                (value->frameMin != value->frameMax))
            {
                size_t i = 0;
                format[i++] = '%';
                format[i++] = 's';
                format[i++] = '%';
                format[i++] = '0';
                format[i++] = '0' + value->framePadding;
                for (size_t j = 0; j < l; ++j)
                {
                    format[i++] = PRId64[j];
                }
                format[i++] = '-';
                format[i++] = '%';
                format[i++] = '0';
                format[i++] = '0' + value->framePadding;
                for (size_t j = 0; j < l; ++j)
                {
                    format[i++] = PRId64[j];
                }
                format[i++] = '%';
                format[i++] = 's';
                format[i++] = 0;
                snprintf(
                    out,
                    max,
                    format,
                    value->fileName.base,
                    value->frameMin,
                    value->frameMax,
                    value->fileName.extension);
            }
            else if (value->fileName.number[0])
            {
                size_t i = 0;
                format[i++] = '%';
                format[i++] = 's';
                format[i++] = '%';
                format[i++] = '0';
                format[i++] = '0' + value->framePadding;
                for (size_t j = 0; j < l; ++j)
                {
                    format[i++] = PRId64[j];
                }
                format[i++] = '%';
                format[i++] = 's';
                format[i++] = 0;
                snprintf(
                    out,
                    max,
                    format,
                    value->fileName.base,
                    value->frameMin,
                    value->fileName.extension);
            }
            else
            {
                size_t i = 0;
                format[i++] = '%';
                format[i++] = 's';
                format[i++] = '%';
                format[i++] = 's';
                format[i++] = 0;
                snprintf(
                    out,
                    max,
                    format,
                    value->fileName.base,
                    value->fileName.extension);
            }
        }
    }
}

static void _fseqSetError(FSeqBool* error)
{
    if (error)
    {
        *error = FSEQ_TRUE;
    }
}

struct _FSeqDirEntry
{
    char*                    fileName;
    struct FSeqFileNameSizes sizes;
    int64_t                  frameMin;
    int64_t                  frameMax;
    char                     framePadding;
    struct _FSeqDirEntry*    next;
};

static int64_t _toInt64(const char* value)
{
    size_t max = 0;
    for (; '0' == value[0] && value[1] && max < 256; ++value, ++max)
        ;
    return strtoll(value, NULL, 0);
}

static struct _FSeqDirEntry* _fseqDirEntryCreate(
    const char*                     fileName,
    size_t                          fileNameLen,
    const struct FSeqFileNameSizes* sizes)
{
    struct _FSeqDirEntry* out = (struct _FSeqDirEntry*)malloc(sizeof(struct _FSeqDirEntry));
    if (!out)
    {
        return NULL;
    }

    out->fileName = (char*)malloc((fileNameLen + 1) * sizeof(char));
    if (!out->fileName)
    {
        free(out);
        return NULL;
    }
    memcpy(out->fileName, fileName, fileNameLen);
    out->fileName[fileNameLen] = 0;
    
    out->sizes = *sizes;
    if (sizes->number)
    {
        char buf[FSEQ_STRING_LEN];
        memcpy(buf, fileName + sizes->path + sizes->base, sizes->number);
        buf[sizes->number] = 0;
        out->frameMin = out->frameMax = _toInt64(buf);
        if ('0' == fileName[sizes->number] && _IS_NUMBER(fileName[sizes->number + 1]))
        {
            out->framePadding = (char)FSEQ_MIN(sizes->number, 255);
        }
        else
        {
            out->framePadding = 0;
        }
    }
    else
    {
        out->frameMin     = 0;
        out->frameMax     = 0;
        out->framePadding = 0;
    }

    out->next = NULL;
    return out;
}

static void _fseqDirEntryDel(struct _FSeqDirEntry* value)
{
    free(value->fileName);
    value->fileName = NULL;
    value->next     = NULL;
}

#define _IS_DOT_DIR(V, LEN) \
    (1 == LEN && '.' == V[0])
#define _IS_DOT_DOT_DIR(V, LEN) \
    (2 == LEN && '.' == V[0] && '.' == V[1])

struct FSeqDirEntry* fseqDirList(
    const char*                  path,
    const struct FSeqDirOptions* options,
    FSeqBool*                    error)
{
    struct FSeqDirEntry*  out        = NULL;
    struct FSeqDirEntry*  entry      = NULL;
    struct _FSeqDirEntry* _entries   = NULL;
    struct _FSeqDirEntry* _entry     = NULL;
    struct _FSeqDirEntry* _lastEntry = NULL;
    struct FSeqDirOptions _options;
#if defined(WIN32) || defined(_WIN32)
    char                  glob[FSEQ_STRING_LEN];
    size_t                pathLen    = 0;
    int                   wLen       = 0;
    wchar_t*              wBuf       = NULL;
    WIN32_FIND_DATAW      ffd;
    HANDLE                hFind      = NULL;
#else
    DIR*                 dir = NULL;
    const struct dirent* de  = NULL;
#endif

    if (!options)
    {
        fseqDirOptionsInit(&_options);
        options = &_options;
    }

#if defined(WIN32) || defined(_WIN32)

    pathLen = strlen(path);
    memcpy(glob, path, pathLen);
    glob[pathLen] = '\\';
    glob[pathLen + 1] = '*';
    glob[pathLen + 2] = 0;

    wLen = MultiByteToWideChar(CP_UTF8, 0, glob, -1, NULL, 0);
    wBuf = malloc(wLen * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, glob, -1, wBuf, wLen);

    hFind = FindFirstFileW(wBuf, &ffd);
    if (INVALID_HANDLE_VALUE == hFind)
    {
        free(wBuf);
        _fseqSetError(error);
        return NULL;
    }

    do
    {
        int                      fileNameByteCount = 0;
        char*                    fileNameBuf       = NULL;
        struct FSeqFileNameSizes sizes;
        unsigned short           fileNameLen       = 0;
        FSeqBool                 filter            = FSEQ_FALSE;

        fileNameByteCount = WideCharToMultiByte(CP_UTF8, 0, ffd.cFileName, -1, NULL, 0, NULL, NULL);
        fileNameBuf = malloc(fileNameByteCount);
        WideCharToMultiByte(CP_UTF8, 0, ffd.cFileName, -1, fileNameBuf, fileNameByteCount, NULL, NULL);

        fseqFileNameSizesInit(&sizes);
        fileNameLen = fseqFileNameParseSizes(fileNameBuf, &sizes, FSEQ_STRING_LEN);

        // Filter the entry.
        if (!options->dotAndDotDotDirs && _IS_DOT_DIR(fileNameBuf, fileNameLen))
        {
            filter = FSEQ_TRUE;
        }
        else if (!options->dotAndDotDotDirs && _IS_DOT_DOT_DIR(fileNameBuf, fileNameLen))
        {
            filter = FSEQ_TRUE;
        }
        else if (!options->dotFiles && sizes.base)
        {
            filter |= '.' == *(fileNameBuf + sizes.path);
        }

        if (!filter)
        {
            if (!_entries)
            {
                // Create the first entry in the list.
                _entries = _fseqDirEntryCreate(fileNameBuf, fileNameLen, &sizes);
                if (!_entries)
                {
                    _fseqSetError(error);
                    break;
                }
                _lastEntry = _entries;
            }
            else
            {
                _entry = NULL;

                if (options->sequence && sizes.number > 0)
                {
                    // Check if this entry matches any in the list.
                    _entry = _entries;
                    while (_entry)
                    {
                        if (fseqFileNameMatch(fileNameBuf, &sizes, _entry->fileName, &_entry->sizes))
                        {
                            char buf[FSEQ_STRING_LEN];
                            int64_t number = 0;

                            memcpy(buf, fileNameBuf + sizes.path + sizes.base, sizes.number);
                            buf[sizes.number] = 0;
                            number = _toInt64(buf);

                            //printf("number: %" PRId64 "\n", number);

                            _entry->frameMin = FSEQ_MIN(_entry->frameMin, number);
                            _entry->frameMax = FSEQ_MAX(_entry->frameMax, number);
                            if ('0' == buf[0] && _IS_NUMBER(buf[1]))
                            {
                                _entry->framePadding = (char)FSEQ_MIN(FSEQ_MAX((size_t)_entry->framePadding, sizes.number), 255);
                            }
                            else
                            {
                                _entry->framePadding = 0;
                            }

                            break;
                        }
                        _entry = _entry->next;
                    }
                }

                if (!_entry)
                {
                    // Create a new entry.
                    _lastEntry->next = _fseqDirEntryCreate(fileNameBuf, fileNameLen, &sizes);
                    if (!_lastEntry->next)
                    {
                        _fseqSetError(error);
                        break;
                    }
                    _lastEntry = _lastEntry->next;
                }
            }
        }

        free(fileNameBuf);

    } while (FindNextFileW(hFind, &ffd) != 0);

    FindClose(hFind);

    free(wBuf);

#else

    dir = opendir(path);
    if (!dir)
    {
        _fseqSetError(error);
        return NULL;
    }

    while ((de = readdir(dir)))
    {
        struct FSeqFileNameSizes sizes;
        unsigned short           fileNameLen = 0;
        FSeqBool                 filter      = FSEQ_FALSE;

        fseqFileNameSizesInit(&sizes);
        fileNameLen = fseqFileNameParseSizes(de->d_name, &sizes, FSEQ_STRING_LEN);

        // Filter the entry.
        if (!options->dotAndDotDotDirs && _IS_DOT_DIR(de->d_name, fileNameLen))
        {
            filter = FSEQ_TRUE;
        }
        else if (!options->dotAndDotDotDirs && _IS_DOT_DOT_DIR(de->d_name, fileNameLen))
        {
            filter = FSEQ_TRUE;
        }
        else if (!options->dotFiles && sizes.base)
        {
            filter |= '.' == *(de->d_name + sizes.path);
        }

        if (!filter)
        {
            if (!_entries)
            {
                // Create the first entry in the list.
                _entries = _fseqDirEntryCreate(de->d_name, fileNameLen, &sizes);
                if (!_entries)
                {
                    _fseqSetError(error);
                    break;
                }
                _lastEntry = _entries;
            }
            else
            {
                _entry = NULL;

                if (options->sequence && sizes.number > 0)
                {
                    // Check if this entry matches any already in the list.
                    _entry = _entries;
                    while (_entry)
                    {
                        if (fseqFileNameMatch(de->d_name, &sizes, _entry->fileName, &_entry->sizes))
                        {
                            char buf[FSEQ_STRING_LEN];
                            int64_t number = 0;

                            memcpy(buf, de->d_name + sizes.path + sizes.base, sizes.number);
                            buf[sizes.number] = 0;
                            number = _toInt64(buf);

                            _entry->frameMin = FSEQ_MIN(_entry->frameMin, number);
                            _entry->frameMax = FSEQ_MAX(_entry->frameMax, number);
                            if ('0' == buf[0] && _IS_NUMBER(buf[1]))
                            {
                                _entry->framePadding = (char)FSEQ_MIN(FSEQ_MAX((size_t)_entry->framePadding, sizes.number), 255);
                            }
                            else
                            {
                                _entry->framePadding = 0;
                            }

                            break;
                        }
                        _entry = _entry->next;
                    }
                }

                if (!_entry)
                {
                    // Create a new entry.
                    _lastEntry->next = _fseqDirEntryCreate(de->d_name, fileNameLen, &sizes);
                    if (!_lastEntry->next)
                    {
                        _fseqSetError(error);
                        break;
                    }
                    _lastEntry = _lastEntry->next;
                }
            }
        }
    }

    closedir(dir);

#endif

    // Create the list of FSeqDirEntry structs to return.
    _entry = _entries;
    while (_entry)
    {
        if (!out)
        {
            out = (struct FSeqDirEntry*)malloc(sizeof(struct FSeqDirEntry));
            if (!out)
            {
                _fseqSetError(error);
                break;
            }
            fseqDirEntryInit(out);
            if (!fseqFileNameSplit2(_entry->fileName, &_entry->sizes, &out->fileName))
            {
                _fseqSetError(error);
                break;
            }
            out->frameMin     = _entry->frameMin;
            out->frameMax     = _entry->frameMax;
            out->framePadding = _entry->framePadding;
            entry = out;
        }
        else
        {
            entry->next = (struct FSeqDirEntry*)malloc(sizeof(struct FSeqDirEntry));
            if (!entry->next)
            {
                _fseqSetError(error);
                break;
            }
            fseqDirEntryInit(entry->next);
            if (!fseqFileNameSplit2(_entry->fileName, &_entry->sizes, &entry->next->fileName))
            {
                _fseqSetError(error);
                break;
            }
            entry->next->frameMin     = _entry->frameMin;
            entry->next->frameMax     = _entry->frameMax;
            entry->next->framePadding = _entry->framePadding;
            entry = entry->next;
        }

        _entry = _entry->next;
    }

    // Delete the temoprary list.
    _entry = _entries;
    while (_entry)
    {
        struct _FSeqDirEntry* tmp = _entry;
        _entry = _entry->next;
        _fseqDirEntryDel(tmp);
        free(tmp);
    }

    return out;
}

void fseqDirListDel(struct FSeqDirEntry* value)
{
    while (value)
    {
        struct FSeqDirEntry* tmp = value;
        value = value->next;
        fseqDirEntryDel(tmp);
        free(tmp);
    }
}
