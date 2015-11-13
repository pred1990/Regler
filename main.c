#include <stdio.h>
#include "number_name_defs.h"

void interpret_all(int32, char**);
int32 index_of(char*, char);

int32 main(int32 argL, char** argV){
	interpret_all(argL, argV);
	return 0;
}

void interpret_all(int32 argL, char** argV){
	for(int32 i = 0; i < argL; ++i){
		int32 index = index_of(argV[i], '=');
		if(index == -1){
			continue;
		}
		//TODO parse kv pair
	}
}

int32 index_of(char* search_str, char find_char){
	int32 i = 0;
	while(search_str[i] && search_str[i] != find_char){
		++i;
  	}
	return search_str[i] ? -1 : i;
}
