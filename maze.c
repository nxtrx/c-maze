#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef struct {
  int rows;
  int cols;
  unsigned char *cells;
} Map;

//cellToBits -> PŘEVÁDÍ ČÍSLA NA JEJICH BITOVOU REPREZENTACI
//hodnost bitů je ve stejném pořadí jako u políčka tímpádem arr[0] == 2^0, arr[1] == 2^2, arr[2] == 2^1
//jednoduše  Levá-stěna == arr[0], Horní/Spodní stěna == arr[1], Pravá stěna == arr[2]

void cellToBits(int num, int *arr){
  int temp;
  int index = 0;

  while (index < 3){

    arr[index] = num % 2;
    num = num / 2;
    index++;

  }

  temp = arr[2];
  arr[2] = arr[1];
  arr[1] = temp;

}

// isValid -> KONTROLUJE VALIDITU MAPY
// Zkontroluje zda sedí velikost která je v prvním řádku mapy s reálnou velikostí načtené mapy
// Poté v prvním průchodu kontroluje levou a pravou hranici
// Druhý průchod kontroluje horní a spodní hranici

// Výstup: 1 = Valid; 0 = Invalid

int isValid(Map map, int checksum){

  if (checksum != map.cols * map.rows)
    return 0;

  int cell1[3];
  int cell2[3];
  int val1;
  int val2;
  int invalidFlag = 0;

  //Průchod polem 
  for (int row = 0; row < map.rows; row++){
    for (int col = 0; col < map.cols - 1; col++){

      //Převod čísel do binární soustavy
      val1 = map.cells[col + (map.cols*row)];
      val2 = map.cells[col + (map.cols*row) + 1];
      cellToBits(val1, cell1);
      cellToBits(val2, cell2);

      //DEBUG/LOG -> Výpis jednotlivých buňek
      //for (int i = 0; i < 3; i++){
      //printf("%d", cell1[i]);
      //}
      //printf("\n");


      //Kontrola zda jsou sousední hranice (levá a pravá) stejné
      if (cell1[2] != cell2[0]){
        invalidFlag = 1;
      }
    }
  }

  int rowCounter = 1; 
  int colCounter = 1;

  //Průchod polem
  for (int row = 0; row < map.rows - 1; row++){
    for (int col = 0; col < map.cols; col++){

      //Převod čísel do binární soustavy
      val1 = map.cells[col + (map.cols*row)];
      val2 = map.cells[col + (map.cols*row) + map.cols];
      cellToBits(val1, cell1);
      cellToBits(val2, cell2);

      //Kontrola zda jsou sousední hranice (spodní a horní [o řádek níž]) stejné
      //Lichý řádek, kontrolujeme pouze sudé buňky
      if(rowCounter % 2 == 1){
        if (colCounter % 2 == 0){
          if (cell1[1] != cell2[1]){
            invalidFlag = 1;
          }
        }
      }

      //Sudý řádek, kontrolujeme pouze liché buňky
      if(rowCounter % 2 == 0){
        if (colCounter % 2 == 1){
          if (cell1[1] != cell2[1]){
            invalidFlag = 1;
          }
        }
      }

      colCounter++;
    }
    colCounter = 1;
    rowCounter++;
  }

  //Vyhodnocení zda je bludiště validní
  if (invalidFlag == 1)
    return 0;
  else
    return 1;
}

//mapCtor -> NAČTENÍ MAPY DO STRUKTURY
//Přečte první 2 čísla ty uloží do map.rows a map.cols
//alokuje paměť o velikosti kterou vypočítá z velikosti bludiště
//uloží zbytek čísel do map.cells

//Výstup: inCount používá se ve validaci abysme věděli jestli je bludiště stejně velké jako zadaná velikost v rows a cols
//        Naplněná struktura, kterou jsme funkci předali

