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

#define HASHTABLE_SLOTSIZE 3500000
#define bytesPerMB 1048576

typedef struct nd{
	wchar_t key[3];
	long count;
	float ratio;
	struct nd *next;
	
}node;

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

unsigned int hash33(wchar_t* key, long htsize);

node** create_hashtable(node** nodelist, long slotsize);
node* insert_node(node** nodelist, long slotsize, wchar_t* keyToAdd, long count, float ratio);
node* find_node(node** nodelist, long slotsize, wchar_t* keyToFind);

void hashtable_traverse(node** nodelist, long slotsize);

wordnode* article_loadIntoHt(char* read_buffer, int* article_wordcount, long* term_totalCount,node ** articleHashTable, stopwordNode * stopwordList);
void article_ratioCount(node ** article_nodelist, long slotsize, long term_count);
void article_countWeight(node** articleHashTable, node** hashtable, wordnode* article_wordlist, int article_wordcount, double* ratio_total, int* ratio_count, float* ratio_max);

float kmeans_rate(wordnode* article_wordlist, int article_wordcount, int ratio_count, float ratio_max);

keywordnode* article_countKeyword(wordnode* article_wordlist, int article_wordcount, double filterRate);
void article_outputKeyword(keywordnode* keywordlist);


int main(int argc, char* argv[]){

	FILE *fp;
	setlocale(LC_ALL, "zh_TW.UTF-8");

	wchar_t wstring[20];
	wchar_t term[3]={'\0'};
	long count;
	float ratio;
	node ** hashtable;
	node * tempNode;
	wchar_t wchar, pre_wchar;

	hashtable = create_hashtable(hashtable, HASHTABLE_SLOTSIZE);

	fp = fopen(argv[argc-3],"r");

	while(fgetws(wstring,20,fp)!=NULL){
//		wprintf(L"%lc-%ld\n",wchar,wchar);		
	
		swscanf(wstring, L"%lc%lc %ld %f",&term[0],&term[1], &count, &ratio);
		insert_node(hashtable, HASHTABLE_SLOTSIZE, term, count, ratio);	

	}
	
	fclose(fp);


//load stopwords
	fp = fopen(argv[argc-2],"r");
	stopwordNode * stopwordList = loadStopwordFromFile(fp);
	fclose(fp);

//load into article
	wordnode * article_wordlist;
	node ** articleHashTable;
	int article_wordcount=0;
	long term_totalCount=0;

	double ratio_total=0;
	int ratio_count=0;
	float ratio_max=-1000;
	double filterRate;

	keywordnode * keywordlist;
	keywordnode * keywordlistFlag;


	char *buf, *sflag, *eflag, *bufflag;
	char *bflag;
	//char char_buffer[2000];
	char *article_buffer;
	long content_size;
	long buffer_size = 10*bytesPerMB;
	int i;
	size_t frsize=1;
	fpos_t fppos;

	articleHashTable = create_hashtable(articleHashTable, 2000);

	fp = fopen(argv[argc-1],"rb");
	
	if(fp==NULL)
		exit(1);

	buf = (char*)malloc(buffer_size);

	do{
		fgetpos(fp, &fppos);
		frsize = fread(buf, 1, buffer_size, fp);
		bufflag = buf;

		while((sflag=strstr(bufflag,"@GAISRec:"))!=NULL && sflag-buf<frsize){

			eflag=strstr(sflag+2,"@GAISRec:");

			if(eflag==NULL){
				if(!feof(fp)){
					fsetpos(fp,&fppos);
					fseek(fp, sflag-buf, SEEK_CUR);
					break;
				}
				else{
					if((bflag=strstr(sflag,"\n@B:"))!=NULL){
						content_size = buf+frsize-bflag-4;
						//strncpy(char_buffer, bflag+4, 2000);
						//swprintf(article_buffer, 2000, L"%hs", char_buffer);
						article_buffer = (char*)malloc(content_size+sizeof(char));
						strncpy(article_buffer, bflag+4, content_size/sizeof(char));
						article_buffer[content_size/sizeof(char)]='\0';

						ratio_total=0;
						ratio_count=0;
						ratio_max=-1000;
					
						article_wordlist = article_loadIntoHt(article_buffer, &article_wordcount, &term_totalCount, articleHashTable, stopwordList);
						article_ratioCount(articleHashTable, 2000, term_totalCount);
						article_countWeight(articleHashTable,hashtable,article_wordlist,article_wordcount, &ratio_total, &ratio_count, &ratio_max);
						
						filterRate = kmeans_rate(article_wordlist, article_wordcount, ratio_count, ratio_max);
						//printf("filterRate: %lf\n",filterRate);
						
						//filterRate = ((ratio_total/(double)ratio_count))*4;
						//printf("%lf\n",filterRate);
						
						keywordlist=article_countKeyword(article_wordlist, article_wordcount, filterRate);
						article_outputKeyword(keywordlist);

	//	printf("\ntotal term:%d\nratio ave:%lf\nlog count:%lf\nratio max:%lf\nfilter rate:%lf\n",ratio_count,ratio_total/(double)ratio_count,log10((double)ratio_count),ratio_max, filterRate);

						for(i=0;i<2000;i++)
							articleHashTable[i]=NULL;

	//memset(articleHashTable, 0, sizeof(node*)*2000);
							free(article_wordlist);
							free(keywordlist);
							free(article_buffer);
							article_wordlist=NULL;
							keywordlist=NULL;

					}
					break;
				}

			}
			else{
				if((bflag=strstr(sflag,"\n@B:"))!=NULL){
					content_size = eflag-bflag-4;
					//fwrite(bflag+4, 1, content_size,stdout);
						//strncpy(char_buffer, bflag+4, 2000);
						//swprintf(article_buffer, 2000, L"%hs", char_buffer);
						article_buffer = (char*)malloc(content_size+sizeof(char));
						strncpy(article_buffer, bflag+4, content_size/sizeof(char));
						article_buffer[content_size/sizeof(char)]='\0';

					ratio_total=0;
					ratio_count=0;
					ratio_max=-1000;
					
					article_wordlist =article_loadIntoHt(article_buffer, &article_wordcount, &term_totalCount, articleHashTable, stopwordList);
					article_ratioCount(articleHashTable, 2000, term_totalCount);
					article_countWeight(articleHashTable,hashtable,article_wordlist,article_wordcount, &ratio_total, &ratio_count, &ratio_max);
					
					filterRate = kmeans_rate(article_wordlist, article_wordcount, ratio_count, ratio_max);
					//printf("filterRate: %lf\n",filterRate);
					
					//filterRate = ((ratio_total/(double)ratio_count))*4;
					//printf("%lf\n",filterRate);

					keywordlist=article_countKeyword(article_wordlist, article_wordcount, filterRate);
					article_outputKeyword(keywordlist);
					
//	printf("\ntotal term:%d\nratio ave:%lf\nlog count:%lf\nratio max:%lf\nfilter rate:%lf\n",ratio_count,ratio_total/(double)ratio_count,log10((double)ratio_count),ratio_max, filterRate);

						for(i=0;i<2000;i++)
							articleHashTable[i]=NULL;
							
	//memset(articleHashTable, 0, sizeof(node*)*2000);
							free(article_wordlist);
							free(keywordlist);
							free(article_buffer);
							article_wordlist=NULL;
							keywordlist=NULL;

				}
	
			}
			bufflag = eflag;

		}

	}while(!feof(fp));


	fclose(fp);


	
	
	
	return 0;

}

