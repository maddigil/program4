#ifndef SQLITE3_H
#define SQLITE3_H
#include <stdarg.h> /*para la definicion de va_list*/

#ifdef __cplusplus
extern "C"{
    #endif

    #ifndef SQLITE_EXTERN
    #define SQLITE_EXTERN extern
    #endif

    #ifndef SQLITE_API
    #define SQLITE_API
    #endif

    #ifndef SQLITE_CDECL
    #define SQLITE_CDECL
    #endif

    #ifndef SQLITE_APICALL
    #define SQLITE_APICALL
    #endif

    #ifndef SQLITE_STDCALL
    #define SQLITE_STDCALL SQLITE_APICALL
    #endif

    #ifndef SQLITE_CALLBACK
    #define SQLITE_CALLBACK
    #endif

    #ifndef SQLITE_SYSAPI
    #define SQLITE_SYSAPI
    #endif

    #define SQLITE_DEPRECATED
    #define SQLITE_EXPERIMENTAL

    #ifdef SQLITE_VERSION
    #undef SQLITE_VERSION
    #endif

    #ifdef SQLITE_VERSION_NUMBER
    #undef SQLITE_VERSION_NUMBER
    #endif

    #define SQLITE_VERSION "3.51.3"
    #define SQLITE_VERSION_NUMBER 3051003
    /*dia de creacion y hora*/
    #define SQLITE_SOURCE_ID "2026-03-13 10:38:09 737ae4a34738ffa0c3ff7f9bb18df914dd1cad163f28fd6b6e114a344fe6d618"
    #define SQLITE_SCM_BRANCH "branch-3.51"
    #define SQLITE_SCM_TAGS "release version-3.51.3"
    #define SQLITE_SCM_DATETIME "2026-03-13T10:38:09.694Z"

    /*declaración de funciones*/
    SQLITE_API SQLITE_EXTERN const char sqlite3_version[];
    SQLITE_API const char *sqlite3_libversion(void);
    SQLITE_API const char *sqlite3_sourceid(void);
    SQLITE_API int sqlite3_libversion_number(void);

    #ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS

    SQLITE_API int sqlite3_compileoption_used(const char *zOptName);
    SQLITE_API const char *sqlite3_compileoption_get(int N);
    #else
    #define sqlite3_compileoption_used(X) 0
    #define sqlite3_compileoption_get(X) ((void*)0)
    #endif

    SQLITE_API int sqlite3_threadsafe(void);

    typedef struct sqlite3 sqlite3;

    #ifdef SQLITE_INT64_TYPE
        typedef SQLITE_INT64_TYPE sqlite_int64;
    #ifdef SQLITE_UINT64_TYPE
        typedef SQLITE_UINT64_TYPE sqlite_uint64;
    #else
        typedef unsigned SQLITE_INT64_TYPE sqlite_uint64;
    #endif

    #elif defined (_MSC_VER) || dfined(__BORLANDC__)
        typedef __int64 sqlite_int64;
        typedef usigned __int64 sqlite_uint64;
    #else
        typedef long long int sqlite_int64;
        typedef unsigned long long int sqlite_uint64;
    #endif
        typedef sqlite_int64 sqlite3_int64;
        typedef sqlite_uint64 sqlite3_uint64;
    

    #ifdef SQLITE_OMIT_FLOATING_POINT
    #define double sqlite3_int64
    #endif

    SQLITE_API int sqlite3_close(sqlite3*);
    SQLITE_API int sqlite3_close_v2(sqlite3*);

    typedef int (*sqlite3_callback)(void*,int,char**,char**);

    SQLITE_API int sqlite3_exec(
        sqlite3*,
        const char *sql,
        int (*callback)(void*,int,char**char**),
        void*,
        char **errmsg
    );

    #define SQLITE_OK 0
    /*codigos de error*/
    #define SQLITE_ERROR 1
    #define SQLITE_INTERNAL 2
    #define SQLITE_PERM 3
    #define SQLITE_ABORT 4
    #define SQLITE_BUSY 5
    #define SQLITE_LOCKED 6
    #define SQLITE_NOMEM 7
    #define SQLITE_READONLY 8
    #define SQLITE_INTERRUPT 9
    #define SQLITE_IOERR 10
    #define SQLITE_CORRUPT 11
    #define SQLITE_NOTFOUND 12
    #define SQLITE_FULL 13
    #define SQLITE_CANTOPEN 14
    #define SQLITE_PROTOCOL 15
    #define SQLITE_EMPTY 16
    #define SQLITE_SCHEMA 17
    #define SQLITE_TOOBIG 18
    #define SQLITE_CONSTRAINT 19
    #define SQLITE_MISMATCH 20
    #define SQLITE_MISUSE 21
    #define SQLITE_NOLFS 22
    #define SQLITE_AUTH 23
    #define SQLITE_FORMAT 24
    #define SQLITE_RANGE 25
    #define SQLITE_NOTADB 26
    #define SQLITE_NOTICE 27
    #define SQLITE_WARNING 28
    #define SQLITE_ROW 100
    #define SQLITE_DONE 101

    #define SQLITE_ERROR_MISSING_COLLSEQ (SQLITE_ERROR | (1<<8))
#define SQLITE_ERROR_RETRY (SQLITE_ERROR | (2<<8))
#define SQLITE_ERROR_SNAPSHOT (SQLITE_ERROR | (3<<8))
#define SQLITE_ERROR_RESERVESIZE (SQLITE_ERROR | (4<<8))
#define SQLITE_ERROR_KEY (SQLITE_ERROR | (5<<8))
#define SQLITE_ERROR_UNABLE (SQLITE_ERROR | (6<<8))
#define SQLITE_IOERR_READ (SQLITE_IOERR | (1<<8))
#define SQLITE_IOERR_SHORT_READ (SQLITE_IOERR | (2<<8))
#define SQLITE_IOERR_WRITE (SQLITE_IOERR | (3<<8))
#define SQLITE_IOERR_FSYNC (SQLITE_IOERR | (4<<8))
#define SQLITE_IOERR_DIR_FSYNC (SQLITE_IOERR | (5<<8))
#define SQLITE_IOERR_TRUNCATE (SQLITE_IOERR | (6<<8))
#define SQLITE_IOERR_FSTAT (SQLITE_IOERR | (7<<8))
#define SQLITE_IOERR_UNLOCK (SQLITE_IOERR | (8<<8))
#define SQLITE_IOERR_RDLOCK (SQLITE_IOERR | (9<<8))
#define SQLITE_IOERR_DELETE (SQLITE_IOERR | (10<<8))
#define SQLITE_IOERR_BLOCKED (SQLITE_IOERR | (11<<8))
#define SQLITE_IOERR_NOMEM (SQLITE_IOERR | (12<<8))
#define SQLITE_IOERR_ACCESS (SQLITE_IOERR | (13<<8))
#define SQLITE_IOERR_CHECKRESERVEDLOCK (SQLITE_IOERR | (14<<8))
#define SQLITE_IOERR_LOCK (SQLITE_IOERR | (15<<8))
#define SQLITE_IOERR_CLOSE (SQLITE_IOERR | (16<<8))
#define SQLITE_IOERR_DIR_CLOSE (SQLITE_IOERR | (17<<8))
#define SQLITE_IOERR_SHMOPEN (SQLITE_IOERR | (18<<8))
#define SQLITE_IOERR_SHMSIZE (SQLITE_IOERR | (19<<8))
#define SQLITE_IOERR_SHMLOCK (SQLITE_IOERR | (20<<8))
#define SQLITE_IOERR_SHMMAP (SQLITE_IOERR | (21<<8))
#define SQLITE_IOERR_SEEK (SQLITE_IOERR | (22<<8))
#define SQLITE_IOERR_DELETE_NOENT (SQLITE_IOERR | (23<<8))
#define SQLITE_IOERR_MMAP (SQLITE_IOERR | (24<<8))
#define SQLITE_IOERR_GETTEMPPATH (SQLITE_IOERR | (25<<8))
#define SQLITE_IOERR_CONVPATH (SQLITE_IOERR | (26<<8))
#define SQLITE_IOERR_VNODE (SQLITE_IOERR | (27<<8))
#define SQLITE_IOERR_AUTH (SQLITE_IOERR | (28<<8))
#define SQLITE_IOERR_BEGIN_ATOMIC (SQLITE_IOERR | (29<<8))
#define SQLITE_IOERR_COMMIT_ATOMIC (SQLITE_IOERR | (30<<8))
#define SQLITE_IOERR_ROLLBACK_ATOMIC (SQLITE_IOERR | (31<<8))
#define SQLITE_IOERR_DATA (SQLITE_IOERR | (32<<8))
#define SQLITE_IOERR_CORRUPTFS (SQLITE_IOERR | (33<<8))
#define SQLITE_IOERR_IN_PAGE (SQLITE_IOERR | (34<<8))
#define SQLITE_IOERR_BADKEY (SQLITE_IOERR | (35<<8))
#define SQLITE_IOERR_CODEC (SQLITE_IOERR | (36<<8))
#define SQLITE_LOCKED_SHAREDCACHE (SQLITE_LOCKED |  (1<<8))
#define SQLITE_LOCKED_VTAB (SQLITE_LOCKED |  (2<<8))
#define SQLITE_BUSY_RECOVERY (SQLITE_BUSY   |  (1<<8))
#define SQLITE_BUSY_SNAPSHOT (SQLITE_BUSY   |  (2<<8))
#define SQLITE_BUSY_TIMEOUT (SQLITE_BUSY   |  (3<<8))
#define SQLITE_CANTOPEN_NOTEMPDIR (SQLITE_CANTOPEN | (1<<8))
#define SQLITE_CANTOPEN_ISDIR (SQLITE_CANTOPEN | (2<<8))
#define SQLITE_CANTOPEN_FULLPATH (SQLITE_CANTOPEN | (3<<8))
#define SQLITE_CANTOPEN_CONVPATH (SQLITE_CANTOPEN | (4<<8))
#define SQLITE_CANTOPEN_DIRTYWAL (SQLITE_CANTOPEN | (5<<8)) /* Not Used */
#define SQLITE_CANTOPEN_SYMLINK (SQLITE_CANTOPEN | (6<<8))
#define SQLITE_CORRUPT_VTAB (SQLITE_CORRUPT | (1<<8))
#define SQLITE_CORRUPT_SEQUENCE (SQLITE_CORRUPT | (2<<8))
#define SQLITE_CORRUPT_INDEX (SQLITE_CORRUPT | (3<<8))
#define SQLITE_READONLY_RECOVERY (SQLITE_READONLY | (1<<8))
#define SQLITE_READONLY_CANTLOCK (SQLITE_READONLY | (2<<8))
#define SQLITE_READONLY_ROLLBACK (SQLITE_READONLY | (3<<8))
#define SQLITE_READONLY_DBMOVED (SQLITE_READONLY | (4<<8))
#define SQLITE_READONLY_CANTINIT (SQLITE_READONLY | (5<<8))
#define SQLITE_READONLY_DIRECTORY (SQLITE_READONLY | (6<<8))
#define SQLITE_ABORT_ROLLBACK (SQLITE_ABORT | (2<<8))
#define SQLITE_CONSTRAINT_CHECK (SQLITE_CONSTRAINT | (1<<8))
#define SQLITE_CONSTRAINT_COMMITHOOK (SQLITE_CONSTRAINT | (2<<8))
#define SQLITE_CONSTRAINT_FOREIGNKEY (SQLITE_CONSTRAINT | (3<<8))
#define SQLITE_CONSTRAINT_FUNCTION (SQLITE_CONSTRAINT | (4<<8))
#define SQLITE_CONSTRAINT_NOTNULL (SQLITE_CONSTRAINT | (5<<8))
#define SQLITE_CONSTRAINT_PRIMARYKEY (SQLITE_CONSTRAINT | (6<<8))
#define SQLITE_CONSTRAINT_TRIGGER (SQLITE_CONSTRAINT | (7<<8))
#define SQLITE_CONSTRAINT_UNIQUE (SQLITE_CONSTRAINT | (8<<8))
#define SQLITE_CONSTRAINT_VTAB (SQLITE_CONSTRAINT | (9<<8))
#define SQLITE_CONSTRAINT_ROWID (SQLITE_CONSTRAINT |(10<<8))
#define SQLITE_CONSTRAINT_PINNED (SQLITE_CONSTRAINT |(11<<8))
#define SQLITE_CONSTRAINT_DATATYPE (SQLITE_CONSTRAINT |(12<<8))
#define SQLITE_NOTICE_RECOVER_WAL (SQLITE_NOTICE | (1<<8))
#define SQLITE_NOTICE_RECOVER_ROLLBACK (SQLITE_NOTICE | (2<<8))
#define SQLITE_NOTICE_RBU (SQLITE_NOTICE | (3<<8))
#define SQLITE_WARNING_AUTOINDEX (SQLITE_WARNING | (1<<8))
#define SQLITE_AUTH_USER (SQLITE_AUTH | (1<<8))
#define SQLITE_OK_LOAD_PERMANENTLY (SQLITE_OK | (1<<8))
#define SQLITE_OK_SYMLINK (SQLITE_OK | (2<<8))

#define SQLITE_OPEN_READONLY 0x00000001
#define SQLITE_OPEN_READWRITE 0x00000002
#define SQLITE_OPEN_CREATE 0x00000004
#define SQLITE_OPEN_DELETEONCLOSE 0x00000008
#define SQLITE_OPEN_EXCLUSIVE 0x00000010
#define SQLITE_OPEN_AUTOPROXY 0x00000020
#define SQLITE_OPEN_URI 0x00000040
#define SQLITE_OPEN_MEMORY 0x00000080
#define SQLITE_OPEN_MAIN_DB 0x00000100
#define SQLITE_OPEN_TEMP_DB 0x00000200
#define SQLITE_OPEN_TRANSIENT_DB 0x00000400
#define SQLITE_OPEN_MAIN_JOURNAL 0x00000800
#define SQLITE_OPEN_TEMP_JOURNAL 0x00001000
#define SQLITE_OPEN_SUBJOURNAL 0x00002000
#define SQLITE_OPEN_SUPER_JOURNAL 0x00004000
#define SQLITE_OPEN_NOMUTEX 0x00008000
#define SQLITE_OPEN_FULLMUTEX 0x00010000
#define SQLITE_OPEN_SHAREDCACHE 0x00020000
#define SQLITE_OPEN_PRIVATECACHE 0x00040000
#define SQLITE_OPEN_WAL 0x00080000
#define SQLITE_OPEN_NOFOLLOW 0x01000000
#define SQLITE_OPEN_EXRESCODE 0x02000000
#define SQLITE_OPEN_MASTER_JOURNAL 0x00004000

#define SQLITE_IOCAP_ATOMIC 0x00000001
#define SQLITE_IOCAP_ATOMIC512 0x00000002
#define SQLITE_IOCAP_ATOMIC1K 0x00000004
#define SQLITE_IOCAP_ATOMIC2K 0x00000008
#define SQLITE_IOCAP_ATOMIC4K 0x00000010
#define SQLITE_IOCAP_ATOMIC8K 0x00000020
#define SQLITE_IOCAP_ATOMIC16K 0x00000040
#define SQLITE_IOCAP_ATOMIC32K 0x00000080
#define SQLITE_IOCAP_ATOMIC64K 0x00000100
#define SQLITE_IOCAP_SAFE_APPEND 0x00000200
#define SQLITE_IOCAP_SEQUENTIAL 0x00000400
#define SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN 0x00000800
#define SQLITE_IOCAP_POWERSAFE_OVERWRITE 0x00001000
#define SQLITE_IOCAP_IMMUTABLE 0x00002000
#define SQLITE_IOCAP_BATCH_ATOMIC 0x00004000
#define SQLITE_IOCAP_SUBPAGE_READ 0x00008000

#define SQLITE_LOCK_NONE 0 
#define SQLITE_LOCK_SHARED 1 
#define SQLITE_LOCK_RESERVED 2  
#define SQLITE_LOCK_PENDING 3 
#define SQLITE_LOCK_EXCLUSIVE 4 

#define SQLITE_SYNC_NORMAL 0x00002
#define SQLITE_SYNC_FULL 0x00003
#define SQLITE_SYNC_DATAONLY 0x00010
};
typedef struct sqlite3_file sqlite3_file;
struct sqlite3_file {
  const struct sqlite3_io_methods *pMethods;
};

