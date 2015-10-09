#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define COORDO 0
#define BOOTS 1

#define AX 0
#define AY 1
#define BX 2
#define BY 3
#define CARD 4
#define RANK 5

#define NORD 4
#define SUD 5
#define OUEST 6
#define EST 7

#define INSERT 1000
#define ADDED 1001
#define SWCH_V 1002
#define N_VOIS 1003
#define SEARCH 1004
#define FIRST 1005
#define DATA_IN 1006
#define DELE 1007
#define MAJ 1008
#define SYNC 1009

#define NBDEM 10
#define NBPROC 4
struct Espace
{
	int aX;
	int aY;
	int bY;
	int bX;
	int rank;
}typedef Espace;

typedef struct Liste Liste;
typedef struct ListeData ListeData;
struct Liste
{
	Liste* next;
	Espace* data;
};
struct Point
{
	int pointX;
	int pointY;
	int rank;
}typedef Point;

struct DHT
{
	int valX;
	int valY;
	int valeur;
	int dest;
}typedef DHT;

struct ListeData{
	DHT* data;
	ListeData *next;
};

Liste *voisinOuest,*voisinNord,*voisinEst,*voisinSud;
ListeData *liste_dht = NULL;

void coordinateur(){
	printf("Je suis coordinateur \n");
}

/*Méthode pour supprimer une structure espace de la liste */
Liste* suppressionVoisin(Liste *voisin, int rank, int myRank){
	Liste *prec,*aSup,*retour;
	aSup = voisin;
	retour = voisin;
	prec = NULL;
	if(voisin == NULL){
		return NULL;
	}
	while(aSup != NULL && aSup->data->rank != rank){
		prec = aSup;
		aSup = aSup->next;
	}
	if(aSup == NULL){
		printf("Le voisin %d a supprimer n'est pas dans cette liste de voisin \n",  rank);
	}else{
		if(voisin->data == aSup->data){
			printf("Je supprime le 1er element de ma liste %d \n", aSup->data->rank);
			if(aSup->next != NULL){
				voisin = aSup->next;
			}else{
				voisin = NULL;
			}
		}else{
			printf("Le voisin %d a supprimer est dans cette liste donc je le supprime \n",  rank);
			prec->next = aSup->next;
		}
		MPI_Send(&myRank,1,MPI_INT,aSup->data->rank,DELE,MPI_COMM_WORLD);
		free(aSup->data);
		free(aSup);
	}
	return voisin;
}

/*Mise à jour d'une structure espace dans une liste */
Liste* updateEspaceVoisin(Liste *voisins, int tab_espace[5], int rank, Espace *mon_espace){
	Liste *tmp = voisins;
	while(tmp != NULL && tmp->data->rank != rank){
		tmp = tmp->next;
	}

	if(tmp != NULL){
		printf("Je met a jour l'espace du voisin %d \n",tmp->data->rank );
		tmp->data->aX=tab_espace[AX];
		tmp->data->aY=tab_espace[AY];
		tmp->data->bX=tab_espace[BX];
		tmp->data->bY=tab_espace[BY];
		if(estMonVoisin(mon_espace,tmp->data) == 0){
			voisins=suppressionVoisin(voisins,rank,mon_espace->rank);	
		}
	}
	return voisins;
}

/*Méhtode pour savoir si 2 espaces ont des segments communs*/
int segmentCommun(int voisX1, int voisX2, int monX1,int monX2){

	if(voisX1 >= monX1 && voisX1 <= monX2 && voisX2 >= monX1 && voisX2 <= monX2){
		printf("On compare %d >= %d && %d <= %d && %d >= %d && %d <= %d \n", voisX1,monX1, voisX1,monX2,voisX2,monX1,voisX2,monX2);
		printf("Cas 1 \n");
		return 1;
	}
	/*if( voisX1 == monX1 && voisX2 == monX2){
		return 1;
	}*/
	if( voisX1 >= monX1 && voisX1 < monX2 && voisX2 > monX2){
		printf("On compare %d >= %d && %d < %d && %d > %d \n", voisX1,monX1, voisX1,monX2,voisX2,monX2);
		printf("Cas 2\n");
		return 1;
	}
	if( voisX2 > monX1 && voisX2 <= monX2 && voisX1 < monX1){
		printf("On compare %d > %d && %d <= %d && %d < %d  \n", voisX2,monX1, voisX2,monX2,voisX1,monX1);
		printf(" Cas 3\n");
		return 1;
	}
	return 0;
}

/*Méthode pour savoir s'il 2 espaces sont voisins */
int estMonVoisin(Espace *mon_espace, Espace *espace_vois){
	if(segmentCommun(espace_vois->aX, espace_vois->bX,mon_espace->aX,mon_espace->bX) == 1){
		if(espace_vois->aY >= mon_espace->bY ){
			printf("Va au nord\n");
			if(espace_vois->aY == mon_espace->bY){
							return NORD;
			}else{
				return 0;
			}
		}else if( espace_vois->bY <= mon_espace->aY ){
			printf("Va au sud \n");
			if(espace_vois->bY == mon_espace->aY){
				return SUD;
			}else{
				return 0;
			}
		}else{
			printf("Probleme au niveau des voisins N/S \n");
			return -1;
		}
	}else if(segmentCommun(espace_vois->aY, espace_vois->bY,mon_espace->aY,mon_espace->bY) == 1){
		if(espace_vois->aX >= mon_espace->bX ){
			printf("Va au est\n");
			if(espace_vois->aX == mon_espace->bX){
				return EST;
			}else{
				return 0;
			}
		}else if(espace_vois->bX <= mon_espace->aX){
			printf("Va au ouest \n");
			if(espace_vois->bX == mon_espace->aX){
				return OUEST;
			}else{
				return 0;
			}
		}else{
			printf("Probleme au niveau des voisins E/O soit propage soit pas chez moi \n");
			return -1;
		}
	}else{
		printf("Pas mon voisin \n");
		return 0;
	}
}

