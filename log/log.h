#pragma once

#include "core/core.h"

void slog_disable();
void slog_enable();
void slog_set_output_file( const char* filename );
void slog_reset(); //remove the log file
void slog_close(); //deinit
void slog( const char* format, ... );
