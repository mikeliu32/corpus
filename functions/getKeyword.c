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

#define HASHTABLE_SLOTSIZE 500000

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
void article_ratioCount(node ** article_nodelist, long slotsize, long term_count);

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
	long article_size;
	wordnode * article;
	node ** articleHashTable;
	node * articleTempNode;
	int article_wordcount;
	int i;
	long term_totalCount=0;


//implementing SOCKET

	int sockfd;
	struct sockaddr_in dest;
	char read_buffer[2000];
	char write_buffer[2000];
	char read_header[500];
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	
	bzero(&dest, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(9210);
	dest.sin_addr.s_addr = INADDR_ANY;

	bind(sockfd, (struct sockaddr*)&dest, sizeof(dest));

	listen(sockfd, 20);
	printf("listen to :9210\n");

	while(1){
		int clientfd;
		struct sockaddr_in client_addr;
		int addrlen = sizeof(client_addr);

		clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);

		int res = recv(clientfd, read_header,sizeof(read_header), 0);
	printf("%s",read_header);

	//printf("-%d-",write(clientfd, "100", 3));
	write(clientfd, "100", 3);
	res = recv(clientfd, read_buffer,sizeof(read_buffer), 0);
	
	printf("%s",read_buffer);
	articleHashTable = create_hashtable(articleHashTable, 2000);
	
	//fp = fopen(argv[argc-1],"rb");

	//fseek(fp, 0L, SEEK_END);
	
	//article_size = ftell(fp);
	//fseek(fp, 0L, SEEK_SET);
	
	article_size = strlen(read_buffer);

	article = (wordnode*)malloc((article_size/sizeof(wchar_t))*sizeof(wordnode));
	article_wordcount=0;

	pre_wchar=0;

	//while((wchar=fgetwc(fp))!=WEOF){
	for(i=0;i<article_size;i++){
		//article[article_wordcount].word=wchar;
		article[i].word = read_buffer[i];
		wchar = read_buffer[i];
		if(isStopword(stopwordList, wchar)==1 || wchar<=127){
			article[article_wordcount].isTerm = 0;
			article[article_wordcount].breakmark =1;

			if(article_wordcount!=0){
				article[article_wordcount-1].breakmark=1;
				article[article_wordcount-1].nextRatio=-1;
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

				if((tempNode=find_node(articleHashTable, 2000, term))!=NULL)
					
					tempNode->count++;
				else{
					insert_node(articleHashTable, 2000, term,  1, 0);
				}
				term_totalCount++;
			pre_wchar=wchar;
			}
			article[article_wordcount].isTerm=1;
			article[article_wordcount].breakmark=1;
		}

		article[article_wordcount].nextCount = -1;
		article[article_wordcount].nextRatio = -1;
		article_wordcount++;
	}
	article[article_wordcount-1].breakmark=1;

	//fclose(fp);

	//printf("----%ld----\n", term_totalCount);
	article_ratioCount(articleHashTable, 2000, term_totalCount);

	pre_wchar=0;
	double ratio_total=0;
	int ratio_count=0;
	float ratio_max=-1000;
	wordnode* articleword;
	wordnode* articleword_next;
	int j;


	for(i=0;i<article_wordcount-1;i++){

		if(article[i].isTerm!=0 && article[i+1].isTerm!=0){
		
		articleword = &article[i];

		articleword_next=NULL;

		for(j=i+1;j<article_wordcount;j++){
			if(article[j].isTerm!=0){
				articleword_next = &article[j];
				break;
			}
		}

		if(articleword_next!=NULL){
			term[0]=articleword->word;
			term[1]=articleword_next->word;

			articleTempNode = find_node(articleHashTable, 2000, term);

			if((tempNode=find_node(hashtable, HASHTABLE_SLOTSIZE, term))!=NULL){
				articleword->nextCount = articleTempNode->count;
				articleword->nextRatio = tempNode->ratio * articleTempNode->ratio;
			}
			else{
				articleword->nextCount = articleTempNode->count;
				articleword->nextRatio = 0.0;
			}

			ratio_total+=articleword->nextRatio;
			ratio_count+=articleword->nextCount;
			
			if(articleword->nextRatio>ratio_max)
				ratio_max = articleword->nextRatio;

		}

		}

	}
/*
	for(i=0;i<article_wordcount;i++){

		if(article[i].isTerm!=0)
		wprintf(L"%lc-%f-",article[i].word,article[i].nextRatio);
		
		else
		wprintf(L"%lc",article[i].word);

	}
*/
	keywordnode * keywordlist=NULL;
	keywordnode * tempKeywordNode;
	keywordnode * keywordlistFlag;
	int keyword_start=-1, keyword_end=-1;
	int k;
	wchar_t * tempKeyword;
	//double filterRate= (ratio_total/(double)ratio_count)*log10((double)ratio_count);
//	double filterRate = ((ratio_max-(ratio_total/(double)ratio_count)))*2*0.2;
	double filterRate = ((ratio_total/(double)ratio_count))*4;
	for(i=0;i<article_wordcount;i++){

			if(article[i].nextRatio>=filterRate)
			{
				if(keyword_start==-1)
					keyword_start=i;

				keyword_end=i+1;
			}
			else if(keyword_start!=-1){
				tempKeyword = (wchar_t*) malloc(sizeof(wchar_t)*(keyword_end+1-keyword_start+1));
					
				for(j=keyword_start,k=0;j<keyword_end+1;j++,k++)
				{
					tempKeyword[k] = article[j].word; 
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

	keywordlistFlag=keywordlist;
	while(keywordlistFlag!=NULL)
	{
		wprintf(L"\n%d\t%ls", keywordlistFlag->count, keywordlistFlag->keyword);
		keywordlistFlag=keywordlistFlag->next;
	}

	
	printf("\ntotal term:%d\nratio ave:%lf\nlog count:%lf\nratio max:%lf\nfilter rate:%lf\n",ratio_count,ratio_total/(double)ratio_count,log10((double)ratio_count),ratio_max, filterRate);
	
	}
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
	
	nodelist = (node**)malloc(sizeof(node*)*slotsize);
	memset(nodelist, 0, sizeof(node*)*slotsize);
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
