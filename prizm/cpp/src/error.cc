#include "error.h"

void prizm_error_construct(PrizmErr* err, enum PrizmErrType _type, const char* _file, const char* _func, int _line, const char* _msg) {
    err->setFile(_file);
    err->type = _type;
    err->setMsg(_msg);
    err->line = _line;
    err->setFunc(_func);
}

void prizm_log(PrizmLogType type, const char* _file, const char* _func, int _line, const char* msg) {
    std::string s = "";
    std::string func(_func);
    std::string file(_file);
    std::string line = std::to_string(_line);
    std::string t = std::to_string(std::time(nullptr));
    std::string path;
    std::string m(msg);
    switch(type) {
        case LERR:
            s = "ERR-" + t + "-" + file + "-" + func + "-" + line + " :: " + msg;
            path = "log/err.txt";
            break;
        case LINFO:
            s = "INFO-" + t + "-" + file + "-" + func + "-" + line + " :: " + msg;
            path = "log/info.txt";
            break;
        case LFAIL:
            s = "FAIL-" + t + "-" + file + "-" + func + "-" + line + " :: " + msg;
            path = "log/fail.txt";
            break;
        case LWARN:
            s = "WARN-" + t + "-" + file + "-" + func + "-" + line + " :: " + msg;
            path = "log/warn.txt";
            break;
        case LDB:
            s = "DB-" + t + "-" + file + "-" + func + "-" + line + " :: " + msg;
            path = "log/db.txt";
            break;
        case LSERVER:
            s = "SERVER-" + t + "-" + file + "-" + func + "-" + line + " :: " + msg;
            path = "log/server.txt";
            break;
        case LENTITY:
            s = "ENTITY-" + t + "-" + file + "-" + func + "-" + line + " :: " + msg;
            path = "log/entity.txt";
            break;
        default:
            s = "DEF-" + t + "-" + file + "-" + func + "-" + line + " :: " + msg;
            path = "log/junk.txt";
            break;
    }
    FileSystem::write(path.c_str(), s);
    FileSystem::write("log/master.txt", s);
}