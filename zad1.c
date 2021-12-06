#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"
#define REZERWA 500
#define PORT 1
#define WYRUSZENIE 2
#define PRZEJAZDZKA 3
#define DOTARCIE_DO_CELU 4
#define TANKOWANIE 5
#define WYPADEK 6
#define TANKUJ 1500
int paliwo = 2000, dlugosc_przejazdzki = 0;
int HAMUJ=1, NIE_HAMUJ=0;
int liczba_procesow;
int nr_procesu;
int ilosc_motorow;
int ilosc_stanowisk=5;
int ilosc_zajetych_stanowisk=0;
int tag=2137;
int wyslij[2];
int odbierz[2];
MPI_Status mpi_status;

void Wyslij(int nr_motoru, int stan)
{
	wyslij[0]=nr_motoru;
	wyslij[1]=stan;
	MPI_Send(&wyslij, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);
	sleep(1);
}

void Port(int liczba_procesow){
	int nr_motoru, status;
	ilosc_motorow = liczba_procesow - 1;
	if(rand()%2==1){
		printf("Piekna pogoda na przejazdzke motorkiem\n");
	}
	else{
		printf("Brzydka pogoda, przejazdzka motorkiem bedzie ciezka\n");
	}
	printf("Jest %d miejsc na parkingu\n", ilosc_stanowisk);
	sleep(2);
	while(ilosc_stanowisk<=ilosc_motorow){
		MPI_Recv(&odbierz,2,MPI_INT,MPI_ANY_SOURCE,tag,MPI_COMM_WORLD, &mpi_status);
		nr_motoru=odbierz[0];
		status=odbierz[1];
		if(status==1){
			printf("Motor o numerze %d stoi na postoju\n", nr_motoru);
		}
		if(status==2){
			printf("Motor o numerze %d wyrusza na przejazdzke ze stanowiska nr %d\n", nr_motoru, ilosc_zajetych_stanowisk);
			ilosc_zajetych_stanowisk--;
		}
		if(status==3){
			printf("Motor o numerze %d smiga jak szalony\n", nr_motoru);
		}
		if(status==4){
			if(ilosc_zajetych_stanowisk<ilosc_stanowisk){
				ilosc_zajetych_stanowisk++;
				MPI_Send(&HAMUJ, 1, MPI_INT, nr_motoru, tag, MPI_COMM_WORLD);
			}
			else{
				MPI_Send(&NIE_HAMUJ, 1, MPI_INT, nr_motoru, tag, MPI_COMM_WORLD);
			}
		}
		if(status==5){
			printf("Motor o numerze %d tankuje\n", nr_motoru);
		}
		if(status==6){
			ilosc_motorow--;
			printf("Ilosc motorow %d\n", ilosc_motorow);
		}
	}
	printf("Program zakonczyl dzialanie\n");
}

void Motor(){
	int stan,suma,i;
	stan=PRZEJAZDZKA;
	while(1){
		if(stan==1){
			if(rand()%2==1){
				stan=WYRUSZENIE;
				dlugosc_przejazdzki = 0;
				printf("Motor o numerze %d jest gotowy do podrozy\n",nr_procesu);
				Wyslij(nr_procesu,stan);
			}
			else{
				Wyslij(nr_procesu,stan);
			}
		}
		else if(stan==2){
			printf("Motor o numerze %d wyrusza na przejazdzke\n",nr_procesu);
			stan=PRZEJAZDZKA;
			Wyslij(nr_procesu,stan);
		}
		else if(stan==3){
			paliwo-=rand()%500; 
			dlugosc_przejazdzki += rand()%500;
			if(dlugosc_przejazdzki >= 2500){
                stan = DOTARCIE_DO_CELU;
		dlugosc_przejazdzki = 0;
                printf("Motor o numerze %d przejechal ponad 200 km\n",nr_procesu);
                Wyslij(nr_procesu, stan);
            }
			else if(paliwo<=REZERWA){
				stan=TANKOWANIE;
				printf("Motor o numerze %d musi zatankowac\n",nr_procesu);
				Wyslij(nr_procesu, stan);
			}
			else{
				for(i=0; rand()%10000;i++);
			}
		}
		else if(stan==4){
			int temp;
			MPI_Recv(&temp, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &mpi_status);
			if(temp == HAMUJ){
				stan=PORT;
				dlugosc_przejazdzki = 0;
				printf("Motor o numerze %d dotarl na parking\n", nr_procesu);
			}
			else
				{
				dlugosc_przejazdzki += rand()%500;
				int wypadek = rand()%10;
				if(wypadek == 3){
					stan = WYPADEK;
					printf("Motor o numerze %d brał udzial w wypadku\n", nr_procesu);
					Wyslij(nr_procesu,stan);
				}
				else if(paliwo>0){
					stan=TANKOWANIE;
					Wyslij(nr_procesu,stan);
				}
				else{
					stan=WYPADEK;
					printf("Motor bral udzial w kolizji\n");
					Wyslij(nr_procesu,stan);
					return;
				}
			}
		}
		else if(stan == 5){
			printf("Motor o numerze %d tankuje\n", nr_procesu);
			paliwo = TANKUJ;
			stan = PRZEJAZDZKA;
			Wyslij(nr_procesu,stan);
		}
	}
}
int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&nr_procesu);
	MPI_Comm_size(MPI_COMM_WORLD,&liczba_procesow);
	srand(time(NULL));
	if(nr_procesu == 0)
		Port(liczba_procesow);
	else 
		Motor();
	MPI_Finalize();
	return 0;
}