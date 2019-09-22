#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <cjson/cJSON.h>
#include <signal.h>

#include "OLED_Driver.h"
#include "DEV_Config.h"

#define WHITE 0xF
#define BLACK 0x0

void help() {
  printf("expono [JSON FILE] (-e MICROSECONDS)\n");
}

void Handler(int signo) {
  //System Exit
  printf("\r\nHandler:exit\r\n");
  DEV_ModuleExit();

  exit(0);
}

void point(POINT Xpoint, POINT Ypoint, COLOR Color) {
    if(Xpoint > 127 || Ypoint > 127) {
		    printf("Can't %ix%i\n", Xpoint, Ypoint);
        return;
    }

    uint16_t XDir_Num ,YDir_Num;
    for(XDir_Num = 0; XDir_Num < 2 - 1; XDir_Num++) {
        for(YDir_Num = 0; YDir_Num < 2 - 1; YDir_Num++) {
            OLED_SetColor(Xpoint + XDir_Num, Ypoint + YDir_Num, Color);
        }
    }
}

void plot(int fd) {
  int len = lseek(fd, 0, SEEK_END);
	char *data = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);

  if((int)data == -1) {
    printf("Couldn't mmap file\n");
    exit(1);
  }

  cJSON *elem;
	cJSON *name;

	cJSON *root = cJSON_Parse(data);

  int n = cJSON_GetArraySize(root);

  for(int i=0; i<n; i++) {
		elem = cJSON_GetArrayItem(root, i);
		int sn = cJSON_GetArraySize(elem);

		for(int j=0; j<sn; j++) {
			int v = cJSON_GetArrayItem(elem, j)->valueint;
      if(v > 0) point(i, j, 0x1); // (uint8_t)v
		}
	}

  munmap(data, len);
}

int main(int argc, char **argv) {
  int exposure=0;

  if(argc<2 || argv[1][0] == '-') {
    help();
    printf("No file given\n");
    exit(1);
  }

  for(int argi=2; argi<argc; argi++) {
    if(argv[argi][0] == '-')
      switch(argv[argi][1]) {
        case 'e': {
          exposure = atoi(argv[argi+1]);
          break;
        }
      }
  }

  int fd = open(argv[1], O_RDONLY);

  if(fd == -1) {
    help();
    printf("%s file not found\n", argv[1]);
    exit(1);
  }
  printf("Exposure: %i microseconds\n", exposure);

  if(DEV_ModuleInit()) exit(0);

  OLED_SCAN_DIR OLED_ScanDir = SCAN_DIR_DFT;//SCAN_DIR_DFT = D2U_L2R
  OLED_Init(OLED_ScanDir);

  OLED_Clear(BLACK);
  plot(fd);
  OLED_Display();
  usleep(exposure);
  OLED_Clear(BLACK);
  OLED_Display();

  DEV_ModuleExit();
  return 0;
}
