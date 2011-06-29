//
//  echoprint-codegen
//  Copyright 2011 The Echo Nest Corporation. All rights reserved.
//


#ifndef FILE_H
#define FILE_H
#include <string.h>
#ifdef _WIN32
#include "win_unistd.h"
#endif
/*
This makes file writing a bit easier (makes sure we don't forget to fclose, basically). Use it like this:

bool WriteStuffToFile(const char* filename)
{
    File f(filename);
    if (f)
        fprintf(f, "stuff I want to print: %s", stuff);
    return f; // success/failure
}
*/
class File {
public:
    File(const char* filename){_f = fopen(filename, "w");};
    ~File(){fclose(_f); _f = NULL;}
    operator bool(){return _f != NULL;}
    operator FILE*(){return _f;}
    static bool Exists(const char* filename){return (access(filename, F_OK) == 0);}
    static bool ends_with(const char* filename, const char* ending) {
        int nFilename = strlen(filename);
        int nEnding = strlen(ending);

        bool same = false;
        if (nEnding <= nFilename) {
            const char* file_end = filename + strlen(filename) - strlen(ending);

            for (int i = 0; i < nEnding; i++)
                if (tolower(file_end[i]) != tolower(ending[i])) return false;

            same = true;
        }
        return same;
    }
private:
    FILE* _f;
};

#endif
