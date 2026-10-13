#pragma once

#include "../lib/hw.h"

void* operator new(size_t size);
void operator delete(void* ptr) noexcept;

void* operator new[](size_t size);
void operator delete[](void* ptr) noexcept;

void* operator new(size_t size, void* ptr);