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

#include <sys/socket.h>
#include <netinet/in.h>

#define ERROR_OK 0
#define ERROR 1

/**
 * @brief Vypocita informace o behu CPU z /proc/stat.
 * Kod je prevzaty a lehce upraveny od Alexander Heinlein
 * z: https://github.com/scaidermern/top-processes
 *
 * @param idled Pamet, kam se ulozi data pro dalsi vypocet.
 * @return Data o vyuziti cpu v dany moment.
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
 *
 * @param load Buffer, kam se ulozi zatez CPU.
 * @return Vytizeni v tvaru desetinneho cisla.
 */
int getcpuload(char *load) {
  unsigned long long previdle = 0;
  unsigned long long prevtotal = gettotalcputime(&previdle);
	if(!prevtotal)
		return ERROR;
  sleep(1);
  unsigned long long idle = 0;
  unsigned long long total = gettotalcputime(&idle);
	if(!total)
		return ERROR;

  double totald = total - prevtotal;
  double idled = idle - previdle;
    
  sprintf(load, "%0.f%%", (totald - idled)/totald * 100);
	return ERROR_OK;
}


/**
 * @brief Ze souboru /proc/cpuinfo precte jmeno CPU.
 *
 * @param name String, do ktereho ulozi jmeno CPU.
 * @return 0 pri uspechu
*/
int getcpuname(char *name) {
  FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
  if(cpuinfo == NULL)
    return ERROR;
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
  return ERROR_OK;
}


/**
 * @brief Vygeneruje a odesle HTTP response, na zaklade hodnoty type.
 *
 * @param sockfd File descriptor socketu klienta
 * @param type Udava, ktera odpoved se ma vygenerovat.
 *             0 = hostname
 *             1 = cpu-name
 *             2 = load
 *            -1 = bad request
*/
void sendresponse(int sockfd, int type) {
	char resp[512];
	// predchystam hlavicku response
	strcpy(resp, "HTTP/1.1 200 OK\r\nContent-Type: text/plain;\r\n\r\n");
	char data[100];
	switch(type) {
		case 0:
			gethostname(data, sizeof(data));
		break;

		case 1:
			if(getcpuname(data)) {
				strcpy(resp, "HTTP/1.1 500 Internal Server Error\r\n\r\n");
				write(sockfd, resp, strlen(resp));
				return;
			}
		break;

		case 2:
			if(getcpuload(data)) {
				strcpy(resp, "HTTP/1.1 500 Internal Server Error\r\n\r\n");
				write(sockfd, resp, strlen(resp));
				return;
			}
		break;

		case -1:
			// neznamy pozadavek
			strcpy(data, "HTTP/1.1 400 Bad Request\r\n\r\n");
			write(sockfd, data, strlen(data));
			return;
		break;

		default:
		break;
	}
	// k response prilepim telo a zapisu do socketu
	strcat(resp, data);
	write(sockfd, resp, strlen(resp));
}

/**
 * @brief Na zaklade HTTP requestu odesle prislusnou HTTP response.
 * 
 * @param sockfd File descriptor socketu klienta
 */
void handleresponse(int sockfd) { 
	// predpokladam, ze delka requestu neprekroci 1024 znaku.
	char buff[1024];
	read(sockfd, buff, sizeof(buff));
	int option;

	if(strstr(buff, "GET /hostname ") != NULL)
		option = 0;
	else if(strstr(buff, "GET /cpu-name ") != NULL)
		option = 1;
	else if(strstr(buff, "GET /load ") != NULL)
		option = 2;
	else
		option = -1;
	sendresponse(sockfd, option);
}

/**
 * @brief Nastavi http server.
 * 
 * @param portno Cislo portu, na kterem ma server naslouchat
 */
int setupserver(int portno) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		fprintf(stderr, "Error: Could not open socket\n");
		return ERROR;
	}
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(struct sockaddr_in));

	serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr, "Error: Error on binding\n");
		return 1;
	}

	if(listen(sockfd, 1) < 0) {
		fprintf(stderr, "Error: Error on listening\n");
		return ERROR;
	}

	int newsockfd;
	while(1) {
		newsockfd = accept(sockfd, NULL, NULL);
		if(newsockfd < 0) {
			fprintf(stderr, "Error: Could not open client socket\n");
			return ERROR;
		}
		// zpracovani request/response
		handleresponse(newsockfd);
		close(newsockfd);
	}
	close(sockfd);
}

int main(int argc, char **argv) {
	if(argc < 2) {
		fprintf(stderr, "Port number must be specified\n");
		return ERROR;
	}
	
	int portno = atoi(argv[1]);
	if(portno == 0) {
		fprintf(stderr, "Port number must be an integer");
		return ERROR;
	}
	return setupserver(portno);
}
