#include<stdlib.h>
#include<stdio.h>
#include<locale.h>
#include<wchar.h>
#include<string.h>
#include<wctype.h>
#include<math.h>

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>


#include "news.h"

#define HASHTABLE_SLOTSIZE 1000000
#define bytesPerMB 1048576
#define SLIDING_WINDOW_SIZE 10

typedef struct wnode{
	wchar_t word;
	int isTerm;
	int breakmark;
	long nextCount;
	float nextRatio;
}wordnode;

typedef struct keynode{
	wchar_t* keyword;
	int count;
	struct keynode * next;
}keywordnode;

keywordnode** getKeywordList(char * kflagStart);

int* convertToWordMap(wchar_t* article,keywordnode** keywordlist);
keywordnode* sortKeywordList(keywordnode* keywordlist);

void coSlidingWindow(int* wordmap, keywordnode** keywordlist,FILE* wfp);

int keywordnode_compare (const void * a, const void * b);
keywordnode* article_countKeyword(wordnode* article_wordlist, int article_wordcount, double filterRate);
void article_outputKeyword(keywordnode* keywordlist);

int keywordCount,wordmapCount;
int main(int argc, char* argv[]){

	FILE *fp;
	setlocale(LC_ALL, "zh_TW.UTF-8");

	wchar_t wstring[20];
	wchar_t term[3]={'\0'};
	long count;

	wchar_t wchar, pre_wchar;



//load into article
	wordnode * article_wordlist;
	int article_wordcount=0;


	keywordnode ** keywordlist;
	keywordnode * keywordlistFlag;


	char *buf, *sflag, *eflag, *bufflag;
	char *bflag, *kflag;
	//char char_buffer[2000];
	char *article_buffer;
	wchar_t *article;
	int *wordmap;
	long content_size;
	
	long buffer_size = 10*bytesPerMB;
	int i;
	size_t frsize=1;
	fpos_t fppos;

	fp = fopen(argv[argc-1],"rb");
	
	if(fp==NULL)
		exit(1);

	FILE *wfp;
	wfp=fopen("coCurrenceCorpus","w");

	if(wfp==NULL)
		exit(1);

	buf = (char*)malloc(buffer_size);

	do{
		fgetpos(fp, &fppos);
		frsize = fread(buf, 1, buffer_size, fp);
		bufflag = buf;

		while((sflag=strstr(bufflag,"@\n@B:"))!=NULL && sflag-buf<frsize){

			eflag=strstr(sflag+2,"@\n@B:");

			if(eflag==NULL){
				if(!feof(fp)){
					fsetpos(fp,&fppos);
					fseek(fp, sflag-buf, SEEK_CUR);
					break;
				}
				else{
					if((bflag=strstr(sflag,"\n@B:"))!=NULL){
					

						kflag = strstr(bflag+2,"\n@K:");
						if(kflag!=NULL && kflag<eflag){
						keywordlist=getKeywordList(kflag);
		
						content_size = kflag-bflag-4;
	
						article_buffer = (char*)malloc(content_size+sizeof(char));
						strncpy(article_buffer, bflag+4, content_size/sizeof(char));
						article_buffer[content_size]='\0';
						
						article = (wchar_t*)malloc(content_size+sizeof(wchar_t));	
						swprintf(article,content_size+sizeof(wchar_t) , L"%s\0", article_buffer);

						//wprintf(L"%ls\t",article);
						
						wordmap=convertToWordMap(article,keywordlist);

						coSlidingWindow(wordmap, keywordlist,wfp);	
							keywordlist=NULL;
							keywordCount=0;
							wordmap=NULL;
							wordmapCount=0;
						/*article_countKeyword(article_wordlist, article_wordcount, filterRate);
						article_outputKeyword(keywordlist);

*/
							
							free(article_buffer);
							free(article);
							free(wordmap);
							free(keywordlist);
							wordmap=NULL;
							keywordlist=NULL;
							keywordCount=0;
							wordmapCount=0;
							//free(article_wordlist);
							//free(keywordlist);
							//article_wordlist=NULL;
							//keywordlist=NULL;
							}
						
					}
					break;
				}

			}
			else{
				if((bflag=strstr(sflag,"\n@B:"))!=NULL){
					//content_size = eflag-bflag-4;
						

						kflag = strstr(bflag+2,"\n@K:");
						if(kflag!=NULL && kflag<eflag){
						keywordlist=getKeywordList(kflag);

						content_size = kflag-bflag-4;

						article_buffer = (char*)malloc(content_size+sizeof(char));
						strncpy(article_buffer, bflag+4, content_size/sizeof(char));
						article_buffer[content_size]='\0';
						
						article = (wchar_t*)malloc((content_size+1)*sizeof(wchar_t));
						swprintf(article,(content_size+1)*sizeof(wchar_t) , L"%s\0", article_buffer);

						//wprintf(L"%ls\t",article);
							
						wordmap = convertToWordMap(article,keywordlist);
	
						coSlidingWindow(wordmap, keywordlist,wfp);
							
						
							free(article_buffer);
							free(article);
							free(wordmap);
							free(keywordlist);
							wordmap=NULL;
							keywordlist=NULL;
							keywordCount=0;
							wordmapCount=0;

						}

				}
	
			}
			bufflag = eflag;

		}

	}while(!feof(fp));

	fclose(wfp);
	fclose(fp);


	
	
	
	return 0;

}