typedef struct sqlite3_io_methods sqlite3_io_methods;
struct sqlite3_io_methods {
  int iVersion;
  int (*xClose)(sqlite3_file*);
  int (*xRead)(sqlite3_file*, void*, int iAmt, sqlite3_int64 iOfst);
  int (*xWrite)(sqlite3_file*, const void*, int iAmt, sqlite3_int64 iOfst);
  int (*xTruncate)(sqlite3_file*, sqlite3_int64 size);
  int (*xSync)(sqlite3_file*, int flags);
  int (*xFileSize)(sqlite3_file*, sqlite3_int64 *pSize);
  int (*xLock)(sqlite3_file*, int);
  int (*xUnlock)(sqlite3_file*, int);
  int (*xCheckReservedLock)(sqlite3_file*, int *pResOut);
  int (*xFileControl)(sqlite3_file*, int op, void *pArg);
  int (*xSectorSize)(sqlite3_file*);
  int (*xDeviceCharacteristics)(sqlite3_file*);
  int (*xShmMap)(sqlite3_file*, int iPg, int pgsz, int, void volatile**);
  int (*xShmLock)(sqlite3_file*, int offset, int n, int flags);
  void (*xShmBarrier)(sqlite3_file*);
  int (*xShmUnmap)(sqlite3_file*, int deleteFlag);
  int (*xFetch)(sqlite3_file*, sqlite3_int64 iOfst, int iAmt, void **pp);
  int (*xUnfetch)(sqlite3_file*, sqlite3_int64 iOfst, void *p);
};

