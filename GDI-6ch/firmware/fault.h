#pragma once

#include <cstdint>

enum class Fault : uint8_t
{
    None = 0,

};


bool HasFault();
Fault GetCurrentFault();