unsigned int hash33(wchar_t* key, long htsize){

	int i;
	unsigned int hashValue=0;

	for(i=0;i<wcslen(key);i++){
		hashValue=(hashValue<<5)+hashValue+(unsigned int)key[i];
	}
	
	return hashValue%htsize;
}

node** create_hashtable(node** nodelist, long slotsize){
	
	int i;
	nodelist = (node**)malloc(sizeof(node*)*slotsize);
	
	for(i=0;i<slotsize;i++)
		nodelist[i]=NULL;

	//memset(nodelist, 0, sizeof(node*)*slotsize);
	return nodelist;

}


node* insert_node(node** nodelist, long slotsize, wchar_t* keyToAdd, long count, float ratio){

	unsigned int hashkey = hash33(keyToAdd, slotsize);
	node *newNode=(node*)malloc(sizeof(node));

	if(nodelist[hashkey]==NULL){
		nodelist[hashkey] = (node*)malloc(sizeof(node));
		
		wcscpy(newNode->key,keyToAdd);
		//newNode->key[0] = keyToAdd[0];
		//newNode->key[1] = keyToAdd[1];
		newNode->count = count;
		newNode->ratio = ratio;
		newNode->next = NULL;
		nodelist[hashkey]->next = newNode;
	}
	else{
		wcscpy(newNode->key, keyToAdd);
		//newNode->key[0] = keyToAdd[0];
		//newNode->key[1] = keyToAdd[1];
		newNode->count = count;
		newNode->ratio = ratio;
		newNode->next = nodelist[hashkey]->next;
		nodelist[hashkey]->next = newNode;
	}
	//wprintf(L"%lc %lc\n", nodeSlot->next->key[0], nodeSlot->next->key[1]);
	return newNode;
}

