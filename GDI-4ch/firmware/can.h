#pragma once

#include <cstdint>
#include <cstddef>

int canGetOutputCanIdBase(size_t chip);
int canGetInputCanIdBase(size_t chip);

void InitCan();
