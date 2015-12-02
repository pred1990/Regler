#pragma once
#include "typedefs.h"

//finds the first index at which a given char is found
//stops if a string terminator ('\0') is found
//returns -1 if the searched char is not found
int32 index_of(char*, char);

//finds the first index at which a given char is found
//does not stop if a string terminator ('\0') is found
//returns -1 if the searched char is not found
int32 index_of_ignore_terminate(char*, uint32, char);
bool str_begins_with(char*, char*);

//compares a substring with a proper (i.e. zero-terminated) string
bool substr_begins_with(char*, uint32, char*);

//string copy
void str_cpy(char* dst_chars, char* src_chars);

//copies a substring from src_chars to dst_chars
//the copied string may zero-terminate before the specified src_end_index is reached
//the copy string will be zero-terminated
//dst_chars needs to be sufficiently big
void str_cpy_substr(char* dst_chars, char* src_chars, uint32 src_offset, uint32 src_end_index);

#include "string_utils.c"