node* find_node(node** nodelist, long slotsize, wchar_t* keyToFind){

	unsigned int hashkey = hash33(keyToFind, slotsize);
	node *nodeSlot = nodelist[hashkey];

	if(nodeSlot!=NULL){
		
		nodeSlot = nodeSlot->next;

		while(nodeSlot!=NULL){
			
		//	if(nodeSlot->key[0]==keyToFind[0] && nodeSlot->key[1]==keyToFind[1])
			if(wcscmp(nodeSlot->key, keyToFind)==0)
			{
				return nodeSlot;
			}
			else
				nodeSlot= nodeSlot->next;

		}

	}
	return NULL;
}

void hashtable_traverse(node ** nodelist, long slotsize){

	int row;
	node * temp;

	for(row=0;row<slotsize;row++){

		if(nodelist[row]!=NULL){
			temp = nodelist[row]->next;	

			while(temp!=NULL){
	
				wprintf(L"%ld\t%ls\n",temp->count,temp->key);
				temp = temp->next;
			}
		}
	}

}

wordnode* article_loadIntoHt(char* read_buffer, int* article_wordcount, long* term_totalCount,node ** articleHashTable, stopwordNode * stopwordList){

//load into article
	wchar_t wchar, pre_wchar;
	wordnode* article_wordlist;
	long article_size;
	node * articleTempNode;
	int i;
	wchar_t term[3]={'\0'};
	int bufferflag=0, wordcount=0;

	wchar_t article_buffer[2000];
	wchar_t dest;
	size_t length, max;
	mbstate_t mbs;
	
	printf("@\n@B:%s", read_buffer);
	
	mbrlen( NULL, 0, &mbs);
	max= strlen(read_buffer);
	
	bufferflag=0;
	while(max>0){
		length = mbrtowc(&dest, read_buffer, max, &mbs);
			
		if((length==0)||(length>max) || bufferflag>=2000)
			break;
		
		article_buffer[bufferflag] = dest;
		//wprintf(L"[%lc]", dest);
	//	printf(" max: %d, length: %d\n", max, length);
		read_buffer+=length;
		max-=length;
		bufferflag++;
	}
	
	/*
	printf("%s", read_buffer);
	for (i=0; i<strlen(read_buffer); i++){
			temp_wchar = btowc(read_buffer[i]);
			
			if (temp_wchar != WEOF){
				article_buffer[i] = temp_wchar;
				wprintf(L"'%c'", article_buffer[i]);
			}
			else{
			break;
			}
	}*/
	wprintf(L"%s",article_buffer);

	article_size = wcslen(article_buffer);

	article_wordlist = (wordnode*)malloc(bufferflag*sizeof(wordnode));

	pre_wchar=0;

	wordcount=0;
	for(i=0;i<bufferflag;i++){
		
		article_wordlist[wordcount].word = article_buffer[i];
		wchar = article_buffer[i];
		if(isStopword(stopwordList, wchar)==1 || wchar<=127){
			article_wordlist[wordcount].isTerm = 0;
			article_wordlist[wordcount].breakmark =1;

			if(wordcount!=0){
				article_wordlist[wordcount-1].breakmark=1;
				article_wordlist[wordcount-1].nextRatio=-1;
			}

			pre_wchar = 0;
		}

		else
		{

			if(pre_wchar==0)
				pre_wchar = wchar;
			else{

				term[0]=pre_wchar;
				term[1]= wchar;

				if((articleTempNode=find_node(articleHashTable, 2000, term))!=NULL)
					
					articleTempNode->count++;
				else{
					insert_node(articleHashTable, 2000, term,  1, 0);
					//wprintf(L"[%lc %lc]\n", term[0], term[1]);
				}
				(*term_totalCount)++;
			pre_wchar=wchar;
			}
			article_wordlist[wordcount].isTerm=1;
			article_wordlist[wordcount].breakmark=1;
		}

		article_wordlist[wordcount].nextCount = -1;
		article_wordlist[wordcount].nextRatio = -1;
		
		//wprintf(L"[%lc]", article_wordlist[wordcount].word);
		wordcount++;
	}
	article_wordlist[wordcount-1].breakmark=1;

		*article_wordcount =wordcount;
		
		return article_wordlist;
}