int mapCtor(Map *map, int argc, char **argv){

    FILE *file;
    int intValue;
    int intCount = 0;

    file = fopen(argv[argc-1], "r");

    if (file == NULL) {
        printf("Error opening the file.\n");
        return 1;
    }

    //Načtení velikosti row a col z prvního řádku
    fscanf(file, "%d", &intValue);
    map->rows = intValue;
    fscanf(file, "%d", &intValue);
    map->cols = intValue;

    //DEBUG/LOG -> Vypíše velikost bludiště zadanou v prvním řádku
    //printf("rows: %d cols: %d\n", map->rows, map->cols);

    //Alokování paměti dle velikosti bludiště zadané v prvním řádku
    map->cells = (unsigned char *)malloc(sizeof(unsigned char) * map->cols * map->rows);

    //Načtení bludiště do map->cells
    while (fscanf(file, "%d", &intValue) == 1) {
      map->cells[intCount] = (unsigned char)intValue;
      intCount++;
    }

    fclose(file);
    file = NULL;

    return intCount;
}

//isBorder -> JE TO STĚNA
//Funkci předáme bludiště, políčko a určitou stěnu
//Pokud je v deném políčku daná stěna hodnoty 1 tak tam stěna je, pokud je hodnoty 0 tak tam stěna není

//Výstup: True; False

bool isBorder(Map *map, int r, int c, int border){
  int cell[3];

  //(c - 1) + (map->cols*(r - 1)) odečítá jedničku protože v poli se indexuje od nuly
  cellToBits(map->cells[(c - 1) + (map->cols*(r - 1))], cell);
  
  if (cell[border] == 1)
    return true;
  else
    return false;

}

// validRCinput -> ZJIŠŤUJE ZDA VSTUPUJEME DO BLUDIŠTĚ Z VENKU
// nemůžeme přijít do bludiště od prostřed takže se kouká jestli jsme zadali hodnotu pouze na okraji matice
// pokud začneme od prostředka nevíme odkud jsme přišli(zleva, zprava, shora, zdola)

// Výstup: 1; 0;

int validRCinput(int row, int col, Map *map){

  if (row == 1 || row == map->rows || col == 1 || col == map->cols)
    return 1;
  
  return 0;

}

// start_border -> ZJIŠTUJE POČÁTEČNÍ STĚNU KTERÉ SE MÁME DRŽET
// zjistí odkude jsme přišli (zleva, zprava, shora, zespoda)
// poté se podle pravidel rozhoduje jaká je 
// Funce vyhodnocuje podle priorit pravidel ze zadání [1,2,5,6,3,4]

// Výstup: 0; 1; 2; hranice políčka (levá, pravá, horní/spodní)

int start_border(Map *map, int r, int c, int leftright){
  //(r+c) % 2 = 1; políčko má SPODNÍ hranici
  //(r+c) % 2 = 0; políčko má HORDNÍ hranici

  int border = -1;

  //shora/zdola
  if (r == 1 && (r+c) % 2 == 0 && isBorder(map, r, c, 1) == false){
    border = 1;
  }
    
  if (r == map->rows && (r+c) % 2 == 1 && isBorder(map, r, c, 1) == false){
    border = 1;
  }
    

  //zprava
  if (c == map->cols){
    if (isBorder(map, r, c, 2) == false){
      border = 2;
    }
  }

  //zleva
  if (c == 1){
    if (isBorder(map, r, c, 0) == false){
      border = 0;
    }
  }


 //PRAVÁ RUKA
  if (leftright == 1){
    //PRAVIDLO 1 & 2
    if (border == 0){
      if (r % 2 == 1){
        return 2;
      }
      else{
        return 1;
      }
    }
    
    //PRAVIDLO 5 & 6
    if (border == 2){
      if((r + c) % 2 == 1){
        return 0;
      }
      else{
        return 1;
      }
    }

    //PRAVIDLO 3 & 4
    if(border == 1){
      if ((r + c) % 2 == 1){
        return 2;
      }
      else {
        return 0;
      }
    }
  }

 // LEVÁ RUKA
  if (leftright == -1){
    //PRAVIDLO 1 & 2
    if (border == 0){
      if (r % 2 == 1){
        return 1;
      }
      else{
        return 2;
      }
    }

    //PRAVIDLO 5 & 6
    if (border == 2){
      if((r + c) % 2 == 1){
        return 1;
      }
      else{
        return 0;
      }
    }

    //PRAVIDLO 3 & 4
    if(border == 1){
      if ((r + c) % 2 == 1){
        return 1;
      }
      else {
        return 2;
      }
    }
  }

  return -1;
  
}

