// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2021 Darby Johnston
// All rights reserved.

#ifndef FSEQ_H
#define FSEQ_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Boolean type.
typedef char FSeqBool;
#define FSEQ_TRUE 1
#define FSEQ_FALSE 0

// The default string length.
#define FSEQ_STRING_LEN 4096

// Get the minimum or maximum of two values.
#define FSEQ_MIN(A, B) (A < B ? A : B)
#define FSEQ_MAX(A, B) (A > B ? A : B)

// This struct file name component options.
struct FSeqFileNameOptions
{
    FSeqBool negativeNumbers;
    uint8_t  maxNumberDigits;
};
void fseqFileNameOptionsInit(struct FSeqFileNameOptions*);

// This struct provides file name component sizes.
struct FSeqFileNameSizes
{
	unsigned short path;
	unsigned short base;
	unsigned short number;
	unsigned short extension;
};
void fseqFileNameSizesInit(struct FSeqFileNameSizes*);

// Parse the file name component sizes.
// Args:
// * fileName - The file name to be parsed
// * out - The output sizes
// * max - The maximum length of the file name
// * options - The options or NULL
// Returns:
// * The length of the file name
unsigned short fseqFileNameParseSizes(
    const char*                       fileName,
    struct FSeqFileNameSizes*         out,
    size_t                            max,
    const struct FSeqFileNameOptions* options);

// Compare structs.
FSeqBool fseqFileNameSizesCompare(
    const struct FSeqFileNameSizes*,
    const struct FSeqFileNameSizes*);

// This struct provides file name components.
struct FSeqFileName
{
    char* path;
    char* base;
    char* number;
    char* extension;
};
void fseqFileNameInit(struct FSeqFileName*);
void fseqFileNameDel(struct FSeqFileName*);

// Split a file name into components.
// Args:
// * fileName - The file name to be parsed
// * out - The output file name components
// * max - The maximum length of the file name
// * options - The options or NULL
void fseqFileNameSplit(
    const char*                       fileName,
    struct FSeqFileName*              out,
    size_t                            max,
    const struct FSeqFileNameOptions* options);

// Split a file name into components.
// Args:
// * fileName - The file name to be parsed
// * sizes - The sizes
// * out - The output file name components
// Returns:
// * Whether the file name was successfully split
FSeqBool fseqFileNameSplit2(
    const char*                     fileName,
    const struct FSeqFileNameSizes* sizes,
    struct FSeqFileName*            out);

// Test whether two file names are part of the same sequence (all components
// are equal except for the number).
FSeqBool fseqFileNameMatch(
    const char*,
    const struct FSeqFileNameSizes*,
    const char*,
    const struct FSeqFileNameSizes*);

// This struct provides a directory entry.
struct FSeqDirEntry
{
    struct FSeqFileName  fileName;
    int64_t              frameMin;
    int64_t              frameMax;
    uint8_t              framePadding;
    struct FSeqDirEntry* next;
};
void fseqDirEntryInit(struct FSeqDirEntry*);
void fseqDirEntryDel(struct FSeqDirEntry*);

// Convert a directory entry to a string.
// Args:
// * entry - The directory entry
// * out - The output string
// * path - Whether to include the path
// * max - The maximum output string length
void fseqDirEntryToString(
    const struct FSeqDirEntry* entry,
    char*                      out,
    FSeqBool                   path,
    size_t                     max);

// This struct provides directory listing options.
struct FSeqDirOptions
{
    FSeqBool                   dotAndDotDotDirs;
    FSeqBool                   dotFiles;
    FSeqBool                   sequence;
    struct FSeqFileNameOptions fileNameOptions;
};
void fseqDirOptionsInit(struct FSeqDirOptions*);

// List the contents of a directory. Use fseqDirListDel() to delete the list.
// Args:
// * path - The directory path
// * options - The directory listing options, may also pass NULL instead
// * error - Whether any erros occurred, may also pass NULL instead
// Returns:
// * A list of directory entries
struct FSeqDirEntry* fseqDirList(
    const char*                  path,
    const struct FSeqDirOptions* options,
    FSeqBool*                    error);

// Delete a directory list.
void fseqDirListDel(struct FSeqDirEntry*);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // FSEQ_H