#define SQLITE_FCNTL_LOCKSTATE 1
#define SQLITE_FCNTL_GET_LOCKPROXYFILE 2
#define SQLITE_FCNTL_SET_LOCKPROXYFILE 3
#define SQLITE_FCNTL_LAST_ERRNO 4
#define SQLITE_FCNTL_SIZE_HINT 5
#define SQLITE_FCNTL_CHUNK_SIZE 6
#define SQLITE_FCNTL_FILE_POINTER 7
#define SQLITE_FCNTL_SYNC_OMITTED 8
#define SQLITE_FCNTL_WIN32_AV_RETRY 9
#define SQLITE_FCNTL_PERSIST_WAL 10
#define SQLITE_FCNTL_OVERWRITE 11
#define SQLITE_FCNTL_VFSNAME 12
#define SQLITE_FCNTL_POWERSAFE_OVERWRITE 13
#define SQLITE_FCNTL_PRAGMA 14
#define SQLITE_FCNTL_BUSYHANDLER 15
#define SQLITE_FCNTL_TEMPFILENAME 16
#define SQLITE_FCNTL_MMAP_SIZE 18
#define SQLITE_FCNTL_TRACE 19
#define SQLITE_FCNTL_HAS_MOVED 20
#define SQLITE_FCNTL_SYNC 21
#define SQLITE_FCNTL_COMMIT_PHASETWO 22
#define SQLITE_FCNTL_WIN32_SET_HANDLE 23
#define SQLITE_FCNTL_WAL_BLOCK 24
#define SQLITE_FCNTL_ZIPVFS 25
#define SQLITE_FCNTL_RBU 26
#define SQLITE_FCNTL_VFS_POINTER 27
#define SQLITE_FCNTL_JOURNAL_POINTER 28
#define SQLITE_FCNTL_WIN32_GET_HANDLE 29
#define SQLITE_FCNTL_PDB 30
#define SQLITE_FCNTL_BEGIN_ATOMIC_WRITE 31
#define SQLITE_FCNTL_COMMIT_ATOMIC_WRITE 32
#define SQLITE_FCNTL_ROLLBACK_ATOMIC_WRITE 33
#define SQLITE_FCNTL_LOCK_TIMEOUT 34
#define SQLITE_FCNTL_DATA_VERSION 35
#define SQLITE_FCNTL_SIZE_LIMIT 36
#define SQLITE_FCNTL_CKPT_DONE 37
#define SQLITE_FCNTL_RESERVE_BYTES 38
#define SQLITE_FCNTL_CKPT_START 39
#define SQLITE_FCNTL_EXTERNAL_READER 40
#define SQLITE_FCNTL_CKSM_FILE 41
#define SQLITE_FCNTL_RESET_CACHE 42
#define SQLITE_FCNTL_NULL_IO 43
#define SQLITE_FCNTL_BLOCK_ON_CONNECT 44
#define SQLITE_FCNTL_FILESTAT 45

