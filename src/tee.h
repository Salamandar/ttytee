#pragma once

#include <stdbool.h>
#include <stddef.h>

bool run_tee(const char* tty, const char* ptys[], size_t ptys_count, bool overwrite_ptys);

void clear_tee(void);
