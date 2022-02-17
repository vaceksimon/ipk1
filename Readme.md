# IPK Projekt 1
Úkolem je vytvoření serveru v jazyce C/C++ komunikujícího prostřednictvím protokolu HTTP, který bude poskytovat různé informace o systému.
**Autorem řešení je Šimon Vacek xvacek10@stud.fit.vutbr.cz**

## Implementace serveru
Implementace se nachází v souboru hinfosvc.c. Aplikace očekává jako parametr číslo portu, na kterém bude server běžet. Pokud jako parametr nedostane číslo, nebo dostane 0, vypíše chybové hlášení a  ukončí se. Jinak se server spustí, poslouchá na localhost na daném portu a čeká na request.
V případě, že by došlo k chybě při nastavení serveru, ukončí se s chybovým kódem 1.

## Spuštění serveru
Program se dá přeložit s použitím Makefile spuštěním `make`, nebo `make all`.
Překlad vytvoří binární soubor hinfosvc, který se spustí `./hinfosvc portno`, kde `portno` je čílo portu, na kterém bude server přijímat requesty, např. 12345.
V případě, že chcete odstranit binární soubor, lze použít `make clean`.

## Funkce serveru
Server se běží na adrese 127.0.0.1 a zprácovává 3 typy dotazů:
- `GET http://localhost:12345/hostname` vrátí síťové jméno počítače včetně domény
- `GET http://localhost:12345/cpu-name` vrátí informace o CPU
- `GET http://localhost:12345/load` vrátí aktuální informaci o zátěži

## Vypnutí serveru
`Server se dá vypnout podržením kláves CTRL+C. V případě, že byl spuštěn na pozadí, je nutné ručně ukončit proces pomocí `kill pid`, kde `pid` je identifikátor procesu.
Např.:
```
./hinfosvc 12345 &
[1] 26720
kill 26720
```

## Zdroje
Při řešení jsem čerpal především z [HTTP Made Really Easy](https://www.jmarshall.com/easy/http/?fbclid=IwAR0vecq6PnRdz6I8PkT8pmhpytXAPp7z5U4zE0kwNYQX_S4c-VoHeMBOzBM) a [Sockets Tutorial](https://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html?fbclid=IwAR22Nv1ySfS8BjsHMhpy2qPbjJQ6TSx0sD7aAqN7AVy5Bv1Sjjn0G3MsMJE). Pro výpočet zátěže procesoru jsem použil [funkci z projektu od Alexander Heinlein](https://github.com/scaidermern/top-processes) licencovaného pod CC0 1.0.