#define SQLITE_GET_LOCKPROXYFILE SQLITE_FCNTL_GET_LOCKPROXYFILE
#define SQLITE_SET_LOCKPROXYFILE SQLITE_FCNTL_SET_LOCKPROXYFILE
#define SQLITE_LAST_ERRNO SQLITE_FCNTL_LAST_ERRNO

typedef struct sqlite3_mutex sqlite3_mutex;
typedef struct sqlite3_api_routines sqlite3_api_routines;
typedef const char *sqlite3_filename;

typedef struct sqlite3_vfs sqlite3_vfs;
typedef void (*sqlite3_syscall_ptr)(void);
struct sqlite3_vfs {
  int iVersion;
  int szOsFile;
  int mxPathname;
  sqlite3_vfs *pNext;
  const char *zName;
  void *pAppData;
  int (*xOpen)(sqlite3_vfs*, sqlite3_filename zName, sqlite3_file*,
               int flags, int *pOutFlags);
  int (*xDelete)(sqlite3_vfs*, const char *zName, int syncDir);
  int (*xAccess)(sqlite3_vfs*, const char *zName, int flags, int *pResOut);
  int (*xFullPathname)(sqlite3_vfs*, const char *zName, int nOut, char *zOut);
  void *(*xDlOpen)(sqlite3_vfs*, const char *zFilename);
  void (*xDlError)(sqlite3_vfs*, int nByte, char *zErrMsg);
  void (*(*xDlSym)(sqlite3_vfs*,void*, const char *zSymbol))(void);
  void (*xDlClose)(sqlite3_vfs*, void*);
  int (*xRandomness)(sqlite3_vfs*, int nByte, char *zOut);
  int (*xSleep)(sqlite3_vfs*, int microseconds);
  int (*xCurrentTime)(sqlite3_vfs*, double*);
  int (*xGetLastError)(sqlite3_vfs*, int, char *);
  int (*xCurrentTimeInt64)(sqlite3_vfs*, sqlite3_int64*);
  int (*xSetSystemCall)(sqlite3_vfs*, const char *zName, sqlite3_syscall_ptr);
  sqlite3_syscall_ptr (*xGetSystemCall)(sqlite3_vfs*, const char *zName);
  const char *(*xNextSystemCall)(sqlite3_vfs*, const char *zName);
};

