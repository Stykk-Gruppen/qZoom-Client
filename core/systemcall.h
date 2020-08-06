#ifndef SYSTEMCALL_H
#define SYSTEMCALL_H
#include <QString>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

class SystemCall
{
public:
    //SystemCall();
    static QString exec(const char* cmd);
};

#endif // SYSTEMCALL_H
