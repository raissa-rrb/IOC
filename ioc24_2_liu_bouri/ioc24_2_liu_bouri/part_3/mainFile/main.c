#include <stdio.h>
#include <fcntl.h>


int main(){

	char *bp ="0";
	int fled_bp = open("/dev/led_s", O_RDWR);
	if(fled_bp < 0){
		fprintf(stderr, "erreur");
		return 1;
	}
	
	while(1){
		read(fled_bp, &bp,1);
		printf("bp : %s \n", bp);
		(bp == "1") ? write(fled_bp,&"1",1) : write(fled_bp,&"0",1);
		sleep(1);
	}
	return 0;

}