#define SQLITE_ACCESS_EXISTS 0
#define SQLITE_ACCESS_READWRITE 1
#define SQLITE_ACCESS_READ 2

#define SQLITE_SHM_UNLOCK 1
#define SQLITE_SHM_LOCK 2
#define SQLITE_SHM_SHARED 4
#define SQLITE_SHM_EXCLUSIVE 8

#define SQLITE_SHM_NLOCK 8

SQLITE_API int sqlite3_initialize(void);
SQLITE_API int sqlite3_shutdown(void);
SQLITE_API int sqlite3_os_init(void);
SQLITE_API int sqlite3_os_end(void);

SQLITE_API int sqlite3_config(int, ...);

SQLITE_API int sqlite3_db_config(sqlite3*, int op, ...);

typedef struct sqlite3_mem_methods sqlite3_mem_methods;
struct sqlite3_mem_methods {
  void *(*xMalloc)(int);
  void (*xFree)(void*);
  void *(*xRealloc)(void*,int);
  int (*xSize)(void*);
  int (*xRoundup)(int);
  int (*xInit)(void*);
  void (*xShutdown)(void*);
  void *pAppData;
};

#define SQLITE_CONFIG_SINGLETHREAD 1
#define SQLITE_CONFIG_MULTITHREAD 2
#define SQLITE_CONFIG_SERIALIZED 3
#define SQLITE_CONFIG_MALLOC 4
#define SQLITE_CONFIG_GETMALLOC 5
#define SQLITE_CONFIG_SCRATCH 6
#define SQLITE_CONFIG_PAGECACHE 7
#define SQLITE_CONFIG_HEAP 8
#define SQLITE_CONFIG_MEMSTATUS 9
#define SQLITE_CONFIG_MUTEX 10
#define SQLITE_CONFIG_GETMUTEX 11
#define SQLITE_CONFIG_LOOKASIDE 13
#define SQLITE_CONFIG_PCACHE 14
#define SQLITE_CONFIG_GETPCACHE 15
#define SQLITE_CONFIG_LOG 16
#define SQLITE_CONFIG_URI 17
#define SQLITE_CONFIG_PCACHE2 18
#define SQLITE_CONFIG_GETPCACHE2 19
#define SQLITE_CONFIG_COVERING_INDEX_SCAN 20
#define SQLITE_CONFIG_SQLLOG 21
#define SQLITE_CONFIG_MMAP_SIZE 22
#define SQLITE_CONFIG_WIN32_HEAPSIZE 23
#define SQLITE_CONFIG_PCACHE_HDRSZ 24
#define SQLITE_CONFIG_PMASZ 25
#define SQLITE_CONFIG_STMTJRNL_SPILL 26
#define SQLITE_CONFIG_SMALL_MALLOC 27
#define SQLITE_CONFIG_SORTERREF_SIZE 28
#define SQLITE_CONFIG_MEMDB_MAXSIZE 29
#define SQLITE_CONFIG_ROWID_IN_VIEW 30

