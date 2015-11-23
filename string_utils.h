#pragma once
#include "typedefs.h"

//string utils
int32 index_of(char*, char);
int32 index_of_ignore_terminate(char*, uint32, char);
bool str_begins_with(char*, char*);
bool substr_begins_with(char*, uint32, char*);
void str_cpy(char* dst_chars, char* src_chars);
void str_cpy_substr(char* dst_chars, char* src_chars, uint32 src_offset, uint32 src_end_index);

#include "string_utils.c"