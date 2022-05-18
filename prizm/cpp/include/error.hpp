
/* 
Frank Bender
Prizm software testing
5-5-2022: May make a c only version of this file to support portability 
*/

#ifndef ERROR_H_
#define ERROR_H_

#include <string.h>
#include <stdio.h>
#include <ctime>
#include "file_system.hpp"

#include <vector>
#include <string>

static int prizm_err = -1;

enum PrizmErrType {
    EINVARG, // invalid argument
    EUNDEFREF, // undefined reference
    EDNE, // does not exist
    EMIGERR, // migration error
    EDBE, // database error
    ENULLPTR, // nullptr
    ECOMPILE, // compilation error
    ESERVER // Network error
};

enum PrizmLogType {
    LFAIL,
    LINFO,
    LERR,
    LWARN,
    LDB,
    LENTITY,
    LSERVER,
};

// C++ CODE
static std::vector<std::string> err_labelsz = {
    "INVALID_ARGUMENT",
    "UNDEFINED_REFERENCE",
    "DOES NOT EXIST",
    "MIGRATION ERROR",
    "DATABASE ACCESS ERROR",
    "NULLPTR ACCESSED",
    "COMPILATION FAILED",
    "SERVER ERROR"
};

struct PrizmErr {
    enum PrizmErrType type = EDNE;
    int line = 0;
    char file[32] = "";
    char msg[256] = "";
    char func[64] = "";

    void setFile(const char* _file) {
        if (strlen(_file) > 32) {
            printf("PrizmErrType buffer overflow on file name\n");
        }
        strncpy(file, _file, 32);
    }

    void setMsg(const char* _msg) {
        if (strlen(_msg) > 256) {
            printf("PrizmErrType buffer overflow on message\n");
        }
        strncpy(msg, _msg, 256);
    }

    void setFunc(const char* _func) {
        if (strlen(_func) > 64) {
            printf("PrizmErrType buffer overflow on function name\n");
        }
        strncpy(func, _func, 64);
    }

    void print() {
        printf("PRIZM ERROR::%s -- %s @ %s (LINE:%i) %s\n", err_labelsz.at((int)type).c_str(), file, func, line, msg);
    }

};

static PrizmErr global_prizm_err;

void prizm_error_construct(PrizmErr* err, enum PrizmErrType _type, const char* _file, const char* _func, int _line, const char* _msg);

#define PERR(_type, _msg) prizm_error_construct(&global_prizm_err, _type, __FILE__, __PRETTY_FUNCTION__, __LINE__, _msg); \
            printf("\033[1;31m"); global_prizm_err.print(); printf("\033[0m"); PLOG(LERR, _msg);

#define PFAIL(_type, _msg) prizm_error_construct(&global_prizm_err, _type, __FILE__, __PRETTY_FUNCTION__, __LINE__, _msg); \
            printf("\033[1;31m"); global_prizm_err.print(); printf("\033[0m"); PLOG(LFAIL, _msg); exit(1);

#define PWARN(_type, _msg) prizm_error_construct(&global_prizm_err, _type, __FILE__, __PRETTY_FUNCTION__, __LINE__, _msg); \
            printf("\033[1;35m"); global_prizm_err.print(); printf("\033[0m"); PLOG(LWARN, _msg);

void prizm_log(PrizmLogType type, const char* _file, const char* _func, int _line, const char* msg);

#define PLOG(_type, _msg) prizm_log(_type, __FILE__, __PRETTY_FUNCTION__, __LINE__, _msg);

#endif