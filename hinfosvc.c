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
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define STRLEN 100

/**
 * @brief Ze souboru /proc/cpuinfo precte jmeno CPU.
 *
 * @param name String, do ktereho ulozi jmeno CPU.
*/
int getcpuname(char *name) {
  FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
  if(cpuinfo == NULL)
    return 1;
  char *TARGET = "model name";

  while(1) {
    int i = 0;
    char k;

    // v textu hledam TARGET
    while(fgetc(cpuinfo) == TARGET[i])
      i++;

    // TARGET se v textu nachazi
    if((size_t) i == strlen(TARGET)) {
      do{k = fgetc(cpuinfo);}while(k != ':'); 
      i = 0;
      // za ':' je ' ' a nasleduje jmeno cpu a konec radku
      char c = fgetc(cpuinfo);
      while((c = fgetc(cpuinfo)) != '\n')
        name[i++] = c;
      name[i] = '\0';
      break;
    }
    // TARGET v textu neni -> doctu do konce radku
    else {
      while(fgetc(cpuinfo) != '\n');
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
