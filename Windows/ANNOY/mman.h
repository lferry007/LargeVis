
// This is from https://code.google.com/p/mman-win32/
// 
// Licensed under MIT

#ifndef _MMAN_WIN32_H
#define _MMAN_WIN32_H

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <sys/types.h>
#include <windows.h>
#include <errno.h>
#include <io.h>

#define PROT_NONE       0
#define PROT_READ       1
#define PROT_WRITE      2
#define PROT_EXEC       4

#define MAP_FILE        0
#define MAP_SHARED      1
#define MAP_PRIVATE     2
#define MAP_TYPE        0xf
#define MAP_FIXED       0x10
#define MAP_ANONYMOUS   0x20
#define MAP_ANON        MAP_ANONYMOUS

#define MAP_FAILED      ((void *)-1)

/* Flags for msync. */
#define MS_ASYNC        1
#define MS_SYNC         2
#define MS_INVALIDATE   4

#ifndef FILE_MAP_EXECUTE
#define FILE_MAP_EXECUTE    0x0020
#endif

static int __map_mman_error(const DWORD err, const int deferr);
static DWORD __map_mmap_prot_page(const int prot);
static DWORD __map_mmap_prot_file(const int prot);
void* mmap(void *addr, size_t len, int prot, int flags, int fildes, long long off);
int munmap(void *addr, size_t len);
int mprotect(void *addr, size_t len, int prot);
int msync(void *addr, size_t len, int flags);
int mlock(const void *addr, size_t len);
int munlock(const void *addr, size_t len);

#endif