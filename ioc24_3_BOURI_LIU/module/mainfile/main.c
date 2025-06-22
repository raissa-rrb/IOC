#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include "../lcd_lb.h"


int main(){
	int flcd_lb = open("/dev/lcd_lb", O_RDWR);
	struct cord_xy* cord;
	if(flcd_lb < 0){
		fprintf(stderr, "erreur");
		return 1;
	}

	char* message_1 = "Bonjour, bienvenue";
	int i;

	while(1){
		// write(flcd_lb,"Bonjour, bienvenue\n",256);
		// sleep(3);

		// cord->line = 1;
		// cord->row = 0;
		// if(ioctl(flcd_lb,LCDIOCT_SETXY, cord) < 0){
		// 	fprintf(stderr, "erreur set xy");
		// 	return 1;
		// }
		// write(flcd_lb,"Voici la salle SESI\n",256);
		// sleep(5);

		// cord->line = 2;
		// cord->row = 5;
		// if(ioctl(flcd_lb,LCDIOCT_SETXY, cord) < 0){
		// 	fprintf(stderr, "erreur set xy");
		// 	return 1;
		// }
		// write(flcd_lb,"Appuyer sur le bouton ?\n",256);
		// sleep(5);
		// if(ioctl(flcd_lb,LCDIOCT_CLEAR) < 0){
		// 	fprintf(stderr, "erreur");
		// 	return 1;
		// }
		for(i = 0; i < 20; i++){
			//ioctl(flcd_lb,LCDIOCT_CLEAR);
			write(flcd_lb,&message_1[i],4);
			usleep(1000000);
		}
		// write(flcd_lb,"Bonjour, bienvenue\n",256);
		//sleep(5);

		// cord->line = 1;
		// cord->row = 0;
		// ioctl(flcd_lb,LCDIOCT_SETXY, cord);
		// write(flcd_lb,"Voici la salle SESI\n",256);
		// sleep(5);

		// cord->line = 2;
		// cord->row = 0;
		// ioctl(flcd_lb,LCDIOCT_SETXY, cord);
		// write(flcd_lb,"Appuie sur le bouton\n",256);
		// sleep(5);

		if(ioctl(flcd_lb,LCDIOCT_CLEAR) < 0){
			fprintf(stderr, "erreur");
			return 1;
		}

		sleep(1);


	}
	return 0;

}