/*Envoie d'un message SWITCH pour remplacer un voisin*/
Liste* send_switch_voisin(Espace *espace, Liste *voisins){
	Liste *tmp = voisins;
	Liste *temp;
	int tab_espace[6];
	printf("Debut envoi switch voisin bis\n");
	while(tmp != NULL){
		tab_espace[AX] = espace->aX;
		tab_espace[BX] = espace->bX;
		tab_espace[AY] = espace->aY;
		tab_espace[BY] = espace->bY;
		tab_espace[RANK] = espace->rank;
		tab_espace[CARD] = 0;
		tab_espace[AX] = espace->aX;
		printf("J envoie a %d pour switcher le remplacant n°%d \n",tmp->data->rank,espace->rank );
		MPI_Send(tab_espace,6,MPI_INT,tmp->data->rank,SWCH_V,MPI_COMM_WORLD);
		tmp = tmp->next;
	}

	tmp = voisins;
	temp = voisins;
	while(tmp != NULL){
		temp = tmp;
		tmp = tmp->next;
		printf("J'efface le proc %d car nouveau voisin s'intercale entre nous \n", temp->data->rank );
		free(temp->data);
		free(temp);
	}
	if(tmp == NULL){
		printf("Alors c'est bon \n");
	}
	printf("Fin envoi switch voisin\n");
	return NULL;
}

/*Reception d'un msg SWITCH */
Liste* rcv_switch_voisin_bis(int voisinARemplacer,Espace *espace_vois, Liste *voisins, Espace *mon_espace,int card){
	if(voisins == NULL){
		printf("Liste vide \n");
		return NULL;
	}

	printf("Debut du changement je vais remplacer %d \n", voisinARemplacer);
	Liste *tmp = voisins;
	Liste *prec = NULL;
	while(tmp != NULL && tmp->data->rank != voisinARemplacer){
		printf("Je cherche mon voisin \n");
		prec = tmp;
		tmp = tmp->next;
	}
	int monVoisin = estMonVoisin(mon_espace,espace_vois);
	if(monVoisin = card){
		int tab_espace[5];
		tab_espace[AX] = mon_espace->aX;
		tab_espace[AY] = mon_espace->aY;
		tab_espace[BX] = mon_espace->bX;
		tab_espace[BY] = mon_espace->bY;
		tab_espace[CARD]=0;
		printf("J'envoie a %d mon espace a(%d,%d) b(%d,%d) \n", espace_vois->rank,tab_espace[AX],tab_espace[AY],tab_espace[BX],tab_espace[BY]);
		MPI_Send(tab_espace,5,MPI_INT,espace_vois->rank,ADDED,MPI_COMM_WORLD);
	}
	if(tmp != NULL ){
		//free(tmp->data);
		if( monVoisin > 1){
			tmp->data = espace_vois;
			printf("J'ai trouve le voisin %d et je le remplace par %d \n",voisinARemplacer,espace_vois->rank );
		}else{
			printf(" J'ai trouve le voisin %d mais l'espace de %d n'est pas proche de moi \n", tmp->data->rank,espace_vois->rank );
			if(prec == NULL){
				voisins = tmp->next;
			}else if(tmp->next != NULL){
				prec->next = tmp->next;
			}else{
				prec->next=NULL;
			}
			//free(e);
			free(tmp);
		}
	}else{
		printf("Le proc %d pas dans cette liste de voisin\n",voisinARemplacer);
	}
	return voisins;
}

/*Envoi d'un message nouveau N_VOIS*/
void send_nouveau_voisin(Espace *espace, Liste *voisins){
	int tab_espace[6];
	tab_espace[AX] = espace->aX;
	tab_espace[AY] = espace->aY;
	tab_espace[BX] = espace->bX;
	tab_espace[BY] = espace->bY;
	tab_espace[CARD] = 0;
	tab_espace[RANK]=espace->rank;
	printf("J'envoie comme nouveau voisin le rank %d \n",tab_espace[RANK] );
	Liste *tmp = voisins;
	printf("Debut envoi nouveau voisin \n");
	while(tmp != NULL){
		printf("J'envoie a %d qu'il doit ajouter mon voisin %d \n",tmp->data->rank,espace->rank );
		MPI_Send(tab_espace,6,MPI_INT,tmp->data->rank,N_VOIS,MPI_COMM_WORLD);
		tmp = tmp->next;
	}
	printf("Fin envoi nouveau voisin\n");
}

/*Envoie d'un msg ADDED aux voisins que l'espace vient d'ajouter*/
void send_ajouter_voisin(Espace *mon_espace, int rank){
	int tab_espace[5];
	tab_espace[AX] = mon_espace->aX;
	tab_espace[AY] = mon_espace->aY;
	tab_espace[BX] = mon_espace->bX;
	tab_espace[BY] = mon_espace->bY;
	tab_espace[CARD] = 0;
	printf("Je repond au voisin %d que je viens d'ajouter pour qu'il me rajoute \n", rank);
	MPI_Send(tab_espace,5,MPI_INT,rank,ADDED,MPI_COMM_WORLD);
}

/*Méhode pour savoir si un point est dans l'espace d'un autre point*/
int est_dans_espace(Espace *mon_espace, Point *point){
	if( point->pointX > mon_espace->aX && point->pointX < mon_espace->bX && point->pointY > mon_espace->aY && point->pointY < mon_espace->bY){
		printf("Le point a ajoute se trouve dans mon espace \n");			
		return 0;
	}
	printf("Le point a ajoute ne se trouve pas dans mon espace rank \n");		
	return 1;
}

/*Affichage des voisins d'un point*/
void afficheVoisin(Point *point,Liste *voisin, char * cote){
	Liste* tmp_voisin = voisin;
	printf("Pour le point rank %d Debut affichage voisin cardinalite %s   \n",point->rank, cote);
	while(tmp_voisin != NULL){
		printf("Voisin de %d : A(%d,%d),B(%d,%d), rank %d \n",point->rank,tmp_voisin->data->aX,tmp_voisin->data->aY,tmp_voisin->data->bX,tmp_voisin->data->bY, tmp_voisin->data->rank);
		tmp_voisin = tmp_voisin->next;
	}
}


