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
 * @brief Vypocita informace o behu CPU z /proc/stat.
 * Kod je prevzaty a lehce upraveny od Alexander Heinlein
 * z: https://github.com/scaidermern/top-processes
 *
 * @param idled Pamet, kam se ulozi data pro dalsi vypocet.
 * @return Data o vyuzit cpu v dany moment.
*/
unsigned long long gettotalcputime(unsigned long long *idled) {
  FILE* file = fopen("/proc/stat", "r");
  if (file == NULL) {
    perror("Could not open stat file");
    return 0;
  }

  char buffer[1024];
  unsigned long long user = 0, nice = 0, system = 0, idle = 0;
  // added between Linux 2.5.41 and 2.6.33, see man proc(5)
  unsigned long long iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guestnice = 0;

  char* ret = fgets(buffer, sizeof(buffer) - 1, file);
  if (ret == NULL) {
    perror("Could not read stat file");
    fclose(file);
    return 0;
  }
  fclose(file);

  sscanf(buffer,
             "cpu  %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu",
             &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guestnice);

  *idled = idle + iowait;

  // sum everything up (except guest and guestnice since they are already included
  // in user and nice, see http://unix.stackexchange.com/q/178045/20626)
  return user + nice + system + idle + iowait + irq + softirq + steal;
}

/**
 * @brief Vypocte vytizeni CPU v rozmezi 1s.
 *
 * Vypocet je podle vzorecku z https://stackoverflow.com/a/23376195
 * @return Vytizeni v tvaru desetinneho cisla.
 */
double getcpuload() {
  unsigned long long previdle = 0;
  unsigned long long prevtotal = gettotalcputime(&previdle);
  sleep(1);
  unsigned long long idle = 0;
  unsigned long long total = gettotalcputime(&idle);

  double totald = total - prevtotal;
  double idled = idle - previdle;
    
  return (totald - idled)/totald;
}


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
  printf("%s\n", hostname);
  printf("%s\n", cpu);

  double load = getcpuload();
  printf("%.0f%%\n", load*100);

  return 0;
}
