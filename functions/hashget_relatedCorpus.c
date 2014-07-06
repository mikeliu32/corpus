#include<stdlib.h>
#include<stdio.h>
#include<locale.h>
#include<wchar.h>
#include<string.h>

#define HASHTABLE_SLOTSIZE 500000


typedef struct nd{
	wchar_t* key;
	long index;
	long size;
	struct nd *next;
}node;


unsigned int hash33(wchar_t* key, long htsize);

node** create_hashtable(node** nodelist, long slotsize);
node* insert_node(node** nodelist, wchar_t* keyToAdd, long keyIndex, long size);
node* find_node(node** nodelist, wchar_t* keyToFind);

int main(int argc, char* argv[]){

	FILE *rfp, *rfp_index;
	setlocale(LC_ALL, "zh_TW.UTF-8");

	node ** hashtable;
	node * tempNode;
	wchar_t inputTerm[50], key[50];
	char * output;
	long recordPos, recordSize;
	char * p;

	hashtable = create_hashtable(hashtable, HASHTABLE_SLOTSIZE);

	rfp_index = fopen("relatedCorpus.index","r");
	rfp = fopen("relatedCorpus","r");

	while(fwscanf(rfp_index, L"%ls %ld %ld",&key,&recordPos,&recordSize)!=EOF){
		insert_node(hashtable, key, recordPos, recordSize);
	}
	fclose(rfp_index);
	
	
	while(wscanf(L"%ls",&inputTerm)!=EOF){
	
		if((tempNode=find_node(hashtable, inputTerm))!=NULL){
			
			recordPos=tempNode->index;
			recordSize=tempNode->size;

			fseek(rfp, recordPos, SEEK_SET );
			
			output = (char*)malloc(recordSize);
			fread(output,1,recordSize,rfp);
			printf("=========================================\n");
			wprintf(L"record of %ls:\n", inputTerm);
			printf("-----------------------------------------\n");
			fwrite(output , sizeof(char), recordSize, stdout);
			printf("=========================================\n");

			free(output);
		}
		else{
			printf("cannot find!\n");
		}
	}

	
	fclose(rfp);

	return 0;

}

unsigned int hash33(wchar_t* key, long htsize){

	int i;
	unsigned int hashValue=0;

	for(i=0;i<2;i++){
		hashValue=(hashValue<<5)+hashValue+(unsigned int)key[i];
	}

	return hashValue%htsize;
}

node** create_hashtable(node** nodelist, long slotsize){
	
	nodelist = (node**)malloc(sizeof(node*)*slotsize);
	memset(nodelist, 0, sizeof(node*)*slotsize);
	return nodelist;

}


node* insert_node(node** nodelist, wchar_t* keyToAdd, long keyIndex, long size){

	unsigned int hashkey = hash33(keyToAdd, HASHTABLE_SLOTSIZE);

	node *newNode=(node*)malloc(sizeof(node));
	int keyToAddLen = wcslen(keyToAdd);
	
	if(nodelist[hashkey]==NULL){
		newNode->key = (wchar_t*)malloc(sizeof(wchar_t)*(keyToAddLen+1));
		wcscpy(newNode->key, keyToAdd);
		newNode->key[keyToAddLen]='\0';
	
		newNode->index = keyIndex;
		newNode->size = size;
		newNode->next = NULL;

		nodelist[hashkey] = (node*)malloc(sizeof(node));
		nodelist[hashkey]->next = newNode;
	}
	else{
		newNode->key = (wchar_t*)malloc(sizeof(wchar_t)*(keyToAddLen+1));
		wcscpy(newNode->key, keyToAdd);
		newNode->key[keyToAddLen]='\0';
	
		newNode->index = keyIndex;
		newNode->size = size;
		newNode->next = NULL;
		
		newNode->next = nodelist[hashkey]->next;
		nodelist[hashkey]->next = newNode;
	}

	return newNode;
}

node* find_node(node** nodelist, wchar_t* keyToFind){

	unsigned int hashkey = hash33(keyToFind, HASHTABLE_SLOTSIZE);
	node *nodeSlot = nodelist[hashkey];

	if(nodeSlot!=NULL){
		
		nodeSlot = nodeSlot->next;

		while(nodeSlot!=NULL){
			
			if(wcscmp(nodeSlot->key,keyToFind)==0)
			{
				return nodeSlot;
			}
			else
				nodeSlot= nodeSlot->next;

		}

	}
	return NULL;
}