Liste* ajouterVoisin(Liste *voisin, Espace *espace, Espace *mon_espace){
		Liste* tmp_voisin =(Liste*) malloc(sizeof(Liste));
		Liste* tmp= voisin;
		int ok = 0;
		int tab_espace[6];
		printf(" je dois ajouter %d a(%d,%d) b(%d,%d)\n", espace->rank, espace->aX, espace->aY,espace->bX, espace->bY);
		while(tmp != NULL){
			if(tmp->data->aX == espace->aX && tmp->data->bX == espace->bX){
					if(tmp->data->aY == espace->bY){
						printf("C'est le voisin sud %d a(%d,%d) b(%d,%d) \n",tmp->data->rank,tmp->data->aX,tmp->data->aY,tmp->data->bX,tmp->data->bY);
						ok = 1;
						tab_espace[CARD]=SUD;
						break;
					}else if(tmp->data->bY == espace->aY){
						printf("C'est le voisin nord %d a(%d,%d) b(%d,%d) \n",tmp->data->rank,tmp->data->aX,tmp->data->aY,tmp->data->bX,tmp->data->bY);
						ok = 1;
						tab_espace[CARD]= NORD;
						break;
					}
			}else if(tmp->data->aY == espace->aY && tmp->data->bY == espace->bY){
				printf("C'est le voisin du voisin  est ou ouest \n");
					if(tmp->data->aX == espace->bX){
						printf("C'est le voisin ouest %d a(%d,%d) b(%d,%d) \n",tmp->data->rank,tmp->data->aX,tmp->data->aY,tmp->data->bX,tmp->data->bY);
						ok = 1;
						tab_espace[CARD]= OUEST;
						break;
					}else if(tmp->data->bX == espace->aX){
						printf("C'est le voisin est %d a(%d,%d) b(%d,%d) \n",tmp->data->rank,tmp->data->aX,tmp->data->aY,tmp->data->bX,tmp->data->bY);
						ok = 1;
						tab_espace[CARD]=EST;
						break;
					}
			}
			tmp = tmp->next;
		}
		if( tmp != NULL){
			tab_espace[AX] =espace->aX;
			tab_espace[AY] =espace->aY;
			tab_espace[BX] =espace->bX;
			tab_espace[BY] =espace->bY;
			tab_espace[RANK] =espace->rank;
			printf(" Au lieu d'ajouter l'espace chez moi je le propage a un de mes voisin \n");
			MPI_Send(tab_espace,6,MPI_INT,tmp->data->rank,N_VOIS,MPI_COMM_WORLD);
			free(tmp_voisin);
			return voisin;
		}else{
			tmp = voisin;
			while(tmp != NULL && tmp->data->rank != espace->rank){
				tmp = tmp->next;
			}
			if(tmp == NULL){
				int cote;
				if(voisin == NULL){
					tmp_voisin->next = NULL;
				}else{
					tmp_voisin->next = voisin;
				}
				tmp_voisin->data = espace;
				printf("Le voisin ajoute est rank %d, A(%d,%d), B(%d,%d) ajouter\n",tmp_voisin->data->rank, tmp_voisin->data->aX,tmp_voisin->data->aY,tmp_voisin->data->bX,tmp_voisin->data->bY, tmp_voisin->data->rank);
				return tmp_voisin;
			}else{
				printf("J'ai deja le voisin %d dans ma liste\n", tmp->data->rank);
				return voisin;
			}
		}	
}

/*Ajout d'un donnée DHT a la liste d'un point*/
ListeData* ajouterData(ListeData *liste, DHT *dht){
		ListeData *tmp_data =(ListeData*) malloc(sizeof(ListeData));
		tmp_data->data = dht;
		tmp_data->next = liste;
		printf("Ajout de la data x :%d y:%d valeur %d \n",tmp_data->data->valX,tmp_data->data->valY,tmp_data->data->valeur);
		return tmp_data;
}

/*Création des valeur DHT par le coordinateur*/
ListeData* insertion_valeur(ListeData *tab,int n){
	int i =0;
	int tab_DHT[4];
	tab_DHT[3]=0;
	int index = 0;
	for(i = 0; i<n*NBDEM;i++){
		tab_DHT[0]=rand()%(1000+1);
		tab_DHT[1]=rand()%(1000+1);
		tab_DHT[2]=tab_DHT[1]+tab_DHT[0];
		printf("DHT cree x : %d, y: %d , valeur : %d \n", tab_DHT[0],tab_DHT[1], tab_DHT[2]);
		if(i < 5 || i >= ((n*NBDEM)-5) ){	
			DHT *tmp_dht = malloc(sizeof(DHT));
			tmp_dht->valX=tab_DHT[0];
			tmp_dht->valY = tab_DHT[1];
			tmp_dht->valeur=tab_DHT[2];
			tab=ajouterData(tab, tmp_dht);
			//printf("DHT sauvegarder x : %d, y: %d , valeur : %d \n", tab->data->valX,tab->data->valY, tab->data->valeur);
		}
		printf("J'envoie au bootsrap une demande d'insertion \n");
		MPI_Send(tab_DHT,4,MPI_INT,BOOTS,DATA_IN,MPI_COMM_WORLD);
	}
	return tab;
}
/*Méhode inutile*/
Liste* redirection_nord_sud(Point *point, Liste *voisins){
	Liste* tmp_voisin = voisins;
	while( tmp_voisin != NULL){
	//printf("Espace voisin rank %d A(%d,%d), B(%d,%d) \n", tmp_voisin->data->rank,tmp_voisin->data->aX,tmp_voisin->data->aY,tmp_voisin->data->bX,tmp_voisin->data->bY);
		if(point->pointY > tmp_voisin->data->aY && point->pointY < tmp_voisin->data->bY){
			printf("Je donne le point a mon voisin %d \n", tmp_voisin->data->rank );
			//MPI_Send(tab_point,3,MPI_INT,tmp_voisin->data->rank,INSERT,MPI_COMM_WORLD);
			break;
		}
		tmp_voisin = tmp_voisin->next;
	}
	return tmp_voisin;
}

