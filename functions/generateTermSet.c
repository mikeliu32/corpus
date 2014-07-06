#include<stdlib.h>
#include<stdio.h>
#include<locale.h>
#include<wchar.h>
#include<string.h>
#include<math.h>

#include "news.h"

#define HASHTABLE_SLOTSIZE 5000000

typedef struct nd{
	wint_t key[2];
	long count;
	struct nd *next;
}node;


unsigned int hash33(wint_t* key, long htsize);

node** create_hashtable(node** nodelist, long slotsize);
node* insert_node(node** nodelist, wint_t* keyToAdd);
node* find_node(node** nodelist, wint_t* keyToFind);

void hashtable_generateResult(node** nodelist, long slotsize, long term_count);

int main(int argc, char* argv[]){

	FILE *fp;
	setlocale(LC_ALL, "zh_TW.UTF-8");

	wint_t wchar, pre_wchar=0;

	node ** hashtable;
	node * tempNode;
	wint_t newTerm[2];

	long term_totalcount=0;

	hashtable = create_hashtable(hashtable, HASHTABLE_SLOTSIZE);

//load stopwords
	fp = fopen(argv[argc-1],"r");
	stopwordNode * stopwordList = loadStopwordFromFile(fp);
	fclose(fp);
	
	fp = fopen(argv[argc-2],"r"); 

	while((wchar=fgetwc(fp))!=WEOF){
//		wprintf(L"%lc-%ld\n",wchar,wchar);		

		if(isStopword(stopwordList, wchar)==1 || wchar<=127)
		{
			pre_wchar=0;
		}
		else
		{

			if(pre_wchar==0)
				pre_wchar = wchar;
			else{
			//wprintf(L"%lc%lc\n",pre_wchar,wchar);

				newTerm[0]=pre_wchar;
				newTerm[1]= wchar;

				if((tempNode=find_node(hashtable, newTerm))!=NULL){
					tempNode->count++;
					term_totalcount++;
				}
				else{
					insert_node(hashtable, newTerm);
					term_totalcount++;
				}
				
			pre_wchar=wchar;
			}
		}

	}
	
	fclose(fp);


	hashtable_generateResult(hashtable, HASHTABLE_SLOTSIZE, term_totalcount);
	return 0;

}

unsigned int hash33(wint_t* key, long htsize){

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


node* insert_node(node** nodelist, wint_t* keyToAdd){

	unsigned int hashkey = hash33(keyToAdd, HASHTABLE_SLOTSIZE);

	node *newNode=(node*)malloc(sizeof(node));

	if(nodelist[hashkey]==NULL){
		nodelist[hashkey] = (node*)malloc(sizeof(node));
		newNode->key[0] = keyToAdd[0];
		newNode->key[1] = keyToAdd[1];
		newNode->count = 1;
		newNode->next = NULL;
		nodelist[hashkey]->next = newNode;
	}
	else{
		newNode->key[0] = keyToAdd[0];
		newNode->key[1] = keyToAdd[1];
		newNode->count = 1;
		newNode->next = nodelist[hashkey]->next;
		nodelist[hashkey]->next = newNode;
	}
	//wprintf(L"%lc %lc\n", nodeSlot->next->key[0], nodeSlot->next->key[1]);
	return newNode;
}

node* find_node(node** nodelist, wint_t* keyToFind){

	unsigned int hashkey = hash33(keyToFind, HASHTABLE_SLOTSIZE);
	node *nodeSlot = nodelist[hashkey];

	if(nodeSlot!=NULL){
		
		nodeSlot = nodeSlot->next;

		while(nodeSlot!=NULL){
			
			if(nodeSlot->key[0]==keyToFind[0] && nodeSlot->key[1]==keyToFind[1])
			{
				return nodeSlot;
			}
			else
				nodeSlot= nodeSlot->next;

		}

	}
	return NULL;
}

void hashtable_generateResult(node ** nodelist, long slotsize, long term_count){

	int row;
	node * temp;

	for(row=0;row<slotsize;row++){

		if(nodelist[row]!=NULL){
			temp = nodelist[row]->next;	

			while(temp!=NULL){
	
				wprintf(L"%lc%lc\t%ld\t%lf\n",temp->key[0],temp->key[1],temp->count,log((double)term_count/(double)temp->count));
				temp = temp->next;
			}
		}
	}


}
