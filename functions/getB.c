#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#define bytesPerMB 1048576

int main(int argc, char* argv[])
{

	FILE *rfp, *wfp;
	char *buf, *sflag, *eflag, *bufflag;
	char *bflag,*tempflag;
	long content_size;
	long buffer_size = 512*bytesPerMB;
	
	size_t frsize=1;
	fpos_t fppos;


	rfp = fopen(argv[argc-1],"rb");
	
	if(rfp==NULL)
		exit(1);

	buf = (char*)malloc(buffer_size);

	do{
		fgetpos(rfp, &fppos);
		frsize = fread(buf, 1, buffer_size, rfp);
		bufflag = buf;
		
		while((sflag=strstr(bufflag,"@GAISRec:"))!=NULL && sflag-buf<frsize){

			eflag=strstr(sflag+2, "@GAISRec:");

			if(eflag==NULL){
				if(!feof(rfp)){
					fsetpos(rfp,&fppos);
					fseek(rfp, sflag-buf, SEEK_CUR);
					break;
				}
				else{
					if((bflag=strstr(sflag,"\n@B:"))!=NULL){
						content_size = buf+frsize-bflag-4;
						fwrite(bflag+4, 1, content_size,stdout);
					}
					break;
				}

			}
			else{
				if((bflag=strstr(sflag,"\n@B:"))!=NULL){
					content_size = eflag-bflag-4;
					fwrite(bflag+4, 1, content_size,stdout);
				}
	
			}
			bufflag = eflag;

		}

	}while(!feof(rfp));


	return 0;
}