// mapDtor -> UVOLNÍ ALOKOVANOU PAMĚŤ
void mapDtor(Map *map){
  free(map->cells);
  map->cells = NULL;
}

//pathFind -> HLEDÁ CESTU
// Zjistí orientaci políčka (Nahoru/Dolů)
// Vypíše políčko kde stojíme
// Otáčí se v políčku od počáteční stěny z funkce isBorder 
// 4 stavy: 
//         1. Pravá ruka && orientace dolů
//                        leftright = 1;
//                        Točí se doleva [entry = entry + 1]                        
//         2. Pravá ruka && orientace nahoru
//                        Točí se doprava [entry = entry - 1]
//         3. Levá ruka && orientace dolů
//                        leftright = -1;
//                        Točí se doprava [entry = entry + (-1)]                        
//         4. Pravá ruka && orientace nahoru
//                        Točí se doleva [entry = entry - (-1)]
// Jakmile najde hranici co není stěna tak se posouvá tím směrem
// Rekurzivně se opakuje dokud nevyjde ven z bludiště

int entry;
int startBorderStop = 0;
void pathFind(Map *map, int r, int c, int leftright){

  if (startBorderStop == 0){
    entry = start_border(map, r, c, leftright);
    startBorderStop = 1;
  }
    

  // Výpočet orientace políčka 
  // Liché == DOLŮ
  // Sudé == NAHORU
  int cellOrientation = (r+c) % 2; // 1 == DOLU, 0 == NAHORU

  //Logika pro ukončení rekuze -> Pokud vyjdeme ven z bludiště vyskočíme z funkce
  if (r < 1 || r > map->rows){
    return;
  } else if (c < 1 || c > map->cols)
    return;

  //TEST -> ANTI WHILE(TRUE) OPATŘENÍ
  //if (limit > 100){
  //  return;
  //}
  //limit++;

  //Výpis políčka na kterém stojíme
  printf("%d,%d\n", r, c);

  //DEBUG/LOG -> 
  //printf("START BORDER: %d\n", entry);

  // Logika pro hledání následující stěny, kterou budeme pokračovat
  // "Otáčení v buňce dokud nenajdeme místo kde stěna chybí"
  if (isBorder(map, r, c, entry) == true){  
    if (cellOrientation == 1){  // Políčko má spodní hranici
      do
      {
        entry = entry + leftright; //podle leftright (1 ; -1) se buď přičítá nebo odečítá
        if (entry > 2)  // "uzavřeme rotaci" aby jsme se nedostali mimo interval (0,2)
          entry = 0;
        else if (entry < 0)
          entry = 2;
      } 
      while (isBorder(map, r, c, entry));
      
    } else {
      do
      {
        entry = entry - leftright; //podle leftright (1 ; -1) se buď přičítá nebo odečítá
        if (entry < 0)  // "uzavřeme rotaci" aby jsme se nedostali mimo interval (0,2)
          entry = 2;
        else if (entry > 2)
          entry = 0;
      } 
      while (isBorder(map, r, c, entry));
      
    }
  }
  //Vyhodnocení do jakého smeru se posouváme ->

  //DOLEVA
  if (entry == 0){
    c = c - 1;

    if (cellOrientation == 1){

      //Vypočítámé následující "počáteční políčko" jako ve funkci start_border, ale vycházíme z políčka ze kterého jsme přišli
      //Stejná logika je i u ostatních 
      entry = 2 - leftright;
      if (entry > 2)
        entry = 0;
      else if (entry < 0)
        entry = 2;


    } else {
      entry = 2 + leftright;
      if (entry < 0)
        entry = 2;
      else if (entry > 2)
        entry = 0;
    }
  }

  //DOPRAVA
  else if (entry == 2){
    c = c + 1;

    if (cellOrientation == 1){

      entry = 0 - leftright;
      if (entry > 2)
        entry = 0;
      else if (entry < 0)
        entry = 2;
    } 
    
    else {
      entry = 0 + leftright;
      if (entry < 0)
        entry = 2;
      else if (entry > 2)
        entry = 0;
    }
  }

  //NAHORU / DOLU podle orientace políčka
  else if (entry == 1){

    if (cellOrientation == 1){
      r = r + 1;
      entry = 1 - leftright;
      if (entry > 2)
        entry = 0;
      else if (entry < 0)
        entry = 2;
    }
    
     else {
      r = r - 1;
      entry = 1 + leftright;
      if (entry < 0)
        entry = 2;
      else if (entry > 2)
        entry = 0;
    }
  }

  //REKURZE
  pathFind(map, r, c, leftright);
}