/*Méthode d'insertion d'un point*/
void insertion_noeud(Point *point,Espace *espace, Point *pointAjouter){
	printf("Mon espace est A(%d,%d), B(%d,%d) mes coord a ajoute(%d,%d) rank : %d\n", espace->aX, espace->aY, espace->bX,espace->bY, pointAjouter->pointX, pointAjouter->pointY,pointAjouter->rank);
	Liste* tmp_voisin;
	if(est_dans_espace(espace,pointAjouter) == 0){
		printf("Dans mon espace \n");
		int tab_espace[5];
		int cote;
		int coteX = espace->bX - espace->aX;
		int coteY = espace->bY - espace->aY;
		Espace *new_espace = malloc(sizeof(Espace));
		new_espace->aX = espace->aX;
		new_espace->aY = espace->aY;
		new_espace->bY = espace->bY;
		new_espace->bX = espace->bX;
		new_espace->rank = pointAjouter->rank;
		if( coteX > coteY ){// cas rectangle allongé
			printf("rectangle allongé\n");
			if(pointAjouter->pointX < (coteX/2 +espace->aX) && point->pointX < (coteX/2 +espace->aX)){
				printf("Je le met a l'est car les 2 point < coteX/2 \n");
				new_espace->aX = espace->aX +( coteX/2);
				espace->bX = espace->aX +( coteX/2);
				voisinEst=send_switch_voisin(new_espace,voisinEst);
				tab_espace[4] = OUEST;//Direction de l'espace du point pour le voisin
			}else if(pointAjouter->pointX > (coteX/2 +espace->aX) && point->pointX > (coteX/2 +espace->aX)){
				printf("Je le met a l'ouest car les 2 points > coteX/2 \n");
				new_espace->bX = espace->aX +( coteX/2);
				espace->aX = espace->aX +( coteX/2);
				voisinOuest=send_switch_voisin(new_espace,voisinOuest);
				tab_espace[4] = EST;//Direction de l'espace du point pour le voisin
			}else if(pointAjouter->pointX > point->pointX){ // vas a ouest Mon
				printf("Dans rectangle allonge, il va a Ouest \n");
				new_espace->aX = espace->aX + (coteX/2);
				espace->bX = espace->aX + (coteX/2);
				voisinOuest=send_switch_voisin(new_espace,voisinOuest);
				tab_espace[4] = EST;//Direction de l'espace du point pour le voisin
			}else{// vas a l'est
				printf("Dans rectangle allonge, il va a Est \n");
				new_espace->bX = espace->aX + (coteX/2);
				espace->aX = espace->aX + (coteX/2);
				voisinEst=send_switch_voisin(new_espace,voisinEst);
				tab_espace[4] = OUEST;//Direction de l'espace du point pour le voisin
			}
			printf("Pre Send voisin vers Sud Nord\n");
			send_nouveau_voisin(new_espace,voisinNord) ;
			send_nouveau_voisin(new_espace,voisinSud) ;
			cote=estMonVoisin(espace,new_espace);
			if(cote == EST){
				printf("Ajoute a l'est \n");
				voisinEst=ajouterVoisin(voisinEst,new_espace,espace);
			}else if(cote == OUEST){
				printf("Ajout a l'ouest \n");
				voisinOuest=ajouterVoisin(voisinOuest,new_espace,espace);
			}else{
				printf("Bizarre cote vaut %d \n",cote);
			}
		}else if (coteX < coteY ){// cas rectangle debout
			printf("rectangle debout \n");
			if(pointAjouter->pointY < (coteY/2 +espace->aY) && point->pointY < (coteY/2 +espace->aY) ){
				new_espace->aY = espace->aY + (coteY/2);
				espace->bY = espace->aY + (coteY/2);
				voisinNord=send_switch_voisin(new_espace,voisinNord);
				tab_espace[4] = SUD;//Direction de l'espace du point pour le voisin
			}else if(pointAjouter->pointY > (coteY/2 +espace->aY) && point->pointY > (coteY/2 +espace->aY)){
				new_espace->bY = espace->aY + (coteY/2);
				espace->aY = espace->aY + (coteY/2);
				voisinSud=send_switch_voisin(new_espace,voisinSud);
				tab_espace[4] = NORD;//Direction de l'espace du point pour le voisin
			}else if(pointAjouter->pointY > point->pointY){ // vas a ouest Mon
				printf("Dans rectangle debout, il va au Nord \n");
				new_espace->aY = espace->aY + (coteY/2);
				espace->bY = espace->aY + (coteY/2);
				voisinNord=send_switch_voisin(new_espace,voisinNord);
				tab_espace[4] = SUD;//Direction de l'espace du point pour le voisin
			}else{// vas a l'est
				printf("Dans rectangle debout, il va au Sud \n");
				new_espace->bY = espace->aY + (coteY/2);
				espace->aY = espace->aY + (coteY/2);
				voisinSud=send_switch_voisin(new_espace,voisinSud);
				tab_espace[4] = NORD;//Direction de l'espace du point pour le voisin
			}
			printf("Pre Send voisin Ouest Est\n");
			send_nouveau_voisin(new_espace,voisinOuest) ;
			send_nouveau_voisin(new_espace,voisinEst) ;
			cote=estMonVoisin(espace,new_espace);
			if(cote == NORD){
				printf("Ajoute au nord \n");
				voisinNord=ajouterVoisin(voisinNord,new_espace,espace);
			}else if(cote == SUD){
				printf("Ajout au sud \n");
				voisinSud=ajouterVoisin(voisinSud,new_espace,espace);
			}else{
				printf("Bizarre 2 cote vaut %d \n",cote);
			}
		}else{// carré
			printf("Carre\n");
			if( pointAjouter->pointX < (coteX/2 +espace->aX ) && point->pointX < (coteX/2 +espace->aX)){
				//PointAjouter va aller a l'est
				printf("Je le met a l'est car les 2 point < coteX/2 \n");
				new_espace->aX = espace->aX +( coteX/2);
				espace->bX = espace->aX +( coteX/2);
				voisinEst=send_switch_voisin(new_espace,voisinEst);
				tab_espace[4] = OUEST;//Direction de l'espace du point pour le voisin
			}
			else if(pointAjouter->pointX > (coteX/2 +espace->aX) && point->pointX > (coteX/2 +espace->aX)){
				// PointAjouter va aller a l'ouest
				printf("Je le met a l'ouest car les 2 points > coteX/2 \n");
				new_espace->bX = espace->aX +( coteX/2);
				espace->aX = espace->aX +( coteX/2);
				voisinOuest=send_switch_voisin(new_espace,voisinOuest);
				tab_espace[4] = EST;//Direction de l'espace du point pour le voisin
			}
			else if(pointAjouter->pointX < point->pointX){
				printf("Je le met a l'ouest \n");
				new_espace->bX = espace->aX +( coteX/2);
				espace->aX = espace->aX +( coteX/2);
				voisinOuest=send_switch_voisin(new_espace,voisinOuest);
				tab_espace[4] = EST;//Direction de l'espace du point pour le voisin
			}else{
				printf("Je le met a l'est \n");
				new_espace->aX = espace->aX +( coteX/2);
				espace->bX = espace->aX +( coteX/2);
				voisinEst=send_switch_voisin(new_espace,voisinEst);
				tab_espace[4] = OUEST;//Direction de l'espace du point pour le voisin
			}
			printf("Pre Send voisin Sud Nord 2\n");
			send_nouveau_voisin(new_espace,voisinNord) ;
			send_nouveau_voisin(new_espace,voisinSud) ;
			cote=estMonVoisin(espace,new_espace);
			if(cote == EST){
				printf("Ajoute a l'est pour carre \n");
				voisinEst=ajouterVoisin(voisinEst,new_espace,espace);
			}else if(cote == OUEST){
				printf("Ajout a l'ouest pour carre \n");
				voisinOuest=ajouterVoisin(voisinOuest,new_espace,espace);
			}else{
				printf("Bizarre 3\n");
			}
		}
		//Envoie reponse au point
		tab_espace[AX] = new_espace->aX;
		tab_espace[AY] = new_espace->aY;
		tab_espace[BX] = new_espace->bX;
		tab_espace[BY] = new_espace->bY;
		//tab_espace[RANK] = point->rank;
		printf("Envoi espace au voisin %d a(%d,%d) b(%d,%d) \n", pointAjouter->rank,new_espace->aX,new_espace->aY,new_espace->bX,new_espace->bY);
		MPI_Send(tab_espace,5,MPI_INT,pointAjouter->rank,FIRST,MPI_COMM_WORLD);
		tab_espace[AX] = espace->aX;
		tab_espace[AY] = espace->aY;
		tab_espace[BX] = espace->bX;
		tab_espace[BY] = espace->bY;
		Liste *tmp;
		
		if(tab_espace[CARD] == NORD){
			printf("Mon rank %d je dis au proc %d de me mettre au nord mon espace a(%d,%d) b(%d,%d) \n",point->rank,pointAjouter->rank, espace->aX,espace->aY,espace->bX,espace->bY );
			MPI_Send(tab_espace,5,MPI_INT,pointAjouter->rank,ADDED,MPI_COMM_WORLD);
			tmp = voisinNord;
		}else if(tab_espace[CARD] == SUD){
			printf("Mon rank %d je dis au proc %d de me mettre au sud mon espace a(%d,%d) b(%d,%d) \n",point->rank,pointAjouter->rank, espace->aX,espace->aY,espace->bX,espace->bY );
			MPI_Send(tab_espace,5,MPI_INT,pointAjouter->rank,ADDED,MPI_COMM_WORLD);
			tmp=voisinSud;
		}else if(tab_espace[CARD] == EST){
			printf("Mon rank %d je dis au proc %d de me mettre a l est mon espace a(%d,%d) b(%d,%d) \n",point->rank,pointAjouter->rank, espace->aX,espace->aY,espace->bX,espace->bY );
			MPI_Send(tab_espace,5,MPI_INT,pointAjouter->rank,ADDED,MPI_COMM_WORLD);
			tmp=voisinEst;
		}else{
			printf("Mon rank %d je dis au proc %d de me mettre a l ouest mon espace a(%d,%d) b(%d,%d) \n",point->rank,pointAjouter->rank, espace->aX,espace->aY,espace->bX,espace->bY );
			MPI_Send(tab_espace,5,MPI_INT,pointAjouter->rank,ADDED,MPI_COMM_WORLD);
			tmp = voisinOuest;
		}

		printf("Je verifie si toutes mes data sont toujours dont mon espace\n");
		ListeData *tmp_liste_dht = liste_dht;
		ListeData *t;
		t=NULL;
		Point *p = malloc(sizeof(Point));
		int tab_DHT[4];
		int aSup = 0;
		while(tmp_liste_dht != NULL){
			p->pointY = tmp_liste_dht->data->valY;
			p->pointX = tmp_liste_dht->data->valX;
			if(est_dans_espace(espace,p) == 0){
				printf("Je garde la data x: %d , y:%d dans mon espace \n", p->pointX,p->pointY);
			}else{
				tab_DHT[0] = tmp_liste_dht->data->valX;
				tab_DHT[1] = tmp_liste_dht->data->valY;
				tab_DHT[2] = tmp_liste_dht->data->valeur;
				printf("La data ne se trouve plus dans mon espace je la donne a %d \n", pointAjouter->rank);
				MPI_Send(tab_DHT,4,MPI_INT,pointAjouter->rank,DATA_IN,MPI_COMM_WORLD);
				aSup = 1;
				if(t != NULL){
					t->next = tmp_liste_dht->next;
				}
			}

			t = tmp_liste_dht;
			tmp_liste_dht = tmp_liste_dht->next;
			if(aSup == 1){
				free(t);
				t = NULL;
				aSup = 0;
			}
		}
	}else{
		//printf("A envoye au voisin \n");
		int aleatoire;
		int tour = 1;
		int tab_point[3];
		tab_point[0]=  pointAjouter->pointX;
		tab_point[1] = pointAjouter->pointY;
		tab_point[2] = pointAjouter->rank;
		Liste *v;
		//printf("Je dois placer p(%d,%d) rank : %d \n",tab_point[0],tab_point[1],tab_point[2] );
		while(tour == 1){
			aleatoire = rand()%2;
			if(pointAjouter->pointX < espace->aX && aleatoire == 0){
				printf("Va a mon ouest \n");
				tmp_voisin = voisinOuest;
				v=voisinOuest;
				tour = 0;
			}else if (pointAjouter->pointX > espace->bX && aleatoire == 0){
				printf("Va a mon est \n");
				tmp_voisin = voisinEst;
				v=voisinEst;
				tour = 0;
			}else if(pointAjouter->pointY < espace->aY && aleatoire == 1){
				printf("Va a mon Sud \n");
				tmp_voisin = voisinSud;
				v = voisinSud;
				tour = 0;
			}else if(pointAjouter->pointY > espace->bY && aleatoire == 1){
				printf("Va a mon Nord \n");
				tmp_voisin = voisinNord;
				v = voisinNord;
				tour = 0;
			}
		}
		//Propagation aléatoire sur un des voisin
		if(tmp_voisin != NULL){
		while( tmp_voisin != NULL){
			aleatoire = rand()%2;
			if( aleatoire != 1){
				tmp_voisin = tmp_voisin->next;
				if(tmp_voisin == NULL){
					tmp_voisin = v;
				}
			}else{
				break;
			}
		}
		printf("Je donne a mon voisin %d \n", tmp_voisin->data->rank);
		MPI_Send(tab_point,3,MPI_INT,tmp_voisin->data->rank,INSERT,MPI_COMM_WORLD);
		}else{
			printf("Je n'ai pas de voisin donc je redonne au bootsrap \n");
			MPI_Send(tab_point,3,MPI_INT,BOOTS,INSERT,MPI_COMM_WORLD);
		}
	}
}

