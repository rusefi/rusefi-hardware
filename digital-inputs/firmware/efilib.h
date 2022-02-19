#pragma once

// C++ helpers go here
namespace efi
{
template <typename T, size_t N>
constexpr size_t size(const T(&)[N]) {
    return N;
}
}

char* itoa10(char *p, int num);