keywordnode** getKeywordList(char * startFlag){
	
	char * endFlag;
	char * keywordStr;
	char *pch;
	int i, keywordStrlen=0;
	keywordnode *klinkedlist,**keywordlist, *tempkeyword, *keywordlistFlag;
	
	keywordCount=0;
	endFlag = strstr(startFlag+4, "\n@");
	keywordStrlen=endFlag-startFlag-4;
	
	if(keywordStrlen==0)
		return NULL;

	keywordStr = (char*)malloc(sizeof(char)*(keywordStrlen+1));
	
	strncpy(keywordStr, startFlag+4, keywordStrlen);
	keywordStr[keywordStrlen]='\0';
	pch = strtok (keywordStr,",");
	
	while (pch != NULL)
	{
		tempkeyword = (keywordnode*)malloc(sizeof(keywordnode));
		
		tempkeyword->keyword = (wchar_t*)malloc(sizeof(wchar_t)*(strlen(pch)+1));
		
		//memcpy ( tempkeyword->keyword, pch, strlen(pch) );
		swprintf(tempkeyword->keyword,sizeof(wchar_t)*(strlen(pch)+1), L"%s\0", pch);
		
		
	//	wprintf(L"%ls\t",tempkeyword->keyword);

		if(klinkedlist==NULL){
			klinkedlist = tempkeyword;
			klinkedlist->next = NULL;
		}
		else{
			tempkeyword->next = klinkedlist;
			klinkedlist = tempkeyword;
		}
		keywordCount++;
		//printf("%s\t",pch);
		
		pch = strtok(NULL, ",");
	}
	//printf("\n");
	/*
	keywordlistFlag = keywordlist;
	while(keywordlistFlag!=NULL){
		wprintf(L"%ls\t", keywordlistFlag->keyword);
		keywordlistFlag = keywordlistFlag->next;
	}
	*/


	keywordlist = (keywordnode**)malloc(sizeof(keywordnode*)*keywordCount);

	for(i=0;i<keywordCount;i++,klinkedlist=klinkedlist->next){
		keywordlist[i]=klinkedlist;
	}

//	qsort(keywordlist, keywordCount, sizeof(keywordnode*), keywordnode_compare);

	int j;
	keywordnode* temp;
	for(i=0;i<keywordCount-1;i++){
		for(j=0;j<keywordCount-i-1;j++){
			if(wcslen(keywordlist[j]->keyword)<wcslen(keywordlist[j+1]->keyword)){
				temp = keywordlist[j];
				keywordlist[j]=keywordlist[j+1];
				keywordlist[j+1]=temp;
			}
		}

	}
/*
	for(i=0;i<keywordCount;i++){
		
		wprintf(L"%ls\t",keywordlist[i]->keyword);
	}
	printf("\n");
*/
	return keywordlist;
}

