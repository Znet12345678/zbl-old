#include "lib.h"
int strcmp(const char *str1,const char *str2){
	if(strlen(str1) != strlen(str2))
		return -1;
	for(int i = 0; i < strlen(str1);i++){
		if(str1[i] != str2[i])
			return ++i;
	}
	return 0;
}
int strncmp(const char *str1,const char *str2,int n){
	for(int i = 0; i < n; i++)
		if(str1[i] != str2[i])
			return -1;
	return 0;
}