#define SQLITE_DBCONFIG_MAINDBNAME 1000
#define SQLITE_DBCONFIG_LOOKASIDE 1001
#define SQLITE_DBCONFIG_ENABLE_FKEY 1002
#define SQLITE_DBCONFIG_ENABLE_TRIGGER 1003
#define SQLITE_DBCONFIG_ENABLE_FTS3_TOKENIZER 1004
#define SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION 1005
#define SQLITE_DBCONFIG_NO_CKPT_ON_CLOSE 1006
#define SQLITE_DBCONFIG_ENABLE_QPSG 1007 
#define SQLITE_DBCONFIG_TRIGGER_EQP 1008 
#define SQLITE_DBCONFIG_RESET_DATABASE 1009 
#define SQLITE_DBCONFIG_DEFENSIVE 1010 
#define SQLITE_DBCONFIG_WRITABLE_SCHEMA 1011
#define SQLITE_DBCONFIG_LEGACY_ALTER_TABLE 1012
#define SQLITE_DBCONFIG_DQS_DML 1013
#define SQLITE_DBCONFIG_DQS_DDL 1014
#define SQLITE_DBCONFIG_ENABLE_VIEW 1015
#define SQLITE_DBCONFIG_LEGACY_FILE_FORMAT 1016
#define SQLITE_DBCONFIG_TRUSTED_SCHEMA 1017
#define SQLITE_DBCONFIG_STMT_SCANSTATUS 1018
#define SQLITE_DBCONFIG_REVERSE_SCANORDER 1019
#define SQLITE_DBCONFIG_ENABLE_ATTACH_CREATE 1020
#define SQLITE_DBCONFIG_ENABLE_ATTACH_WRITE 1021
#define SQLITE_DBCONFIG_ENABLE_COMMENTS 1022
#define SQLITE_DBCONFIG_MAX 1022

SQLITE_API int sqlite3_extended_result_codes(sqlite3*, int onoff);
SQLITE_API sqlite3_int64 sqlite3_last_insert_rowid(sqlite3*);
SQLITE_API void sqlite3_set_last_insert_rowid(sqlite3*,sqlite3_int64);
SQLITE_API int sqlite3_changes(sqlite3*);
SQLITE_API sqlite3_int64 sqlite3_changes64(sqlite3*);
SQLITE_API int sqlite3_total_changes(sqlite3*);
SQLITE_API sqlite3_int64 sqlite3_total_changes64(sqlite3*);
SQLITE_API void sqlite3_interrupt(sqlite3*);
SQLITE_API int sqlite3_is_interrupted(sqlite3*);
SQLITE_API int sqlite3_complete(const char *sql);
SQLITE_API int sqlite3_complete16(const void *sql);
SQLITE_API int sqlite3_busy_handler(sqlite3*,int(*)(void*,int),void*);
SQLITE_API int sqlite3_busy_timeout(sqlite3*, int ms);
SQLITE_API int sqlite3_setlk_timeout(sqlite3*, int ms, int flags);


#define SQLITE_SETLK_BLOCK_ON_CONNECT 0x01

SQLITE_API int sqlite3_get_table(
  sqlite3 *db,
  const char *zSql,
  char ***pazResult,
  int *pnRow,
  int *pnColumn,
  char **pzErrmsg
);
SQLITE_API void sqlite3_free_table(char **result);

SQLITE_API char *sqlite3_mprintf(const char*,...);
SQLITE_API char *sqlite3_vmprintf(const char*, va_list);
SQLITE_API char *sqlite3_snprintf(int,char*,const char*, ...);
SQLITE_API char *sqlite3_vsnprintf(int,char*,const char*, va_list);

SQLITE_API void *sqlite3_malloc(int);
SQLITE_API void *sqlite3_malloc64(sqlite3_uint64);
SQLITE_API void *sqlite3_realloc(void*, int);
SQLITE_API void *sqlite3_realloc64(void*, sqlite3_uint64);
SQLITE_API void sqlite3_free(void*);
SQLITE_API sqlite3_uint64 sqlite3_msize(void*);

SQLITE_API sqlite3_int64 sqlite3_memory_used(void);
SQLITE_API sqlite3_int64 sqlite3_memory_highwater(int resetFlag);

SQLITE_API void sqlite3_randomness(int N, void *P);

SQLITE_API int sqlite3_set_authorizer(
  sqlite3*,
  int (*xAuth)(void*,int,const char*,const char*,const char*,const char*),
  void *pUserData
);

#define SQLITE_DENY 1
#define SQLITE_IGNORE 2

