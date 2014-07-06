#include<stdlib.h>
#include<stdio.h>
#include<locale.h>
#include<wchar.h>
#include<string.h>

#define HASHTABLE_SLOTSIZE 500000

typedef struct relatedNd{
	wchar_t* term;
	long count;
	double weight;
	struct relatedNd *next;
}relatedNode;

typedef struct nd{
	wchar_t* key;
	long count;
	long rTermCount;
	long rTermOCount;
	relatedNode* relatedList;
	struct nd *next;
}node;


unsigned int hash33(wchar_t* key, long htsize);

node** create_hashtable(node** nodelist, long slotsize);
node* insert_node(node** nodelist, wchar_t* keyToAdd);
node* find_node(node** nodelist, wchar_t* keyToFind);

void addToRelatedList(node* keynode, wchar_t* relateTerm);
void hashtable_countWeightAndSort(node** nodelist, long slotsize);
void hashtable_generateResult(node** nodelist, long slotsize, FILE* wfp_result, FILE* wfp_index);
int compare (const void * a, const void * b);


int main(int argc, char* argv[]){

	FILE *fp, *wfp_result, *wfp_index;
	setlocale(LC_ALL, "zh_TW.UTF-8");

	node ** hashtable;
	node * tempNode;
	wchar_t keyTerm[50],relateTerm[50];

	hashtable = create_hashtable(hashtable, HASHTABLE_SLOTSIZE);

	fp = fopen(argv[argc-1],"r");
	wfp_result = fopen("relatedCorpus","w");
	wfp_index = fopen("relatedCorpus.index","w");

	while(fwscanf(fp, L"%ls %ls",&keyTerm,&relateTerm)!=EOF){
	
		//wprintf(L"%ls\t%ls\n",keyTerm, relateTerm);
		if((tempNode=find_node(hashtable, keyTerm))!=NULL){
			tempNode->count++;
			
			addToRelatedList(tempNode, relateTerm);

		}
		else{
			tempNode = insert_node(hashtable, keyTerm);
			addToRelatedList(tempNode, relateTerm);

		}
		
		if((tempNode=find_node(hashtable, relateTerm))!=NULL){
			tempNode->count++;
			
			addToRelatedList(tempNode, keyTerm);

		}
		else{
			tempNode = insert_node(hashtable, relateTerm);
			addToRelatedList(tempNode, keyTerm);

		}
	}

	
	fclose(fp);

	hashtable_countWeightAndSort(hashtable, HASHTABLE_SLOTSIZE);
	hashtable_generateResult(hashtable, HASHTABLE_SLOTSIZE, wfp_result, wfp_index);
	fclose(wfp_result);
	fclose(wfp_index);
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


node* insert_node(node** nodelist, wchar_t* keyToAdd){

	unsigned int hashkey = hash33(keyToAdd, HASHTABLE_SLOTSIZE);

	node *newNode=(node*)malloc(sizeof(node));
	int keyToAddLen = wcslen(keyToAdd);
	
	if(nodelist[hashkey]==NULL){
		newNode->key = (wchar_t*)malloc(sizeof(wchar_t)*(keyToAddLen+1));
		wcscpy(newNode->key, keyToAdd);
		newNode->key[keyToAddLen]='\0';
	
		newNode->count = 1;
		newNode->rTermCount=0;
		newNode->rTermOCount=0;
		newNode->relatedList = NULL;
		newNode->next = NULL;

		nodelist[hashkey] = (node*)malloc(sizeof(node));
		nodelist[hashkey]->next = newNode;
	}
	else{
		newNode->key = (wchar_t*)malloc(sizeof(wchar_t)*(keyToAddLen+1));
		wcscpy(newNode->key, keyToAdd);
		newNode->key[keyToAddLen]='\0';
	
		newNode->count = 1;
		newNode->rTermCount=0;
		newNode->rTermOCount=0;
		newNode->relatedList = NULL;
		newNode->next = NULL;
		
		newNode->next = nodelist[hashkey]->next;
		nodelist[hashkey]->next = newNode;
	}
	//wprintf(L"%lc %lc\n", nodeSlot->next->key[0], nodeSlot->next->key[1]);
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

void addToRelatedList(node* keynode, wchar_t* relateTerm){

	int relateTermLen = wcslen(relateTerm);
	int isExist = 0;
	relatedNode* relatedList_flag = keynode->relatedList;
	
	while(relatedList_flag!=NULL){

		if(wcscmp(relatedList_flag->term,relateTerm)==0)
		{
			isExist = 1;
			break;
		}

		relatedList_flag = relatedList_flag->next;
	}
	
	if(isExist==1){
		relatedList_flag->count++;
	}
	
	else if(isExist==0){
		relatedNode* newRelateNode = (relatedNode*)malloc(sizeof(relatedNode));
		
		newRelateNode->term = (wchar_t*)malloc(sizeof(wchar_t)*(relateTermLen+1));
		wcscpy(newRelateNode->term, relateTerm);
		newRelateNode->term[relateTermLen]='\0';
		
		newRelateNode->count = 1;

		if(keynode->relatedList==NULL){
			newRelateNode->next =NULL;
			keynode->relatedList = newRelateNode;
		}
		else{
			newRelateNode->next = keynode->relatedList;
			keynode->relatedList = newRelateNode;
		}
		keynode->rTermCount++;
	
	}


}

void hashtable_countWeightAndSort(node** nodelist, long slotsize){

	int row,i;
	node * temp, *rtemp;
	relatedNode * rNode;
	relatedNode ** relatedListAry;
	relatedNode * rlist_head, *rlist_flag;
	long key_count, rTerm_count, rTerm_cooccur;
	double weight;
	
	for(row=0;row<slotsize;row++){

		if(nodelist[row]!=NULL){
			temp = nodelist[row]->next;	

			while(temp!=NULL){
				
				key_count = temp->count;
				
				relatedListAry = (relatedNode**)malloc(sizeof(relatedNode*)*temp->rTermCount);
				
				//count related list weight
				rNode=temp->relatedList;
				i=0;
				while(rNode!=NULL){
					
					rtemp = find_node(nodelist, rNode->term);
					
					rTerm_count = rtemp->count;
					rTerm_cooccur = rNode->count;
					
					//weight = ((double)rTerm_cooccur/(double)key_count)*2+((double)rTerm_cooccur/(double)rTerm_count);			
					weight = ((double)rTerm_cooccur/(double)key_count)+((double)rTerm_count/(double)key_count)*rTerm_cooccur;					
					
					rNode->weight = weight;

					relatedListAry[i]=rNode;
					i++;
					rNode = rNode->next;
				}
				
				
				//sort the related list array;
//				 qsort (relatedListAry, temp->rTermCount, sizeof(relatedNode), compare);


	int j;
	relatedNode* swaptemp;
	for(i=0;i<temp->rTermCount-1;i++){
		for(j=0;j<temp->rTermCount-i-1;j++){
			if(relatedListAry[j]->weight<relatedListAry[j+1]->weight){
				swaptemp = relatedListAry[j];
				relatedListAry[j]=relatedListAry[j+1];
				relatedListAry[j+1]=swaptemp;
			}
		}

	}

				//covert related list array back to linked list
				rlist_head=NULL;
				for(i=0;i<temp->rTermCount;i++){
					//wprintf(L"%ls\t%f\n",relatedListAry[i]->term,relatedListAry[i]->weight);

					if(i==0){
						rlist_head = relatedListAry[i];
						rlist_flag = rlist_head;
						continue;
					}

					rlist_flag->next = relatedListAry[i];
					rlist_flag = rlist_flag->next;

				
				}
				rlist_flag->next=NULL;
				
				temp->relatedList = rlist_head;
				//free(relatedListAry);
				temp = temp->next;
			}
		}
	}


}

int compare (const void * a, const void * b)
{
  return ( ((relatedNode*)a)->weight - ((relatedNode*)b)->weight );
}

void hashtable_generateResult(node** nodelist, long slotsize, FILE* wfp_result, FILE* wfp_index){

	int row;
	node * temp;
	relatedNode * rNode;
	long record_start,recordSize;
	
	for(row=0;row<slotsize;row++){

		if(nodelist[row]!=NULL){
			temp = nodelist[row]->next;	

			while(temp!=NULL){

			record_start=ftell(wfp_result);
				

				fprintf(wfp_result,"@\n");
				fwprintf(wfp_result,L"@key:%ls\n@kcount:%ld\n@rOcount:%ld\n",temp->key,temp->count,temp->rTermOCount);
				
				fprintf(wfp_result,"@relate:\n");
				rNode=temp->relatedList;

				
				while(rNode!=NULL){
					
					fwprintf(wfp_result,L"%ls\t%f\n",rNode->term,rNode->weight);

					//fwprintf(wfp_result,L"%ls\t%ld\t%ld\t%ld\n",rNode->term,key_count, rTerm_count, rTerm_cooccur);
					rNode = rNode->next;
				}

				//output index
				recordSize= ftell(wfp_result)-record_start;
				fwprintf(wfp_index, L"%ls\t%ld\t%ld\n",temp->key, record_start, recordSize);
				
				temp = temp->next;
			}
		}
	}


}


