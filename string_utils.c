#pragma once
#include "string_utils.h"

int32 index_of(char* search_str, char find_char){
  int32 i = 0;
  while(search_str[i] && search_str[i] != find_char){
    ++i;
  }
  return search_str[i] ? i : -1;
}

bool str_begins_with(char* a_chars, char* b_chars){
  return substr_begins_with(a_chars, 0, b_chars);
}

//compares a substring with a proper (i.e. zero-terminated) string
bool substr_begins_with(char* a_chars, int32 a_offset, char* b_chars){
  a_chars += a_offset;
  while(*b_chars){
    if(*a_chars != *b_chars){
      return false;   // different chars or a_chars ended early
    }
    ++a_chars;
    ++b_chars;
  }
  return true;
}

void str_cpy(char* dst_chars, char* src_chars){
  while((*dst_chars = *src_chars)){
    ++dst_chars;
    ++src_chars;
  }
}

//copies a substring from src_chars to dst_chars
//the copied string may zero-terminate before the specified src_end_index is reached
//the copy string will be zero-terminated
//dst_chars needs to be sufficiently big
void str_cpy_substr(char* dst_chars, char* src_chars, int32 src_offset, int32 src_end_index){
  int src_i = src_offset;
  while(src_i < src_end_index){
    *dst_chars = src_chars[src_i];
    if(!src_chars[src_i]){
      return;
    }
    ++dst_chars;
    ++src_i;
  }
  *dst_chars = '\0';
}