#define SQLITE_CREATE_INDEX 1
#define SQLITE_CREATE_TABLE 2
#define SQLITE_CREATE_TEMP_INDEX 3
#define SQLITE_CREATE_TEMP_TABLE 4
#define SQLITE_CREATE_TEMP_TRIGGER 5
#define SQLITE_CREATE_TEMP_VIEW 6
#define SQLITE_CREATE_TRIGGER 7
#define SQLITE_CREATE_VIEW 8
#define SQLITE_DELETE 9
#define SQLITE_DROP_INDEX 10
#define SQLITE_DROP_TABLE 11
#define SQLITE_DROP_TEMP_INDEX 12
#define SQLITE_DROP_TEMP_TABLE 13
#define SQLITE_DROP_TEMP_TRIGGER 14
#define SQLITE_DROP_TEMP_VIEW 15
#define SQLITE_DROP_TRIGGER 16
#define SQLITE_DROP_VIEW 17
#define SQLITE_INSERT 18
#define SQLITE_PRAGMA 19
#define SQLITE_READ 20
#define SQLITE_SELECT 21
#define SQLITE_TRANSACTION 22
#define SQLITE_UPDATE 23
#define SQLITE_ATTACH 24
#define SQLITE_DETACH 25
#define SQLITE_ALTER_TABLE 26
#define SQLITE_REINDEX 27
#define SQLITE_ANALYZE 28
#define SQLITE_CREATE_VTABLE 29
#define SQLITE_DROP_VTABLE 30
#define SQLITE_FUNCTION 31
#define SQLITE_SAVEPOINT 32
#define SQLITE_COPY 0
#define SQLITE_RECURSIVE 33

SQLITE_API SQLITE_DEPRECATED void *sqlite3_trace(sqlite3*, void(*xTrace)(void*,const char*), void*);
SQLITE_API SQLITE_DEPRECATED void *sqlite3_profile(sqlite3*, void(*xProfile)(void*,const char*,sqlite3_uint64), void*);

#define SQLITE_TRACE_STMT 0x01
#define SQLITE_TRACE_PROFILE 0x02
#define SQLITE_TRACE_ROW 0x04
#define SQLITE_TRACE_CLOSE 0x08

SQLITE_API int sqlite3_trace_v2(
  sqlite3*,
  unsigned uMask,
  int(*xCallback)(unsigned,void*,void*,void*),
  void *pCtx
);

SQLITE_API void sqlite3_progress_handler(sqlite3*, int, int(*)(void*), void*);

SQLITE_API int sqlite3_open(
  const char *filename,
  sqlite3 **ppDb
);
SQLITE_API int sqlite3_open16(
  const void *filename,
  sqlite3 **ppDb
);
SQLITE_API int sqlite3_open_v2(
  const char *filename,
  sqlite3 **ppDb,
  int flags,
  const char *zVfs
);

SQLITE_API const char *sqlite3_uri_parameter(sqlite3_filename z, const char *zParam);
SQLITE_API int sqlite3_uri_boolean(sqlite3_filename z, const char *zParam, int bDefault);
SQLITE_API sqlite3_int64 sqlite3_uri_int64(sqlite3_filename, const char*, sqlite3_int64);
SQLITE_API const char *sqlite3_uri_key(sqlite3_filename z, int N);

SQLITE_API const char *sqlite3_filename_database(sqlite3_filename);
SQLITE_API const char *sqlite3_filename_journal(sqlite3_filename);
SQLITE_API const char *sqlite3_filename_wal(sqlite3_filename);

SQLITE_API sqlite3_file *sqlite3_database_file_object(const char*);

SQLITE_API sqlite3_filename sqlite3_create_filename(
  const char *zDatabase,
  const char *zJournal,
  const char *zWal,
  int nParam,
  const char **azParam
);
SQLITE_API void sqlite3_free_filename(sqlite3_filename);

SQLITE_API int sqlite3_errcode(sqlite3 *db);
SQLITE_API int sqlite3_extended_errcode(sqlite3 *db);
SQLITE_API const char *sqlite3_errmsg(sqlite3*);
SQLITE_API const void *sqlite3_errmsg16(sqlite3*);
SQLITE_API const char *sqlite3_errstr(int);
SQLITE_API int sqlite3_error_offset(sqlite3 *db);

SQLITE_API int sqlite3_set_errmsg(sqlite3 *db, int errcode, const char *zErrMsg);

typedef struct sqlite3_stmt sqlite3_stmt;

SQLITE_API int sqlite3_limit(sqlite3*, int id, int newVal);

#define SQLITE_LIMIT_LENGTH 0
#define SQLITE_LIMIT_SQL_LENGTH 1
#define SQLITE_LIMIT_COLUMN 2
#define SQLITE_LIMIT_EXPR_DEPTH 3
#define SQLITE_LIMIT_COMPOUND_SELECT 4
#define SQLITE_LIMIT_VDBE_OP 5
#define SQLITE_LIMIT_FUNCTION_ARG 6
#define SQLITE_LIMIT_ATTACHED 7
#define SQLITE_LIMIT_LIKE_PATTERN_LENGTH 8
#define SQLITE_LIMIT_VARIABLE_NUMBER 9
#define SQLITE_LIMIT_TRIGGER_DEPTH 10
#define SQLITE_LIMIT_WORKER_THREADS 11

