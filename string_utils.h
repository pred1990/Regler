#pragma once
#include "typedefs.h"

//string utils
int32 index_of(char*, char);
bool str_begins_with(char*, char*);
bool substr_begins_with(char*, int32, char*);
void str_cpy(char* dst_chars, char* src_chars, int32 src_end_index);
void str_cpy_substr(char* dst_chars, char* src_chars, int32 src_offset, int32 src_end_index);

#include "string_utils.c"