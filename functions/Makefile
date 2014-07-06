#all
getTermset:
	gcc -lm generateTermSet.c stopwordFunction.c -o generateTermSet
all:
	gcc -lm getCount.c stopwordFunction.c
socket:
	gcc -lm getKeyword.c stopwordFunction.c
tag:
	clang -lm -g tagKeyword.c stopwordFunction.c -o tagKeyword
co:
	clang -lm -g getCocurrence.c stopwordFunction.c -o getCocurrence
re:
	clang -lm -g countRelatedCorpus.c -o countRelatedCorpus
hashget_re:
	gcc hashget_relatedCorpus.c -o hashget_relatedCorpus
