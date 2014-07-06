#include<stdlib.h>
#include<stdio.h>
#include<locale.h>
#include<wchar.h>

#include "news.h"


stopwordNode* loadStopwordFromFile(FILE *stopwordFP){

	setlocale(LC_ALL, "zh_TW.UTF-8");

	wchar_t wchar;
	stopwordNode* stopwordList=NULL;
	stopwordNode* newStopword;

	while((wchar=fgetwc(stopwordFP))!=WEOF){

		if(wchar!='\n'){		
			newStopword = (stopwordNode*)malloc(sizeof(stopwordNode));
			newStopword->stopword = wchar;
			newStopword->next = stopwordList;
			stopwordList = newStopword;
		}
		
	}

	return stopwordList;
}

int isStopword(stopwordNode* stopwordList, wchar_t target_wchar){


	stopwordNode* flag = stopwordList;

	while(flag!=NULL){
		
		if(flag->stopword == target_wchar)
			return 1;

		flag=flag->next;
	}

	return 0;
}
