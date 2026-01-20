#include "stdafx.h"

void AssertMessage(const char* expression, const char* message, const char* file, int line, const char* function)
{
    std::cerr << expression << " failed. " << message << "\n"
        << "Function: " << function << "\n"
        << "File: " << file << " at Line" << line << "\n"
        << std::endl;
}
