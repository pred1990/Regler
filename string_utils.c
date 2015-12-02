#pragma once
#include "string_utils.h"

int32 index_of(char* search_str, char find_char){
  int32 i = 0;
  while(search_str[i] && search_str[i] != find_char){
    ++i;
  }
  return search_str[i] ? i : -1;
}

int32 index_of_ignore_terminate(char* search_str, uint32 str_len, char find_char){
  int32 i = 0;
  while(i < str_len && search_str[i] != find_char){
    ++i;
  }
  return i < str_len ? i : -1;
}

bool str_begins_with(char* a_chars, char* b_chars){
  return substr_begins_with(a_chars, 0, b_chars);
}

bool substr_begins_with(char* a_chars, uint32 a_offset, char* b_chars){
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

void str_cpy_substr(char* dst_chars, char* src_chars, uint32 src_offset, uint32 src_end_index){
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