
/* 
Frank Bender
Prizm software testing
5-5-2022: May make a c only version of this file to support portability 
5-20-2022: Converted to c for python ctypes and portability 
*/

#ifndef ERROR_H_
#define ERROR_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>

#include "util/file_system_c.h"
#include "util/print_color.h"

typedef enum ErrType {
    ENULL, // should never happen
    EINVARG, // invalid argument
    EUNDEFREF, // undefined reference
    EDNE, // does not exist
    EMIG, // migration error
    EDB, // database error
    ENEM, // not enough memory
    ENULLPTR, // nullptr
    ECOMPILE, // compilation error
    ESERVER, // Network error
    ETHREAD, // Thread error
    EWS, // Websocket error
    EFR, // Frame error
    ESOCK, // Socket error
    ECONN
} ErrType;

typedef enum LogType {
    LFAIL,
    LINFO,
    LERR,
    LWARN,
    LDB,
    LENTITY,
    LSERVER,
    LTHREAD
} LogType;

#define ENULL_MSG "Null Error!"
#define EINVARG_MSG "Invalid Argument!"
#define EUNDEFREF_MSG "Undefined Reference!"
#define EDNE_MSG "Does Not exist!"
#define EMIG_MSG "Migration Error!"
#define EDB_MSG "Database Error!"
#define ENEM_MSG "Not Enough Memory!"
#define ENULLPTR_MSG "Nullptr Error!"
#define ECOMPILE_MSG "Compilation Error!"
#define ESERVER_MSG "Server Error!"
#define ETHREAD_MSG "Thread Error!"
#define EWS_MSG "Web Socket Error!"
#define EFR_MSG "Frame Error!"
#define ESOCK_MSG "Socket Error!"
#define ECONN_MSG "Connection Error!"

const char* log_get_label(LogType type);
const char* err_get_label(ErrType type);

typedef struct Err {
    ErrType type;
    int line;
    char file[256];
    char msg[256];
    char func[256];
} Err;

void err_set_file(Err* err, const char* file);
void err_set_msg(Err* err, const char* msg);
void err_set_func(Err* err, const char* func);
void err_print(Err* err, int error);
void err_construct(Err* err, ErrType type, const char* file, const char* func, 
                int line, const char* msg);
void log_construct(LogType _type, const char* file, const char* func, int line, const char* msg);

static int seg_helper = 0;

// not re-entrant safe
// not thread safe
static Err* err;
static char error_buffer[128];
static pthread_mutex_t error_mutex = PTHREAD_MUTEX_INITIALIZER;

#define PLOG(_type, ...) \
    err = (Err*)malloc(sizeof(Err)); \
    snprintf(error_buffer, 128, __VA_ARGS__); \
    log_construct(_type, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    free(err);

#define PERR(_type, ...) \
    err = (Err*)malloc(sizeof(Err)); \
    snprintf(error_buffer, 128, __VA_ARGS__); \
    err_construct(err, _type, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    err_print(err, 1); \
    log_construct(LERR, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    free(err);

#define PFAIL(_type, ...) \
    err = (Err*)malloc(sizeof(Err)); \
    snprintf(error_buffer, 128, __VA_ARGS__); \
    err_construct(err, _type, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    err_print(err, 1); \
    log_construct(LFAIL, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    free(err); \
    exit(1);

#define PWARN(_type, ...) \
    err = (Err*)malloc(sizeof(Err)); \
    snprintf(error_buffer, 128, __VA_ARGS__); \
    err_construct(err, _type, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    err_print(err, 0); \
    log_construct(LWARN, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    free(err);

// thread safe, probably not re-entrant safe
#define PLOG_T(...) \
    pthread_mutex_lock(&error_mutex); \
    err = (Err*)malloc(sizeof(Err)); \
    snprintf(error_buffer, 128, __VA_ARGS__); \
    log_construct(LTHREAD, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    free(err); \
    pthread_mutex_unlock(&error_mutex);

#define PERR_T(...) \
    pthread_mutex_lock(&error_mutex); \
    err = (Err*)malloc(sizeof(Err)); \
    snprintf(error_buffer, 128, __VA_ARGS__); \
    err_construct(err, ETHREAD, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    err_print(err, 0); \
    log_construct(LTHREAD, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    free(err); \
    pthread_mutex_unlock(&error_mutex);

#define _DEBUG

#ifdef _DEBUG
#define DEBUG(...) \
    BCYA(__VA_ARGS__)
#else
#define DEBUG(...) ;
#endif

#define SEGH \
    BYEL("Segfault helper: HERE: %i\n", seg_helper); \
    seg_helper++;

#endif