int main(int argc, char **argv) {

  //[too few args] -> error
  if (argc < 2){
    printf("ERROR: Too few arguments. \n help type: maze --help\n");
  }

  //--help
  //výpis toho jak funkce funguje
  if (strcmp(argv[1], "--help") == 0){
    printf("Usage: %s <command> [R] [C] [FILE]\n\n", argv[0]);
    printf("Commands: \n");
    printf("       --help            Displays help                       Required args: no arguments\n");
    printf("       --test            Tests if the maze is valid          Required args: [FILE]\n");
    printf("       --lpath           Finds path with left hand rule      Required args: [R] [C] [FILE]\n");
    printf("       --rpath           Finds path with right hand rule     Required args: [R] [C] [FILE]\n\n");
    printf("Arguments: \n");
    printf("       R                 Starting ROW for finding path\n");
    printf("       C                 Starting COLUMN for finding path\n");
    printf("       FILE              path to .txt file where the maze is stored\n");
  }

  //--test
  //volá funkce pro validaci bludiště
  if (strcmp(argv[1], "--test") == 0){
    if (argc < 3)
      return 1;

    int checkSum;
    Map maze;

    //Počet prvků v matici pokud nesedí počet prvků a její rozměr fuknce isValid poté vyhodnotí bludiště jako Invalid
    checkSum = mapCtor(&maze, argc, argv);

    if (isValid(maze, checkSum) == 1)
      printf("Valid\n");
    else  
      printf("Invalid\n");

    mapDtor(&maze);
  }

  //--lpath
  //volá funkci findPath() s argumentem leftright nastaveným jako -1 == DOLEVA
  if (strcmp(argv[1], "--lpath") == 0){
    Map maze;
    mapCtor(&maze, argc, argv);

    int r = atoi(argv[2]);
    int c = atoi(argv[3]);
    //int limit = 0;

    if(validRCinput(r, c, &maze) == 0){
      printf("ERROR: Wrong value in R or C.");
      return 1;
    }

    // int leftright -> -1 == left, 1 == right;

    int leftright = -1;
    int entry = start_border(&maze, r, c, leftright);

    if (entry == -1){
      printf("Invalid cell, couldn't find entry.");
      return 1;
    }

    pathFind(&maze, r, c, leftright);

    mapDtor(&maze);
  }
  
  //--rpath
  //volá funkci findPath() s argumentem leftright nastaveným jako 1 == DOPRAVA
  if (strcmp(argv[1], "--rpath") == 0){
    Map maze;
    mapCtor(&maze, argc, argv);

    int r = atoi(argv[2]);
    int c = atoi(argv[3]);
    //int limit = 0;

    if(validRCinput(r, c, &maze) == 0){
      printf("ERROR: Wrong value in R or C.");
      return 1;
    }

    // int leftright -> -1 == left, 1 == right;

    int leftright = 1;
    int entry = start_border(&maze, r, c, leftright);

    if (entry == -1){
      printf("Invalid cell, couldn't find entry.");
      return 1;
    }

    pathFind(&maze, r, c, leftright);

    mapDtor(&maze);
  }
   return 0;
}