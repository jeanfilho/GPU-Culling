#pragma once

#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>

#include <vector>
#include <array>
#include <string>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <limits>
#include <numeric>

#include <cctype>
#include <cstdint>
#include <cassert>
#include <cstdlib>
#undef assert

void AssertMessage(const char* expression, const char* message, const char* file, int line, const char* function);
#define assert(expression) ((expression) ? (void)0 : AssertMessage(#expression, "Assertion failed", __FILE__, __LINE__, __func__))
#define assertm(expression, msg) ((expression) ? (void)0 : AssertMessage(#expression, msg, __FILE__, __LINE__, __func__))
#define assertfail() assert(false)