int* convertToWordMap(wchar_t* article,keywordnode** keywordlist){

	int i,j,isKeyword,keyword_length,wordmap_count=0;
	int article_length = wcslen(article);
	int* wordmap_temp = (int*)malloc(sizeof(int)*article_length);
	int* wordmap;
	wchar_t *wcp, *article_p;

	for(article_p=article,j=0;j<article_length;article_p++,j++){
		
		isKeyword=0;

		for(i=0;i<keywordCount;i++){
			
			keyword_length = wcslen(keywordlist[i]->keyword);
			if(wcsncmp(article_p,keywordlist[i]->keyword,keyword_length)==0){
				isKeyword=1;
				break;
			}
		}

		if(isKeyword){
			wordmap_temp[wordmap_count]=i;
			article_p+=keyword_length-1;
		}
		else{
			wordmap_temp[wordmap_count]=-1;
		}

		wordmap_count++;
	}

	wordmap = (int*)malloc(sizeof(int)*wordmap_count);

	for(i=0;i<wordmap_count;i++){
		wordmap[i]=wordmap_temp[i];
	//printf("%d ",wordmap[i]);
	}
	//printf("\n");

	wordmapCount = wordmap_count;
	return wordmap;

}


void coSlidingWindow(int* wordmap, keywordnode** keywordlist, FILE * wfp){
	int i,j,mainCorpusId;

	if(keywordCount>1){

	for(i=0;i<wordmapCount-1;i++){

		if(wordmap[i]==-1)
			continue;
		 
		mainCorpusId=wordmap[i];

		for(j=i;j<i+SLIDING_WINDOW_SIZE && j<wordmapCount;j++){
	
			if(wordmap[j]!=-1 && wordmap[j]!=mainCorpusId){
			//	wprintf(L"%ls\t%ls\n",keywordlist[mainCorpusId]->keyword, keywordlist[wordmap[j]]->keyword);
				
				fwprintf(wfp, L"%ls\t%ls\n",keywordlist[mainCorpusId]->keyword, keywordlist[wordmap[j]]->keyword);
			}
		}

	}

	}


}


keywordnode* article_countKeyword(wordnode* article_wordlist, int article_wordcount, double filterRate){

	keywordnode * keywordlist=NULL;
	keywordnode * tempKeywordNode;
	keywordnode * keywordlistFlag;
	int keyword_start=-1, keyword_end=-1;
	int i,j,k;
	wchar_t * tempKeyword;
	//double filterRate= (ratio_total/(double)ratio_count)*log10((double)ratio_count);
//	double filterRate = ((ratio_max-(ratio_total/(double)ratio_count)))*2*0.2;
	
	for(i=0;i<article_wordcount;i++){

			if(article_wordlist[i].nextRatio>=filterRate)
			{
				if(keyword_start==-1)
					keyword_start=i;

				keyword_end=i+1;
			}
			else if(keyword_start!=-1){
				tempKeyword = (wchar_t*) malloc(sizeof(wchar_t)*(keyword_end+1-keyword_start+1));
					
				for(j=keyword_start,k=0;j<keyword_end+1;j++,k++)
				{
					tempKeyword[k] = article_wordlist[j].word; 
				}
				tempKeyword[k]='\0';
				
				tempKeywordNode=NULL;
				keywordlistFlag=keywordlist;

				while(keywordlistFlag!=NULL){
						
					if(wcscmp(keywordlistFlag->keyword, tempKeyword)==0){
						tempKeywordNode=keywordlistFlag;
						break;
					}
					keywordlistFlag=keywordlistFlag->next;
				}

				if(tempKeywordNode!=NULL)
					tempKeywordNode->count++;
				else{
					tempKeywordNode = (keywordnode*)malloc(sizeof(keywordnode));
					tempKeywordNode->keyword=tempKeyword;
					tempKeywordNode->count=1;
					tempKeywordNode->next = keywordlist;
					keywordlist=tempKeywordNode;
				}

				keyword_start=-1;
			}

	}
		return keywordlist;

}

void article_outputKeyword(keywordnode* keywordlist){

	keywordnode* keywordlistFlag;
	printf("@K:");
	keywordlistFlag=keywordlist;
	while(keywordlistFlag!=NULL)
	{
		if(keywordlistFlag->next!=NULL)
			wprintf(L"%ls,", keywordlistFlag->keyword, keywordlistFlag->count);
			//wprintf(L"%ls(%d),", keywordlistFlag->keyword, keywordlistFlag->count);
		else	
			wprintf(L"%ls\n", keywordlistFlag->keyword, keywordlistFlag->count);
		//	wprintf(L"%ls(%d)\n", keywordlistFlag->keyword, keywordlistFlag->count);

		keywordlistFlag=keywordlistFlag->next;
	}
}


