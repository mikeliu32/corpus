
typedef struct n_stopword{
	wchar_t	stopword;
	struct n_stopword * next;
}stopwordNode;


stopwordNode* loadStopwordFromFile(FILE *stopwordFP);

int isStopword(stopwordNode* stopwordList, wchar_t target_wchar);