void article_countWeight(node** articleHashTable, node** hashtable, wordnode* article_wordlist, int article_wordcount, double* ratio_total, int* ratio_count, float* ratio_max){

	wordnode* articleword;
	wordnode* articleword_next;

	int i,j;
	node * articleTempNode;
	node * tempNode;
	wchar_t term[3]={'\0'};

	for(i=0;i<article_wordcount-1;i++){

		if(article_wordlist[i].isTerm!=0 && article_wordlist[i+1].isTerm!=0){
		
		articleword = &article_wordlist[i];
//wprintf(L"---%lc\n",article_wordlist[i].word);
		articleword_next=NULL;

		for(j=i+1;j<article_wordcount;j++){
			if(article_wordlist[j].isTerm!=0){
				articleword_next = &article_wordlist[j];
//wprintf(L"----%lc\n",article_wordlist[j].word);

				break;
			}
		}

		if(articleword_next!=NULL){
			term[0]=articleword->word;
			term[1]=articleword_next->word;
					//wprintf(L"{%lc %lc}\n", term[0], term[1]);

			articleTempNode = find_node(articleHashTable, 2000, term);

			if((tempNode=find_node(hashtable, HASHTABLE_SLOTSIZE, term))!=NULL){
				articleword->nextCount = articleTempNode->count;
				articleword->nextRatio = tempNode->ratio * articleTempNode->ratio;
			}
			else{
				articleword->nextCount = articleTempNode->count;
				articleword->nextRatio = 0.0;
			}

			(*ratio_total)+=articleword->nextRatio;
			(*ratio_count)+=articleword->nextCount;
			
			if(articleword->nextRatio>(*ratio_max))
				(*ratio_max) = articleword->nextRatio;

		}

		}

	}

}

void article_ratioCount(node ** article_nodelist, long slotsize, long term_count){
	int row;
	node * temp;

	for(row=0;row<slotsize;row++){

		if(article_nodelist[row]!=NULL){
			temp = article_nodelist[row]->next;	

			while(temp!=NULL){
	
				//wprintf(L"%ld\t%lc%lc\n",temp->count,temp->key[0],temp->key[1]);
				temp->ratio = (float) temp->count/ (float)term_count;
				temp = temp->next;
			}
		}
	}

}

float kmeans_rate(wordnode* article_wordlist, int article_wordcount, int ratio_count, float ratio_max){

	float hInit, lInit;
	float inputs[article_wordcount];
	float hDistance, lDistance;
	float hGroupSum, lGroupSum;
	int	hGroupCount, lGroupCount;
	
	float minOfHGroup;
	
	int i,j;
	
	hInit = ratio_max;
	lInit = ratio_max/2;
	
	for(i=0;i<article_wordcount-1;i++){
		inputs[i] = article_wordlist[i].nextRatio;
	}
	
	//set max k-means iteration to 100
	for(i=0;i<100;i++){
		
		hGroupSum=0;
		lGroupSum=0;
		hGroupCount=0;
		lGroupCount=0;
		minOfHGroup=1000;
		//printf("iterate %3d: %lf, %lf\n",i,hInit, lInit);
		for(j=0;j<article_wordcount-1;j++){
		
			if(inputs[j]!=-1){
			
				hDistance = hInit - inputs[j];
				if(hDistance<0)
					hDistance*=-1;

				lDistance = inputs[j]-lInit;
				if(lDistance<0)
					lDistance*=-1;
				
				//printf("distance %3d: %lf, %lf\n",j,hDistance, lDistance);

				if(hDistance > lDistance){
					lGroupSum += inputs[j];
					lGroupCount++;	
				}
				else{
					hGroupSum += inputs[j];
					hGroupCount++;

					if(inputs[j]<minOfHGroup)
						minOfHGroup=inputs[j];
				}
			}
	
		}
	//		printf("lGroupSum %3d: %lf, %d\n",i,lGroupSum, lGroupCount);
		//	printf("hGroupSum %3d: %lf, %d\n",i,hGroupSum, hGroupCount);
		
		if(hGroupCount>0)
			hInit = hGroupSum/hGroupCount;
		if(lGroupCount>0)
			lInit = lGroupSum/lGroupCount;
	
	}

	return minOfHGroup;
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
		if(keywordlistFlag->count>1){
		if(keywordlistFlag->next!=NULL)
			wprintf(L"%ls,", keywordlistFlag->keyword, keywordlistFlag->count);
			//wprintf(L"%ls(%d),", keywordlistFlag->keyword, keywordlistFlag->count);
		else	
			wprintf(L"%ls", keywordlistFlag->keyword, keywordlistFlag->count);
		//	wprintf(L"%ls(%d)\n", keywordlistFlag->keyword, keywordlistFlag->count);
		}
		keywordlistFlag=keywordlistFlag->next;
	}
	printf("\n");
}