/*Boucle des points*/
void boucle_proc(Point *point,Espace *mon_espace){
	int tab_point[3];
	int tab_espace[6];
	MPI_Status status;
	int msg;
	while(1){
		//MPI_IProbe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,msg,&status);
		MPI_Probe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
		printf("Source : %d , tag : %d pour le proc n°%d \n", status.MPI_SOURCE, status.MPI_TAG,point->rank);
		if(status.MPI_TAG == INSERT){
			MPI_Recv(tab_point,3,MPI_INT,MPI_ANY_SOURCE,INSERT,MPI_COMM_WORLD,&status);
			Point *tmp = malloc(sizeof(Point));
			tmp->pointX = tab_point[0];
			tmp->pointY = tab_point[1];
			tmp->rank = tab_point[2];
			insertion_noeud(point,mon_espace,tmp);
			free(tmp);
			//("Mon espace apres insertion A(%d,%d), B(%d,%d) rank : %d \n", mon_espace->aX, mon_espace->aY, mon_espace->bX,mon_espace->bY, point->rank);
		}else if(status.MPI_TAG == SWCH_V){
			int vois;
			printf("Je suis %d et je remplace un voisin par un autre \n", point->rank);
			MPI_Recv(tab_espace,6,MPI_INT,MPI_ANY_SOURCE,SWCH_V,MPI_COMM_WORLD,&status);
			Espace *e = malloc(sizeof(Espace));
			e->aX = tab_espace[AX];
			e->aY = tab_espace[AY];
			e->bY = tab_espace[BY];
			e->bX = tab_espace[BX];
			e->rank = tab_espace[RANK];
			voisinSud=rcv_switch_voisin_bis(status.MPI_SOURCE,e, voisinSud,mon_espace,SUD);	
			voisinNord=rcv_switch_voisin_bis(status.MPI_SOURCE,e, voisinNord,mon_espace,NORD);
			voisinOuest=rcv_switch_voisin_bis(status.MPI_SOURCE,e, voisinOuest,mon_espace,OUEST);
			voisinEst=rcv_switch_voisin_bis(status.MPI_SOURCE,e, voisinEst,mon_espace,EST);
		}else if(status.MPI_TAG == N_VOIS){
			printf("Un de mes voisin a un nouveau voisin donc je l'ajoute\n");
			MPI_Recv(tab_espace,6,MPI_INT,MPI_ANY_SOURCE,N_VOIS,MPI_COMM_WORLD,&status);
			printf("J'envoie a %d une demande de MAJ de son espace \n", status.MPI_SOURCE);
			MPI_Send(1,0,MPI_INT,status.MPI_SOURCE,MAJ,MPI_COMM_WORLD);
			Espace *espace_tmp = malloc(sizeof(Espace));
			espace_tmp->aX = tab_espace[AX];
			espace_tmp->aY= tab_espace[AY];
			espace_tmp->bX = tab_espace[BX];
			espace_tmp->bY = tab_espace[BY];
			espace_tmp->rank = tab_espace[RANK];
			printf("Nouveau voisin a ajoute dont les coord sont A(%d,%d) , B(%d,%d) proc %d \n", espace_tmp->aX,espace_tmp->aY,espace_tmp->bX,espace_tmp->bY,espace_tmp->rank);
			int cote; 
			if( (cote=estMonVoisin(mon_espace,espace_tmp)) > 1){
				if(cote ==  NORD){
					voisinNord= ajouterVoisin(voisinNord,espace_tmp,mon_espace);
				}else if(cote == SUD){
					voisinSud=ajouterVoisin(voisinSud,espace_tmp,mon_espace);
				}else if(cote == EST){
					voisinEst=ajouterVoisin(voisinEst,espace_tmp,mon_espace);
				}else if(cote == OUEST){
					voisinOuest=ajouterVoisin(voisinOuest,espace_tmp,mon_espace);
				}else{
					printf("Probleme ajout voisin \n");
				}
				send_ajouter_voisin(mon_espace,espace_tmp->rank);
			}else if(cote == 0){
				if(espace_tmp->aY >= mon_espace->bY){
					printf("Je propage en nord\n");
					send_nouveau_voisin(espace_tmp,voisinNord);
				}
				if(espace_tmp->bY <= mon_espace->aY){
					printf("Je propage en sud\n");
					send_nouveau_voisin(espace_tmp,voisinSud);
				}
				if(espace_tmp->aX >= mon_espace->bX){
					printf("Je propage en est\n");
					send_nouveau_voisin(espace_tmp,voisinEst);
				}
				if(espace_tmp->bX <= mon_espace->aX){
					printf("Je propage en ouest\n");
					send_nouveau_voisin(espace_tmp,voisinOuest);
				}/*else{
					printf("Probleme \n");
				}*/
			}else{
				printf(" Cote egale %d donc soucis \n", cote);
			}
		}else if(status.MPI_TAG == ADDED){
			Espace *espace_tmp = malloc(sizeof(Espace));
			printf("Je dois ajouter un voisin car on ma ajoute \n");
			MPI_Recv(tab_espace,5,MPI_INT,MPI_ANY_SOURCE,ADDED,MPI_COMM_WORLD,&status);
			printf("Le proc a ajoute est a(%d,%d) b(%d,%d) rank %d \n",tab_espace[AX],tab_espace[AY],tab_espace[BX],tab_espace[BY], status.MPI_SOURCE);
			espace_tmp->aX = tab_espace[AX];
			espace_tmp->aY = tab_espace[AY];
			espace_tmp->bX = tab_espace[BX];
			espace_tmp->bY = tab_espace[BY];
			espace_tmp->rank = status.MPI_SOURCE;
			int cote;
			if( (cote=estMonVoisin(mon_espace,espace_tmp)) > 1){
				if(cote ==  NORD){
					voisinNord= ajouterVoisin(voisinNord,espace_tmp,mon_espace);
				}else if(cote == SUD){
					voisinSud=ajouterVoisin(voisinSud,espace_tmp,mon_espace);
				}else if(cote == EST){
					voisinEst=ajouterVoisin(voisinEst,espace_tmp,mon_espace);
				}else if(cote == OUEST){
					voisinOuest=ajouterVoisin(voisinOuest,espace_tmp,mon_espace);
				}else{
					printf("Probleme ajout voisin \n");
				}
			}else if(cote == 0){
				printf("Pas mon voisin \n");
				if(espace_tmp->aY >= mon_espace->bY){
					printf("Je propage en nord\n");
					send_nouveau_voisin(espace_tmp,voisinNord);
				}
				if(espace_tmp->bY <= mon_espace->aY){
					printf("Je propage en sud\n");
					send_nouveau_voisin(espace_tmp,voisinSud);
				}
				if(espace_tmp->aX >= mon_espace->bX){
					printf("Je propage en est\n");
					send_nouveau_voisin(espace_tmp,voisinEst);
				}
				if(espace_tmp->bX <= mon_espace->aX){
					printf("Je propage en ouest\n");
					send_nouveau_voisin(espace_tmp,voisinOuest);
				}
			}else{
				printf("Cote egale probleme ADDED %d \n", cote );
			}
		}else if(status.MPI_TAG == DATA_IN || status.MPI_TAG == SEARCH){
			int tab_DHT[4];
			MPI_Recv(tab_DHT,4,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
			printf("Le proc %d me demande d'ajouter x :%d y:%d \n",status.MPI_SOURCE,tab_DHT[0],tab_DHT[1] );
			Point *tmpP = malloc(sizeof(Point));
			tmpP->pointY = tab_DHT[1];
			tmpP->pointX = tab_DHT[0];
			DHT * tmp_dht = malloc(sizeof(DHT));
			tmp_dht->valX = tab_DHT[0];
			tmp_dht->valY = tab_DHT[1];
			tmp_dht->valeur = tab_DHT[2];
			printf("Mon espace apres insertion A(%d,%d), B(%d,%d) rank : %d \n", mon_espace->aX, mon_espace->aY, mon_espace->bX,mon_espace->bY, point->rank);
			if(est_dans_espace(mon_espace,tmpP) == 0){
				//liste_dht=ajouterData(liste_dht,tmp_dht);
				printf("Je reponds aux coordinateur \n");
				ListeData *tmp_liste_dht = liste_dht;
				if(status.MPI_TAG == DATA_IN){
						printf("J'ajoute la data dans ma liste de dht \n");
						liste_dht=ajouterData(liste_dht,tmp_dht);
						tmp_liste_dht = liste_dht;
				}else{
					printf(" Je meurt ici \n");
					while(tmp_liste_dht != NULL && (tmp_liste_dht->data->valX != tmp_dht->valX && tmp_liste_dht->data->valY != tmp_dht->valY)){
						tmp_liste_dht = tmp_liste_dht->next;
					}
				}
				if(tmp_liste_dht == NULL){
					printf("Pas de donnees dans ma liste de DHT \n");
					tab_DHT[2]=-1;
					MPI_Send(tab_DHT,4,MPI_INT,0,status.MPI_TAG,MPI_COMM_WORLD);
				}else{
					printf(" Je meurt peut etre la \n");
					tab_DHT[0]=tmp_liste_dht->data->valX;
					tab_DHT[1]=tmp_liste_dht->data->valY;
					tab_DHT[2]=tmp_liste_dht->data->valeur;
					printf("Data dans mon espace \n");
					MPI_Send(tab_DHT,4,MPI_INT,0,status.MPI_TAG,MPI_COMM_WORLD);
				}
			}else{
				Liste *tmp_voisin = NULL;
				int tour=1;
				int aleatoire;
				while(tour == 1){
					aleatoire = rand()%2;
					if(tmp_dht->valX < mon_espace->aX && aleatoire == 0){
						printf("Je propage la recherche de dht a l'ouest \n");
						tmp_voisin = voisinOuest;
						tour = 0;
					}else if (tmp_dht->valX > mon_espace->bX && aleatoire == 0){
						printf("Je propage la recherche de dht a l'est \n");
						tmp_voisin=voisinEst;
						tour = 0;
					}else if(tmp_dht->valY < mon_espace->aY && aleatoire == 1){
						printf("Je propage la recherche de dht au sud \n");
						tmp_voisin = voisinSud;
						tour = 0;
					}else if(tmp_dht->valY > mon_espace->bY && aleatoire == 1){
						printf("Je propage la recherche de dht au nord \n");
						tmp_voisin = voisinNord;
						tour = 0;
					}
					if(tour == 0){
						if(tmp_voisin != NULL){
							while( tmp_voisin != NULL){
								MPI_Send(tab_DHT,4,MPI_INT,tmp_voisin->data->rank,status.MPI_TAG,MPI_COMM_WORLD);
								tmp_voisin = tmp_voisin->next;
							}
						}else{
							printf("Pas de voisin donc je reenvoie au bootsrap \n");
							MPI_Send(tab_DHT,4,MPI_INT,BOOTS,status.MPI_TAG,MPI_COMM_WORLD);
						}
					}
				}
				printf("Fin envoie data\n");
			}
			free(tmpP);
		}else if(status.MPI_TAG == DELE){
			int rank;
			MPI_Recv(&rank,1,MPI_INT,MPI_ANY_SOURCE,DELE,MPI_COMM_WORLD,&status);
			printf("Le vois %d vient de me supprimer de sa liste donc je fais de meme \n",rank);
			voisinSud=suppressionVoisin(voisinSud,rank, point->rank);
			voisinEst=suppressionVoisin(voisinEst,rank,point->rank);
			voisinNord=suppressionVoisin(voisinNord,rank,point->rank);
			voisinOuest=suppressionVoisin(voisinOuest,rank,point->rank);
		}else if(status.MPI_TAG == MAJ){
			int dem;
			MPI_Recv(&dem,1,MPI_INT,MPI_ANY_SOURCE,MAJ,MPI_COMM_WORLD,&status);
			printf("J'ai recu une demande de MAJ de l'espace j'envoie mon espace a au voisin %d \n", status.MPI_SOURCE);
			tab_espace[AX]=mon_espace->aX;
			tab_espace[AY]=mon_espace->aY;
			tab_espace[BX]=mon_espace->bX;
			tab_espace[BY]=mon_espace->bY;
			MPI_Send(tab_espace,5,MPI_INT,status.MPI_SOURCE,SYNC,MPI_COMM_WORLD);
		}else if(status.MPI_TAG == SYNC){
			MPI_Recv(tab_espace,5,MPI_INT,MPI_ANY_SOURCE,SYNC,MPI_COMM_WORLD,&status);
			voisinEst=updateEspaceVoisin(voisinEst,tab_espace,status.MPI_SOURCE,mon_espace);
			voisinNord=updateEspaceVoisin(voisinNord,tab_espace,status.MPI_SOURCE,mon_espace);
			voisinSud=updateEspaceVoisin(voisinSud,tab_espace,status.MPI_SOURCE,mon_espace);
			voisinOuest=updateEspaceVoisin(voisinOuest,tab_espace,status.MPI_SOURCE,mon_espace);
		}
		printf("Noeud;%d;%d;%d;%d;%d;%d;%d;\n",point->rank,point->pointX,point->pointY,mon_espace->aX,mon_espace->aY,mon_espace->bX,mon_espace->bY);
		afficheVoisin(point,voisinEst,"Est");
		afficheVoisin(point,voisinOuest,"Ouest");
		afficheVoisin(point,voisinNord,"Nord");
		afficheVoisin(point,voisinSud,"Sud");
		printf("Fin affichage \n");
	}
}

int main(int argc, char** argv){
	int rank;
	int nb_proc;
	int tab_point[3];
	MPI_Status status;
	MPI_Init(&argc, &argv);
	int file;
	int out;
	char nomFic[15];
	out = dup(1);
	close(1);
	MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	sprintf(nomFic,"proc_%d.txt",rank);	
	file=fopen(nomFic,"w");
	srand(getpid());
	if(rank == 0){
		ListeData *tab_data,*tmp_data;
		int tab_DHT[4];
		coordinateur();
		printf("coordinateur \n");
		tab_data = NULL;
		tab_data=insertion_valeur(tab_data,1);
		int i;
		tmp_data = tab_data;
		while(tmp_data != NULL){
			printf("Je recherche  x:%d,y:%d, valeur %d \n", tmp_data->data->valX,tmp_data->data->valY,tmp_data->data->valeur);
			tmp_data = tmp_data->next;
		}
		for(i = 0; i < NBDEM; i++){
			MPI_Recv(tab_DHT,4,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
			if(status.MPI_TAG == DATA_IN){
				printf("Le proc %d me dis qu'il a ajouter la donnees x:%d y:%d total:%d \n",status.MPI_SOURCE,tab_DHT[0],tab_DHT[1],tab_DHT[2]);
			}else{
				printf("Le proc %d me dis qu'il a trouve ou non la donnees que je cherche\n",status.MPI_SOURCE);
			}		
		}
		tmp_data = tab_data;
		//sleep(6);
		while(tmp_data != NULL){
			tab_DHT[0]=tmp_data->data->valX;
			tab_DHT[1]=tmp_data->data->valY;
			tab_DHT[2]=tmp_data->data->valeur;
			printf("J'envoi une demande de recherche pour la valeur x %d y %d \n", tab_DHT[0],tab_DHT[1]);
			MPI_Send(tab_DHT,4,MPI_INT,BOOTS,SEARCH,MPI_COMM_WORLD);
			tmp_data = tmp_data->next;
		}
		/*
		for(i = 0; i < 3; i++){
			tab_DHT[0]=rand()%(1000+1);
			tab_DHT[1]=rand()%(1000+1);
			tab_DHT[2]=tab_DHT[1]+tab_DHT[0];
			printf("Envoie de valeur aleatoire  x:%d y:%d \n", tab_DHT[0],tab_DHT[1] );
			MPI_Send(tab_DHT,4,MPI_INT,BOOTS,SEARCH,MPI_COMM_WORLD);
		}
		*/
		while(1){
			MPI_Recv(tab_DHT,4,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
			if(status.MPI_TAG == DATA_IN){
				printf("Le proc %d me dis qu'il a ajouter la donnees x:%d y:%d total:%d \n",status.MPI_SOURCE,tab_DHT[0],tab_DHT[1],tab_DHT[2]);
			}else{
				if(tab_DHT[2] == -1){
					printf("Le proc %d me dis  la donnees que je cherche n'est pas chez lui x:%d y:%d total:%d \n",status.MPI_SOURCE,tab_DHT[0],tab_DHT[1],tab_DHT[2]);
				}else{
					printf("Le proc %d me dis qu'il a trouve la donnees x:%d y:%d total:%d \n",status.MPI_SOURCE,tab_DHT[0],tab_DHT[1],tab_DHT[2]);
				}	
			}		
		}
		printf("coordinateur a fini de taffer \n");
	}else{
		Point *point = malloc(sizeof(Point));
		point->rank = rank;
		Espace *mon_espace = malloc(sizeof(Espace));
		mon_espace->rank = rank;
		voisinSud = NULL;
		voisinOuest = NULL;
		voisinNord = NULL;
		voisinSud = NULL;
		if(rank == BOOTS){
			point->pointX = rand()%(1000+1);
			point->pointY = rand()%(1000+1);
			mon_espace->aY = 0;
			mon_espace->aX = 0;
			mon_espace->bY = 1000;
			mon_espace->bX = 1000;
			printf("Je suis le bootsrap, coord(%d,%d) , rank : %d , Mon espace A(%d,%d), B(%d,%d) \n", point->pointX, point->pointY, point->rank,mon_espace->aX, mon_espace->aY, mon_espace->bX,mon_espace->bY);
			boucle_proc(point,mon_espace);
		}else{
			int tab_espace[6];
			//sleep(5);
			point->pointX = rand()%(1000+1);
			point->pointY = rand()%(1000+1);
			tab_point[0] = point->pointX;
			tab_point[1] = point->pointY;
			tab_point[2] = rank;
			point->rank = rank;
			//Envoie demande d'insertion au bootstrap
			MPI_Send(tab_point,3,MPI_INT,BOOTS,INSERT,MPI_COMM_WORLD);
			//Reception de l'espace
			printf("J'ai recu mon espace\n");
			MPI_Recv(tab_espace,5,MPI_INT,MPI_ANY_SOURCE,FIRST,MPI_COMM_WORLD,&status);
			mon_espace->aX = tab_espace[AX];
			mon_espace->aY = tab_espace[AY];
			mon_espace->bX = tab_espace[BX];
			mon_espace->bY = tab_espace[BY];
			mon_espace->rank = rank;
			printf("Mon espace A(%d,%d), B(%d,%d) mon rank : %d ,mes coords : (%d,%d) de source %d \n", mon_espace->aX, mon_espace->aY, mon_espace->bX,mon_espace->bY, point->rank,point->pointX,point->pointY,status.MPI_SOURCE);
			//Test pour savoir si l'espace affecter contient le point si non on change coordonne du point
			while(est_dans_espace(mon_espace,point) != 0){
				point->pointX = rand()%(mon_espace->bX);
				point->pointY = rand()%(mon_espace->bY);
				printf("Nouveau coord Point X: %d , point Y : %d, rank : %d \n",point->pointX, point->pointY, point->rank );
			}
			printf("Firt voisin pour rank %d \n", rank);
			boucle_proc(point,mon_espace);
		}
	}
	close(nomFic);
	dup2(out,1);
	close(out);
	printf("Fin prog pour rank %d \n", rank);
	MPI_Finalize();
}