#define SQLITE_PREPARE_PERSISTENT 0x01
#define SQLITE_PREPARE_NORMALIZE 0x02
#define SQLITE_PREPARE_NO_VTAB 0x04
#define SQLITE_PREPARE_DONT_LOG 0x10

SQLITE_API int sqlite3_prepare(
  sqlite3 *db,
  const char *zSql,
  int nByte,
  sqlite3_stmt **ppStmt,
  const char **pzTail 
);
SQLITE_API int sqlite3_prepare_v2(
  sqlite3 *db,
  const char *zSql,
  int nByte,
  sqlite3_stmt **ppStmt,
  const char **pzTail
);
SQLITE_API int sqlite3_prepare_v3(
  sqlite3 *db,
  const char *zSql,
  int nByte,
  unsigned int prepFlags,
  sqlite3_stmt **ppStmt,
  const char **pzTail
);
SQLITE_API int sqlite3_prepare16(
  sqlite3 *db,
  const void *zSql,
  int nByte,
  sqlite3_stmt **ppStmt,
  const void **pzTail
);
SQLITE_API int sqlite3_prepare16_v2(
  sqlite3 *db,
  const void *zSql,
  int nByte,
  sqlite3_stmt **ppStmt,
  const void **pzTail
);
SQLITE_API int sqlite3_prepare16_v3(
  sqlite3 *db,
  const void *zSql,
  int nByte,
  unsigned int prepFlags,
  sqlite3_stmt **ppStmt,
  const void **pzTail
);

SQLITE_API const char *sqlite3_sql(sqlite3_stmt *pStmt);
SQLITE_API char *sqlite3_expanded_sql(sqlite3_stmt *pStmt);
#ifdef SQLITE_ENABLE_NORMALIZE
SQLITE_API const char *sqlite3_normalized_sql(sqlite3_stmt *pStmt);
#endif

SQLITE_API int sqlite3_stmt_readonly(sqlite3_stmt *pStmt);
SQLITE_API int sqlite3_stmt_isexplain(sqlite3_stmt *pStmt);
SQLITE_API int sqlite3_stmt_explain(sqlite3_stmt *pStmt, int eMode);
SQLITE_API int sqlite3_stmt_busy(sqlite3_stmt*);

typedef struct sqlite3_value sqlite3_value;
typedef struct sqlite3_context sqlite3_context;

SQLITE_API int sqlite3_bind_blob(sqlite3_stmt*, int, const void*, int n, void(*)(void*));
SQLITE_API int sqlite3_bind_blob64(sqlite3_stmt*, int, const void*, sqlite3_uint64,void(*)(void*));
SQLITE_API int sqlite3_bind_double(sqlite3_stmt*, int, double);
SQLITE_API int sqlite3_bind_int(sqlite3_stmt*, int, int);
SQLITE_API int sqlite3_bind_int64(sqlite3_stmt*, int, sqlite3_int64);
SQLITE_API int sqlite3_bind_null(sqlite3_stmt*, int);
SQLITE_API int sqlite3_bind_text(sqlite3_stmt*,int,const char*,int,void(*)(void*));
SQLITE_API int sqlite3_bind_text16(sqlite3_stmt*, int, const void*, int, void(*)(void*));
SQLITE_API int sqlite3_bind_text64(sqlite3_stmt*, int, const char*, sqlite3_uint64,void(*)(void*), unsigned char encoding);
SQLITE_API int sqlite3_bind_value(sqlite3_stmt*, int, const sqlite3_value*);
SQLITE_API int sqlite3_bind_pointer(sqlite3_stmt*, int, void*, const char*,void(*)(void*));
SQLITE_API int sqlite3_bind_zeroblob(sqlite3_stmt*, int, int n);
SQLITE_API int sqlite3_bind_zeroblob64(sqlite3_stmt*, int, sqlite3_uint64);

SQLITE_API int sqlite3_bind_parameter_count(sqlite3_stmt*);

SQLITE_API const char *sqlite3_bind_parameter_name(sqlite3_stmt*, int);
SQLITE_API int sqlite3_bind_parameter_index(sqlite3_stmt*, const char *zName);
SQLITE_API int sqlite3_clear_bindings(sqlite3_stmt*);
SQLITE_API int sqlite3_column_count(sqlite3_stmt *pStmt);
SQLITE_API const char *sqlite3_column_name(sqlite3_stmt*, int N);
SQLITE_API const void *sqlite3_column_name16(sqlite3_stmt*, int N);

SQLITE_API const char *sqlite3_column_database_name(sqlite3_stmt*,int);
SQLITE_API const void *sqlite3_column_database_name16(sqlite3_stmt*,int);
SQLITE_API const char *sqlite3_column_table_name(sqlite3_stmt*,int);
SQLITE_API const void *sqlite3_column_table_name16(sqlite3_stmt*,int);
SQLITE_API const char *sqlite3_column_origin_name(sqlite3_stmt*,int);
SQLITE_API const void *sqlite3_column_origin_name16(sqlite3_stmt*,int);

SQLITE_API const char *sqlite3_column_decltype(sqlite3_stmt*,int);
SQLITE_API const void *sqlite3_column_decltype16(sqlite3_stmt*,int);

