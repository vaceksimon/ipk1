/**
 * Projekt: IPK Projekt1
 *
 * @brief Implementace 1. projektu z predmetu IPK.
 *
 * @author Šimon Vacek xvacek10@stud.fit.vutbr.cz
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define STRLEN 100

int getcpuname(char *name) {
  FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
  char *TARGET = "model name";

  while(1) {
    int i = 0;
    char k;
    while(fgetc(cpuinfo) == TARGET[i])
      i++;
    if(i == sizeof(TARGET) + 2) {
      do{k = fgetc(cpuinfo);}while(k != ':'); 
      i = 0;
      char c;
      while((c = fgetc(cpuinfo)) != '\n')
        name[i++] = c;
      name[i] = '\0';
      break;
    }
    else {
      do {} while(fgetc(cpuinfo) != '\n');
    }
    
  }
  fclose(cpuinfo);
  return 0;
}

int main() {
  char hostname[STRLEN];
  char cpu[STRLEN];
  gethostname(hostname, STRLEN);
  getcpuname(cpu);
  printf("%s\n", cpu);
  return 0;
}
