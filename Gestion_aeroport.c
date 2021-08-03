/******************************************************************************/
/*LES HEADERS*/
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <MlvTypeC.h>        
#include <MlvProcC.h>  
/******************************************************************************/
/*LA COMMANDE NUMÉRIQUE PAR DÉFAUT DU PROGRAMME*/
/******************************************************************************/
#define NPointAvion 17
#define Nb_info 23 /*le maximun necessaire*/
#define W 1000
#define H 600
#define Nb_ZoneClic 34 /*le maximun necessaire*/
#define Nb_rangement 9 /*le maximun necessaire*/
#define MinutesParTour 4 
#define DELAI 5
/******************************************************************************/
/*LA MISE EN OEUVRE DU PROGRAMME*/
/******************************************************************************/
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
/*CHAPITRE I : LES STRUCTURES*/
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

/*************************LES STRUCTURES DE GEOMETRIE**************************/

typedef struct Couple {
	float 		x;
	float 		y;
	} couple;

typedef struct Quadruplet {
	float 		Gx;
	float 		By;
	float 		Dx;
	float 		Hy;
	} rectangle; 

/***********************LA STRUCTURE DE GESTION DU TEMPS***********************/

typedef struct horloge {
	int 		Heure;
	int 		Minutes;
	} Horloge;

/****************************LA STRUCTURE DU DECORS****************************/

typedef struct Decors {
	int 		taille;
	rectangle 	cadre[Nb_ZoneClic];
	char 		*encre[Nb_ZoneClic];
	char 		*color[Nb_ZoneClic];
	char 		*font[Nb_ZoneClic];
	char 		*nom[Nb_ZoneClic];
	couple 		centrage[Nb_ZoneClic];
	} decors;

/*********LA STRUCTURE PERMETTANT D'ORDONNER LES AVIONS A L'AFFICHAGE**********/

typedef struct Rangement {
	int etat;
	couple place;
	} Parking;

/**************LA STRUCTURE DES AVIONS MODIFIEE POUR L'AFFICHAGE***************/


typedef struct avion {
	char 	identifiant[7]; 		/*6*[char] +['\0']*/
	int 	carburant;
	int 	consommation;
	char 	heure_decollage[5]; 		/*4*[char] +['\0']*/	
	int 	mode;
	couple 	centre;
	couple 	axe;
	char 	info[Nb_info];
	XPoint 	forme[NPointAvion];
	XPoint 	ombre[NPointAvion];
	float 	grd;
	float 	elgt;
	struct compagnie *compagnie;
	} Avion;

/**********************STRUCTURES DE GESTION DU TRAFIC*************************/

typedef struct compagnie {
	char 				*nom;
	char 				acronyme[4]; 	/*3*[char] +['\0']*/
	struct cellule_avion 		*avions_compagnie; 
	} Compagnie;

typedef struct cellule_compagnie {
	Compagnie 			comp;
	struct cellule_compagnie 	*suivant;
	} Cellule_compagnie;

typedef Cellule_compagnie 		*Liste_compagnie;

typedef struct cellule_avion {
	Avion avion;
	struct cellule_avion 		*suivant_compagnie;
	struct cellule_avion 		*precedent_compagnie;
	struct cellule_avion 		*suivant_attente;
	} Cellule_avion;

typedef Cellule_avion 			*Liste_avions;

typedef struct queue {
	Liste_avions 			premier;
	Liste_avions 			dernier;
	} Queue;

/******************************************************************************/
	
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
/*FIN DU CHAPITRE I*/
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

/*============================================================================*/
/*COMMENTAIRE:
dans la definition de Avion, le champ "mode" correspond:
à l'etat D piste 1 pour la valeur 0,
à l'etat D piste 2 pour la valeur 1,
à l'etat A piste 1 pour la valeur 2,
à l'etat A piste 2 pour la valeur 3,
à l'etat U piste 1 pour la valeur 4,
à l'etat U piste 2 pour la valeur 5,
à l'etat N pour la valeur 6,
à l'etat I pour la valeur 7.*/
/*============================================================================*/

/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
/*CHAPITRE II : LES FONCTIONS*/
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

/******************************************************************************/
/*LES OPERATEURS GRAPHIQUES*/
/******************************************************************************/
couple plusvect(couple a, couple b) {
	float x = a.x + b.x;
	float y = a.y + b.y;
	couple c = {x, y};
	return c;
	}

couple moinsvect(couple a, couple b) {
	float x = a.x - b.x;
	float y = a.y - b.y;
	couple c = {x, y};
	return c;
	}
		
couple prdtvect(couple a, float s) {
	float x = a.x*s;
	float y = a.y*s;
	couple c = {x, y};
	return c;
	}

int signe(float r) {	
	if(r<0) return -1;
	return 1;
	}
	
/*ATTENTION : les rotations se font dans le sens indirect (gravité inversée)*/
couple rotvect(couple a, double theta) {
	float x = a.x*cos(theta)-a.y*sin(theta);
	float y = a.x*sin(theta)+a.y*cos(theta);
	couple c = {x, y};
	return c;  
	}

couple prdtcomplex(couple a, couple b) {
	float x = a.x*b.x+a.y*b.y;
	float y = a.x*b.y-a.y*b.x;
	couple c = {x, y};
	return c;
	}
	
float norme2(couple a) {
	return sqrt(prdtcomplex(a,a).x);
	}
	
int dim_w(float q) {
	return (int) (q*W);
	}

int dim_h(float q) {
	return (int) (q*H);
	}
/******************************************************************************/
/*LES FONCTIONS D'AFFICHAGES ET DE SELECTIONS*/
/******************************************************************************/

/*****************************Valide_Clic**************************************/

int Valide_Clic(float *X_mouse, float *Y_mouse, rectangle *Cadre,int Nbr, MlvStruct *x_var) {
	int i;
	int X, Y;
	mouse_wait(&X,&Y,x_var);
	*X_mouse=X;
	*Y_mouse=Y;
	for(i=1;i<=Nbr;i++)
	if (*X_mouse >= Cadre[i-1].Gx && *X_mouse <= Cadre[i-1].Dx 
	&&  *Y_mouse >= Cadre[i-1].By && *Y_mouse <= Cadre[i-1].Hy) 
	return i;
	return -1;
	}

/******************************Affiche_Avion***********************************/

void Affiche_Avion(Avion *Ptra, couple *Graph, char *TabColor[], MlvStruct *x_var) {
	int i;
	couple normale, decalage, projection = {(Ptra->grd-0.5)*5,(Ptra->grd-0.5)*3.5};
	for(i=0; i<NPointAvion;i++) {
	/*Calcul de la forme de l'avion*/	
	normale = prdtvect(rotvect(Ptra->axe, Graph[i].y), (Ptra->grd*Graph[i].x)/norme2(Ptra->axe));
	Ptra->forme[i].x = (int short)(plusvect(Ptra->centre,normale).x);
	Ptra->forme[i].y = (int short)(plusvect(Ptra->centre,normale).y);
	/*Calcul similaire pour l'ombre de l'avion*/	
	decalage =plusvect(Ptra->centre,projection);
	normale = prdtvect(rotvect(Ptra->axe, Graph[i].y), (Ptra->elgt*Ptra->grd*Graph[i].x)/norme2(Ptra->axe));
	Ptra->ombre[i].x = (int short)(plusvect(decalage,normale).x);
	Ptra->ombre[i].y = (int short)(plusvect(decalage,normale).y);	
		}
	/*Affichage*/
	int retour=12;
	char affiche1[Nb_info-1-retour]; /*car info possede un retour chariot a la fin*/
	char affiche2[retour+1];
	strncpy(affiche1,Ptra->info,Nb_info-2-retour);
	strncpy(affiche2,Nb_info-2-retour+Ptra->info,retour);
	affiche1[Nb_info-2-retour]='\0';
	affiche2[retour]='\0';
	draw_filled_polygone(Ptra->ombre, NPointAvion, "dark slate gray", x_var);
	draw_filled_polygone(Ptra->forme, NPointAvion, TabColor[Ptra->mode], x_var);
	draw_string(Ptra->centre.x-50,Ptra->centre.y-20,affiche1,"5x8","gold",x_var);
	draw_string(Ptra->centre.x-50,Ptra->centre.y-10,affiche2,"5x8","gold",x_var);
	/*display_window(x_var);*/
	}

/********************************Efface_Avion**********************************/

void Efface_Avion(Avion *Ptra, couple *Graph, char *TabColor[], MlvStruct *x_var) {
	char affiche[Nb_info-1]; /*car info possede un retour chariot a la fin*/
	strcpy(affiche,Ptra->info);
	affiche[Nb_info-2]='\0';
	draw_filled_polygone(Ptra->ombre, NPointAvion, "LightSlateGrey", x_var);
	draw_filled_polygone(Ptra->forme, NPointAvion, "LightSlateGrey", x_var);
	draw_string(Ptra->centre.x-10,Ptra->centre.y-20,affiche,"6x10","LightSlateGrey",x_var);
	}

/*******************************Affiche_Decors*********************************/

void Affiche_Decors(decors Dec, MlvStruct *x_var) {
	int i;
	for(i=0;i<Dec.taille;i++) {
	draw_filled_rectangle ((Dec.cadre[i].Gx), (Dec.cadre[i].By), (Dec.cadre[i].Dx-Dec.cadre[i].Gx),(Dec.cadre[i].Hy-Dec.cadre[i].By), Dec.color[i],x_var);
	draw_string(Dec.cadre[i].Gx+Dec.centrage[i].x, Dec.cadre[i].Hy-Dec.centrage[i].y, Dec.nom[i], Dec.font[i], Dec.encre[i],x_var);
	}
	}

/**********************Affiche_Tous_Les_Avions*********************************/

void Affiche_Tous_Les_Avions(Liste_avions La, int Nb_Max, couple *Graph, char *TabColor[], MlvStruct *x_var) {
	int i;
	for(i=0;La!=NULL && i<Nb_Max;La=La->suivant_attente)
	Affiche_Avion(&La->avion, Graph, TabColor, x_var);
	}

/*******************************Cinematique_Avion******************************/

void Cinematique_Avion(Avion *Ptra, int app, Liste_avions La, int Nb_Max, couple *Graph, char *TabColor[], couple direction, decors Dec, MlvStruct *x_var, int lim, ...) { /*lim=1 ou 0 */
	float Pi = 3.141592;
	float s =1/(norme2(Ptra->axe)*norme2(moinsvect(direction,Ptra->centre)));
	couple Trigo = prdtvect(prdtcomplex(Ptra->axe,moinsvect(direction,Ptra->centre)),s);
	float angle = signe(Trigo.y)*acos(Trigo.x), iangle;
	float th = signe(Trigo.y)*(0.3*Pi)/180;
	float t=1, d=1;
	
	va_list ArgTmp;
	Liste_avions Arg_Liste;
	va_start(ArgTmp, lim);
	Arg_Liste=va_arg(ArgTmp,Liste_avions);
	
	/*Ajustement de l'axe de l'avion sur la direction indiqué par*/
	/*l'utilisateur a une erreur de 0.01 radians*/
	for(iangle=0;fabs(angle-iangle)>0.01;iangle+=th) {
	Ptra->axe=rotvect(Ptra->axe,th);
	Affiche_Decors(Dec, x_var);
	if (lim==1) Affiche_Tous_Les_Avions(Arg_Liste, Nb_Max, Graph, TabColor, x_var);
	Affiche_Tous_Les_Avions(La, Nb_Max, Graph, TabColor, x_var);
	if (!app) Affiche_Avion(Ptra, Graph, TabColor,x_var);
	display_window(x_var);
	wait_micros(t);
	/*Efface_Avion(Ptra, Graph, TabColor,x_var);*/
		}
	/*Ajustement effectif de l'axe*/
	Ptra->axe=moinsvect(direction,Ptra->centre);
	/*Avancement de l'avion vers la cible indiqué par l'utilisateur*/
	/*a une erreur de 0.8 pixels*/
	for(;norme2(moinsvect(Ptra->centre,direction))>0.8; Ptra->centre=plusvect(Ptra->centre,prdtvect(Ptra->axe,d/norme2(Ptra->axe)))) {	
	Affiche_Decors(Dec, x_var);
	if (lim==1) Affiche_Tous_Les_Avions(Arg_Liste, Nb_Max, Graph, TabColor, x_var);
	Affiche_Tous_Les_Avions(La, Nb_Max, Graph, TabColor, x_var);
	if (!app) Affiche_Avion(Ptra, Graph, TabColor,x_var);
	display_window(x_var);
	wait_micros(t);
	/*Efface_Avion(Ptra, Graph, TabColor,x_var);*/	
		}

	va_end(ArgTmp);
	}

/*****************************Init_Graph_Avion*********************************/

void Init_Graph_Avion(Avion *Ptra, int xd, int yd, int xc, int yc, int mode) {
	Ptra->centre.x=xc;
	Ptra->centre.y=yc;
	Ptra->axe.x=xd;
	Ptra->axe.y=yd;
	Ptra->mode = mode;
	Ptra->grd = sqrt(sqrt(((float)(H))*((float)(W))/650000));
	Ptra->elgt =0.95;
	Ptra->info[Nb_info-1]='\0'; /* on n'est jamais trop prudent*/
	}
	
/**************************Completion_Numérique********************************/	

void Completion_Numerique(int nbr,int taille, char *t) {		
		int i,j;
		for(j=0,i=nbr;i!=0;j++,i=i/10);
		for(i=0;i!=taille-j;i++) t[i]='0';
		}

/******************************************************************************/
	
/******************************************************************************/
/*LES FONCTIONS DE COHESION TEMPORELLE*/
/******************************************************************************/

/****************************Determine_Retard**********************************/

int Determine_Retard(Horloge Hor, char Heure_Dec[]) {
	if (Hor.Heure < 10*(Heure_Dec[0]-'0')+(Heure_Dec[1]-'0') ) return 1;  /*à l'heure*/
	if (Hor.Heure > 10*(Heure_Dec[0]-'0')+(Heure_Dec[1]-'0') ) return -1; /*en retard*/
	if (Hor.Heure == 10*(Heure_Dec[0]-'0')+(Heure_Dec[1]-'0') ) {
		if (Hor.Minutes < 10*(Heure_Dec[2]-'0')+(Heure_Dec[3]-'0') ) return 1; /*à l'heure*/
		if (Hor.Minutes > 10*(Heure_Dec[2]-'0')+(Heure_Dec[3]-'0') ) return -1; /*en retard*/;
		}
		return -1;/*sinon en retard*/
		}

/******************************Mise_A_Jour*************************************/

void Mise_A_Jour(Horloge *Hor, int sec) {
	if (Hor->Minutes+sec==(Hor->Minutes+sec)%60) {
	Hor->Minutes=Hor->Minutes+sec;
	Hor->Heure=Hor->Heure;
	}
	else {
	Hor->Minutes=(Hor->Minutes+sec)%60;
	Hor->Heure=(Hor->Heure+1)%24;
	}
	}

/******************************************************************************/	
	
/******************************************************************************/
/*LES FONCTIONS SERVANT A LA SAISIE DES DONNEES*/
/******************************************************************************/

/******************************Choisir_nbr*************************************/

/*une fonction de saisie de nombres modulo Max*/

void Choisir_nbr(int hauteur,char *Question, int *Ptr, int Min, int Modulo, int taille, MlvType *X_var) {
	int marge=20,x,y,lq;
	draw_string(marge,hauteur,Question,"10x20","black", X_var);
	lq=strlen(Question);
	marge +=10*(lq+1);
	
	char *ph1= "parcourez la liste en";
	int lph1=strlen(ph1);
	draw_string(marge,hauteur,ph1,"10x20","black", X_var);
	int bordG1 = marge +=10*(lph1+1);
	
	char *ph2= "descendant";
	int lph2=strlen(ph2);
	draw_filled_rectangle(marge,hauteur-20+5,10*lph2,20,"gray",X_var);
	draw_string(marge,hauteur,ph2,"10x20","black",X_var);
	int bordD1 = marge +=10*(lph2+1);
		
	char *ph3= "ou en";
	int lph3=strlen(ph3);
	draw_string(marge,hauteur,ph3,"10x20","black", X_var);
	int bordG2 = marge +=10*(lph3+1);
	
	char *ph4= "remontant";
	int lph4=strlen(ph4);
	draw_filled_rectangle(marge,hauteur-20+5,10*lph4,20,"gray",X_var);
	draw_string(marge,hauteur,ph4,"10x20","black",X_var);
	int bordD2 = marge +=10*(lph4+1);
	marge =20;
		
	draw_rectangle(marge-5,hauteur+40-5,535+5,30+5,"black", X_var);
	draw_filled_rectangle(marge,hauteur+40,535,30,"white",X_var);
	
	char *ph5= "selectionner";
	int lph5=strlen(ph5);
	draw_filled_rectangle(marge+550,hauteur+40+5,10*lph5,20,"gray",X_var);
	draw_string(marge+550,hauteur+60,ph5,"10x20","black",X_var);
	
	char *Tab;
	Tab=(char*)malloc((taille+1)*sizeof(char)); /* +1 pour le '\0'*/
	if (Tab==NULL) {
	fprintf(stderr,"Erreur d'allocation dynamiquement de memoire\n");
	exit(1);
	}
	Tab[taille] ='\0';
		
	char format[4]="%-d";  /* "4 = 3 + 1" pour le '\0' */
	format[1]='0'+taille;
	*Ptr=Min;
	
	draw_string(marge,hauteur+60,"Choisir","10x20","black", X_var);
	display_window(X_var);
	do {
	mouse_wait(&x,&y,X_var);
		if (x> bordG1 && x<bordD1 && y>hauteur-20+5+2 && y<hauteur+5+2) {
			draw_filled_rectangle(marge,hauteur+40,535,30,"white",X_var);
			break;
			}
		if (x> bordG2 && x<bordD2 && y>hauteur-20+5+2 && y<hauteur+5+2) {
			draw_filled_rectangle(marge,hauteur+40,535,30,"white",X_var);
			break;
			}
		} while (1);
	
	
	while(1) {
		
		sprintf(Tab,format, *Ptr);
		Completion_Numerique(*Ptr,taille,Tab);
		draw_string(marge,hauteur+60,Tab,"10x20","black", X_var);
		display_window(X_var);
	do {
	mouse_wait(&x,&y,X_var);
		if (x> bordG1 && x<bordD1 && y>hauteur-20+5+2 && y<hauteur+5+2) {
			draw_filled_rectangle(marge,hauteur+40,535,30,"white",X_var);
			(*Ptr)++;
			if ((*Ptr=((*Ptr)%Modulo))<Min) *Ptr=Min;
			break;
			}
		if (x> bordG2 && x<bordD2 && y>hauteur-20+5+2 && y<hauteur+5+2) {
			draw_filled_rectangle(marge,hauteur+40,535,30,"white",X_var);
			(*Ptr)--;
			if ((*Ptr=((*Ptr)%Modulo))<Min) *Ptr=Modulo-1;
			break;
			}
		if (x>20+550 && x<20+550+10*lph5 && y>hauteur+40+5+2 && y<hauteur+60+5+2) return;
		
		} while (1);
	}
	free(Tab);
}

/******************************Choisir_Comp************************************/

/*Ici on choisit des compagnies déjà existantes*/

Liste_compagnie Choisir_Comp(int hauteur,char *Question, Liste_compagnie Lc, MlvType *X_var) {
	int marge=20,x,y,lq;
	draw_string(marge,hauteur,Question,"10x20","black", X_var);
	lq=strlen(Question);
	marge +=10*(lq+1);
	
	char *ph1= "parcourez la liste en";
	int lph1=strlen(ph1);
	draw_string(marge,hauteur,ph1,"10x20","black", X_var);
	int bordG1 = marge +=10*(lph1+1);
	
	char *ph2= "descendant";
	int lph2=strlen(ph2);
	draw_filled_rectangle(marge,hauteur-20+5,10*lph2,20,"gray",X_var);
	draw_string(marge,hauteur,ph2,"10x20","black",X_var);
	int bordD1 = marge +=10*(lph2+1);
		
	char *ph3= "ou";
	int lph3=strlen(ph3);
	draw_string(marge,hauteur,ph3,"10x20","black", X_var);
	int bordG2 = marge +=10*(lph3+1);
	
	char *ph4= "revenez au debut";
	int lph4=strlen(ph4);
	draw_filled_rectangle(marge,hauteur-20+5,10*lph4,20,"gray",X_var);
	draw_string(marge,hauteur,ph4,"10x20","black",X_var);
	int bordD2 = marge +=10*(lph4+1);
	marge =20;
		
	draw_rectangle(marge-5,hauteur+40-5,535+5,30+5,"black", X_var);
	
	char *ph5= "selectionner";
	int lph5=strlen(ph5);
	draw_filled_rectangle(marge+550,hauteur+40+5,10*lph5,20,"gray",X_var);
	draw_string(marge+550,hauteur+60,ph5,"10x20","black",X_var);
	
	Liste_compagnie LcTmp=Lc;
	while(Lc!=NULL) {
		draw_string(marge,hauteur+60,Lc->comp.nom,"10x20","black", X_var);
		display_window(X_var);
	do {
	mouse_wait(&x,&y,X_var);
		if (x> bordG1 && x<bordD1 && y>hauteur-20+5+2 && y<hauteur+5+2) {
			draw_filled_rectangle(marge,hauteur+40,535,30,"white",X_var);
			Lc=Lc->suivant;
			if(Lc==NULL) Lc=LcTmp;
			break;
			}
		if (x> bordG2 && x<bordD2 && y>hauteur-20+5+2 && y<hauteur+5+2) {
			draw_filled_rectangle(marge,hauteur+40,535,30,"white",X_var);
			Lc=LcTmp;
			break;
			}
		if (x>20+550 && x<20+550+10*lph5 && y>hauteur+40+5+2 && y<hauteur+60+5+2) return Lc;
				
		} while (1);
	}
	return NULL;
}

/******************************Choisir_Avion***********************************/

/*Ici on choisit des avions déjà existants*/

Liste_avions Choisir_Avion(int hauteur,char *Question, Liste_avions La, MlvType *X_var) {
	int marge=20,x,y,lq;
	draw_string(marge,hauteur,Question,"10x20","black", X_var);
	lq=strlen(Question);
	marge +=10*(lq+1);
	
	char *ph1= "parcourez la liste en";
	int lph1=strlen(ph1);
	draw_string(marge,hauteur,ph1,"10x20","black", X_var);
	int bordG1 = marge +=10*(lph1+1);
	
	char *ph2= "descendant";
	int lph2=strlen(ph2);
	draw_filled_rectangle(marge,hauteur-20+5,10*lph2,20,"gray",X_var);
	draw_string(marge,hauteur,ph2,"10x20","black",X_var);
	int bordD1 = marge +=10*(lph2+1);
		
	char *ph3= "ou";
	int lph3=strlen(ph3);
	draw_string(marge,hauteur,ph3,"10x20","black", X_var);
	int bordG2 = marge +=10*(lph3+1);
	
	char *ph4= "revenez au debut";
	int lph4=strlen(ph4);
	draw_filled_rectangle(marge,hauteur-20+5,10*lph4,20,"gray",X_var);
	draw_string(marge,hauteur,ph4,"10x20","black",X_var);
	int bordD2 = marge +=10*(lph4+1);
	marge =20;
		
	draw_rectangle(marge-5,hauteur+40-5,535+5,30+5,"black", X_var);
	
	char *ph5= "selectionner";
	int lph5=strlen(ph5);
	draw_filled_rectangle(marge+550,hauteur+40+5,10*lph5,20,"gray",X_var);
	draw_string(marge+550,hauteur+60,ph5,"10x20","black",X_var);
	
	Liste_avions LaTmp=La;
	while(La!=NULL) {
		draw_string(marge,hauteur+60,La->avion.identifiant,"10x20","black", X_var);
		display_window(X_var);
	do {
	mouse_wait(&x,&y,X_var);
		if (x> bordG1 && x<bordD1 && y>hauteur-20+5+2 && y<hauteur+5+2) {
			draw_filled_rectangle(marge,hauteur+40,535,30,"white",X_var);
			La=La->suivant_attente;
			if(La==NULL) La=LaTmp;
			break;
			}
		if (x> bordG2 && x<bordD2 && y>hauteur-20+5+2 && y<hauteur+5+2) {
			draw_filled_rectangle(marge,hauteur+40,535,30,"white",X_var);
			La=LaTmp;
			break;
			}
		if (x>20+550 && x<20+550+10*lph5 && y>hauteur+40+5+2 && y<hauteur+60+5+2) return La;
				
		} while (1);
	}
	return NULL;
}

/******************************Choisir_Avion_ParMode***************************/

/*Ici on choisit des avions déjà existants dans Li*/

Liste_avions Choisir_Avion_ParMode(int hauteur, char *Question, Liste_avions La, int mode, MlvType *X_var) {
	int marge=20,x,y,lq;
	draw_string(marge,hauteur,Question,"10x20","black", X_var);
	lq=strlen(Question);
	marge +=10*(lq+1);
	
	char *ph1= "parcourez la liste en";
	int lph1=strlen(ph1);
	draw_string(marge,hauteur,ph1,"10x20","black", X_var);
	int bordG1 = marge +=10*(lph1+1);
	
	char *ph2= "descendant";
	int lph2=strlen(ph2);
	draw_filled_rectangle(marge,hauteur-20+5,10*lph2,20,"gray",X_var);
	draw_string(marge,hauteur,ph2,"10x20","black",X_var);
	int bordD1 = marge +=10*(lph2+1);
		
	char *ph3= "ou en";
	int lph3=strlen(ph3);
	draw_string(marge,hauteur,ph3,"10x20","black", X_var);
	int bordG2 = marge +=10*(lph3+1);
	
	char *ph4= "remontant";
	int lph4=strlen(ph4);
	draw_filled_rectangle(marge,hauteur-20+5,10*lph4,20,"gray",X_var);
	draw_string(marge,hauteur,ph4,"10x20","black",X_var);
	int bordD2 = marge +=10*(lph4+1);
	marge =20;
		
	draw_rectangle(marge-5,hauteur+40-5,535+5,30+5,"black", X_var);
	
	char *ph5= "selectionner";
	int lph5=strlen(ph5);
	draw_filled_rectangle(marge+550,hauteur+40+5,10*lph5,20,"gray",X_var);
	draw_string(marge+550,hauteur+60,ph5,"10x20","black",X_var);
	
	int sensdirect=1;
	char *sauv;
	int Choix=-1;
	
	while (La!=NULL) {
/* dans les boucles for qui suivent on n'a pas besoin de vérifier si La==NULL*/
switch(sensdirect) {
	case 1 : 
	for(;La->avion.mode!=mode; La=La->suivant_compagnie)
		if (La->suivant_compagnie==NULL) {
		sensdirect=0;
		break;
		}
	break;
	
	case 0 :
	for(;La->avion.mode!=mode; La=La->precedent_compagnie)
		if (La->precedent_compagnie==NULL) {
		sensdirect=1;
		break;
		}
	break;
	}
	
if (La->avion.mode==mode) {
	sauv=La->avion.identifiant;/*sauv car le cas NULL existe*/
	Choix=mode;
	}

switch(Choix==mode) {
	case 1 :	
		draw_string(marge,hauteur+60,sauv,"10x20","black", X_var); /*sauv car le cas NULL existe*/
		display_window(X_var);
	break;

	case 0 :
		draw_string(marge,hauteur+60,"Aucun avion","10x20","black", X_var);
		display_window(X_var);
	break;
	}
	
	do {
	mouse_wait(&x,&y,X_var);
		if (x> bordG1 && x<bordD1 && y>hauteur-20+5+2 && y<hauteur+5+2) {
			draw_filled_rectangle(marge,hauteur+40,535,30,"white",X_var);
			if (La->suivant_compagnie!=NULL) La=La->suivant_compagnie;
			break;
			}
		if (x> bordG2 && x<bordD2 && y>hauteur-20+5+2 && y<hauteur+5+2) {
			draw_filled_rectangle(marge,hauteur+40,535,30,"white",X_var);
			if (La->precedent_compagnie!=NULL) La=La->precedent_compagnie;
			break;
			}
		if (x>20+550 && x<20+550+10*lph5 && y>hauteur+40+5+2 && y<hauteur+60+5+2) return La;
				
		} while (1);
	}
	return NULL;
}

/******************************Demande_Comp************************************/

/*Ici on choisit une compagnie dans un fichier : elle n'existe donc pas encore mais on va la créer*/

void Demande_Comp(int hauteur,char *Question, FILE *Lecture, char **Nom, char *Acronyme, int Taille, MlvType *X_var) {
	char stock[25];
	char c;
	int marge=20,x,y,strlg,lq;
	draw_string(marge,hauteur,Question,"10x20","black", X_var);
	lq=strlen(Question);
	marge +=10*(lq+1);
	
	char *ph1= "parcourez la liste en";
	int lph1=strlen(ph1);
	draw_string(marge,hauteur,ph1,"10x20","black", X_var);
	int bordG1 = marge +=10*(lph1+1);
	
	char *ph2= "descendant";
	int lph2=strlen(ph2);
	draw_filled_rectangle(marge,hauteur-20+5,10*lph2,20,"gray",X_var);
	draw_string(marge,hauteur,ph2,"10x20","black",X_var);
	int bordD1 = marge +=10*(lph2+1);
		
	char *ph3= "ou en";
	int lph3=strlen(ph3);
	draw_string(marge,hauteur,ph3,"10x20","black", X_var);
	int bordG2 = marge +=10*(lph3+1);
	
	char *ph4= "remontant";
	int lph4=strlen(ph4);
	draw_filled_rectangle(marge,hauteur-20+5,10*lph4,20,"gray",X_var);
	draw_string(marge,hauteur,ph4,"10x20","black",X_var);
	int bordD2 = marge +=10*(lph4+1);
	marge =20;
		
	draw_rectangle(marge-5,hauteur+40-5,535+5,30+5,"black", X_var);
	
	char *ph5= "selectionner";
	int lph5=strlen(ph5);
	draw_filled_rectangle(marge+550,hauteur+40+5,10*lph5,20,"gray",X_var);
	draw_string(marge+550,hauteur+60,ph5,"10x20","black",X_var);
	
	fseek(Lecture, sizeof(char),SEEK_CUR);
	
	(*Nom)=NULL;
	while(1) {
		while((c=fgetc(Lecture))!='/'&& c!= EOF) {
		fseek(Lecture, -sizeof(char),SEEK_CUR);
		fscanf(Lecture,"%s ",stock);
		draw_string(marge,hauteur+60, stock,"10x20","black", X_var);
		marge+=10*(strlen(stock)+1);
			if ((*Nom)==NULL) {
			(*Nom)=(char *)malloc(strlen(stock)+1);
			if ((*Nom)==NULL) exit(1);
			strcpy((*Nom), stock);
			}
			else {
			strlg=strlen((*Nom));
			(*Nom)=(char *)realloc((*Nom) ,strlg+1+strlen(stock)+1);
			(*Nom)[strlg]=' ';
			(*Nom)[strlg+1]='\0';
			strcat((*Nom),stock);
			}
		}
	fseek(Lecture, (Taille+1+1+1)*sizeof(char),SEEK_CUR); /* "Acronyme" + ' ' +'/'+'\n' */
	display_window(X_var);
	do {
	mouse_wait(&x,&y,X_var);
		if (x> bordG1 && x<bordD1 && y>hauteur-20+5+2 && y<hauteur+5+2) {
			marge=20;
			draw_filled_rectangle(marge,hauteur+40,535,30,"white",X_var);
			break;
			}
		if (x> bordG2 && x<bordD2 && y>hauteur-20+5+2 && y<hauteur+5+2) {
			marge=20;
			draw_filled_rectangle(marge,hauteur+40,535,30,"white",X_var);
			
			fseek(Lecture, -(Taille)*sizeof(char), SEEK_CUR); /*acronyme*/
			while((c=fgetc(Lecture))!='/') fseek(Lecture, -2*sizeof(char), SEEK_CUR);
			
			fseek(Lecture, -3*sizeof(char), SEEK_CUR); /*nom de la compagnie*/
			while((c=fgetc(Lecture))!='/') fseek(Lecture, -2*sizeof(char), SEEK_CUR);
			
			fseek(Lecture, -(Taille)*sizeof(char), SEEK_CUR); /*acronyme*/
			while((c=fgetc(Lecture))!='/') fseek(Lecture, -2*sizeof(char), SEEK_CUR);
			
			fseek(Lecture, -3*sizeof(char), SEEK_CUR); /*nom de la compagnie*/
			while((c=fgetc(Lecture))!='/') fseek(Lecture, -2*sizeof(char), SEEK_CUR);
			
			fseek(Lecture, sizeof(char),SEEK_CUR);
			break;
			}
		if (x>20+550 && x<20+550+10*lph5 && y>hauteur+40+5+2 && y<hauteur+60+5+2) {
			if ((*Nom)!=NULL) {
			fseek(Lecture, -(Taille+1+1+1)*sizeof(char), SEEK_CUR); /* "Acronyme" + ' ' +'/'+'\n' */
			fscanf(Lecture,"%s ",Acronyme);
			*(Acronyme+Taille)='\0';
			}
			return;
			}
			
		} while (1);
		free(*Nom);
		(*Nom)=NULL;
	}
}

/*****************************Voir_Historique**********************************/

void Voir_Historique(int hauteur,char *Question, FILE *Lecture, MlvType *X_var) {
	int marge=20,x,y,lq;
	draw_string(marge,hauteur,Question,"10x20","black", X_var);
	lq=strlen(Question);
	marge +=10*(lq+1);
	
	char *ph1= "parcourez la liste en";
	int lph1=strlen(ph1);
	draw_string(marge,hauteur,ph1,"10x20","black", X_var);
	int bordG1 = marge +=10*(lph1+1);
	
	char *ph2= "descendant";
	int lph2=strlen(ph2);
	draw_filled_rectangle(marge,hauteur-20+5,10*lph2,20,"gray",X_var);
	draw_string(marge,hauteur,ph2,"10x20","black",X_var);
	int bordD1 = marge +=10*(lph2+1);
		
	char *ph3= "ou en";
	int lph3=strlen(ph3);
	draw_string(marge,hauteur,ph3,"10x20","black", X_var);
	int bordG2 = marge +=10*(lph3+1);
	
	char *ph4= "remontant";
	int lph4=strlen(ph4);
	draw_filled_rectangle(marge,hauteur-20+5,10*lph4,20,"gray",X_var);
	draw_string(marge,hauteur,ph4,"10x20","black",X_var);
	int bordD2 = marge +=10*(lph4+1);
	marge =20;
		
	draw_rectangle(marge-5,hauteur+40-5,535+5,30+175+5,"black", X_var);
	
	char *ph5= "Terminer";
	int lph5=strlen(ph5);
	draw_filled_rectangle(marge+550,hauteur+40+5,10*lph5,20,"gray",X_var);
	draw_string(marge+550,hauteur+60,ph5,"10x20","black",X_var);
	
	char phrase[22]; /* 21 char + '\0' */
	int N=10; /*affichage sur 10 lignes*/
	int i;
	
	fseek(Lecture,0,SEEK_END);
/*pour eviter l'affichage de caractères non compris par la bibliothèque lorsque le fichier est vide*/
	if (ftell(Lecture)==0) {
	do {
	mouse_wait(&x,&y,X_var);
		if (x>20+550 && x<20+550+10*lph5 && y>hauteur+40+5+2 && y<hauteur+60+5+2) return;
		} while (1);
	}
	
	fseek(Lecture,0,SEEK_SET);
	
	while(1) {
		for(i=0;i<N && !feof(Lecture);i++) {
		fscanf(Lecture,"%s\n",phrase);
		draw_string(marge,hauteur+60+20*i,phrase,"10x20","black", X_var);
		}
	display_window(X_var);
	
	do {
	mouse_wait(&x,&y,X_var);
		if (x> bordG1 && x<bordD1 && y>hauteur-20+5+2 && y<hauteur+5+2 && !feof(Lecture)) {
			draw_filled_rectangle(20,hauteur+40,535,30+175,"white",X_var);
			for(i=0;i<N-1;i++) fseek(Lecture, -22*sizeof(char), SEEK_CUR);
			break;
			}
		
		if (x> bordG2 && x<bordD2 && y>hauteur-20+5+2 && y<hauteur+5+2) {
			draw_filled_rectangle(20,hauteur+40,535,30+175,"white",X_var);
			for(i=0;i<N+1;i++) fseek(Lecture, -22*sizeof(char), SEEK_CUR);
			break;
			}
		if (x>20+550 && x<20+550+10*lph5 && y>hauteur+40+5+2 && y<hauteur+60+5+2) return;
		
		} while (1);
	}
}

/*************************Voir_Etat_Avions_Compagnie***************************/

/*Affiche l'etat des avions d'une compagnie*/

void Voir_Etat_Avions_Compagnie(int hauteur,char *Question, Liste_avions La, Horloge Hor, MlvType *X_var) {
	int marge=20,x,y,lq;
	draw_string(marge,hauteur,Question,"10x20","black", X_var);
	lq=strlen(Question);
	marge +=10*(lq+1);
	
	char *ph1= "parcourez la liste en";
	int lph1=strlen(ph1);
	draw_string(marge,hauteur,ph1,"10x20","black", X_var);
	int bordG1 = marge +=10*(lph1+1);
	
	char *ph2= "descendant";
	int lph2=strlen(ph2);
	draw_filled_rectangle(marge,hauteur-20+5,10*lph2,20,"gray",X_var);
	draw_string(marge,hauteur,ph2,"10x20","black",X_var);
	int bordD1 = marge +=10*(lph2+1);
		
	char *ph3= "ou en";
	int lph3=strlen(ph3);
	draw_string(marge,hauteur,ph3,"10x20","black", X_var);
	int bordG2 = marge +=10*(lph3+1);
	
	char *ph4= "remontant";
	int lph4=strlen(ph4);
	draw_filled_rectangle(marge,hauteur-20+5,10*lph4,20,"gray",X_var);
	draw_string(marge,hauteur,ph4,"10x20","black",X_var);
	int bordD2 = marge +=10*(lph4+1);
	marge =20;
		
	draw_rectangle(marge-5,hauteur+40-5,535+5,30+5,"black", X_var);
	
	char *ph5= "Terminer";
	int lph5=strlen(ph5);
	draw_filled_rectangle(marge+550,hauteur+40+5,10*lph5,20,"gray",X_var);
	draw_string(marge+550,hauteur+60,ph5,"10x20","black",X_var);
	
	char tab[3];
	
	if(La==NULL) {
	draw_string(marge,hauteur+60,"aucun avion","10x20","black", X_var);
	display_window(X_var);
		do {
		mouse_wait(&x,&y,X_var);
		if (x>20+550 && x<20+550+10*lph5 && y>hauteur+40+5+2 && y<hauteur+60+5+2) return;		
		} while (1);
	}
	
	while(La!=NULL) {

		draw_string(marge,hauteur+60,La->avion.identifiant,"10x20","black", X_var);
		display_window(X_var);
		marge+=10*(6+1);
		
	switch(La->avion.mode) {
		
/*Ici un parcours de la File "Queue" n'était pas intéressant algorithmiquement : grosse complexité*/
		case 0 :
		draw_string(marge,hauteur+60,"(Decollage) / heure :","10x20","black", X_var);
		marge+=10*(21+1);
		draw_string(marge,hauteur+60,La->avion.heure_decollage,"10x20","black", X_var);
		marge+=10*(4+1);
			if (Determine_Retard(Hor, La->avion.heure_decollage )==-1)
			draw_string(marge,hauteur+60,"[en retard] Piste 1","10x20","black", X_var);
			else draw_string(marge,hauteur+60,"[a l'heure] Piste 1","10x20","black", X_var);
		marge=20;
		display_window(X_var);
		break;
		
		case 1 :
		draw_string(marge,hauteur+60,"(Decollage) / heure :","10x20","black", X_var);
		marge+=10*(21+1);
		draw_string(marge,hauteur+60,La->avion.heure_decollage,"10x20","black", X_var);
		marge+=10*(4+1);
			if (Determine_Retard(Hor, La->avion.heure_decollage )==-1)
			draw_string(marge,hauteur+60,"[en retard] Piste 2","10x20","black", X_var);
			else draw_string(marge,hauteur+60,"[a l'heure] Piste 2","10x20","black", X_var);
		marge=20;
		display_window(X_var);
		break;
		
		case 2 :
		draw_string(marge,hauteur+60,"(Atterrissage) / carb. :","10x20","black", X_var);
		sprintf(tab,"%2d", La->avion.carburant);
		Completion_Numerique(La->avion.carburant,2,tab);
		marge+=10*(24+1);
		draw_string(marge,hauteur+60,tab,"10x20","black", X_var);
		marge+=10*(2+1);
		draw_string(marge,hauteur+60,"Piste 1","10x20","black", X_var);
		marge=20;
		display_window(X_var);
		break;
		
		case 3 :
		draw_string(marge,hauteur+60,"(Atterrissage) / carb. :","10x20","black", X_var);
		sprintf(tab,"%2d", La->avion.carburant);
		Completion_Numerique(La->avion.carburant,2,tab);
		marge+=10*(24+1);
		draw_string(marge,hauteur+60,tab,"10x20","black", X_var);
		marge+=10*(2+1);
		draw_string(marge,hauteur+60,"Piste 2","10x20","black", X_var);
		marge=20;
		display_window(X_var);
		break;
		
		case 4 :
		draw_string(marge,hauteur+60,"(Urgence) Piste 1","10x20","black", X_var);
		display_window(X_var);
		marge=20;
		display_window(X_var);
		break;
		
		case 5 :
		draw_string(marge,hauteur+60,"(Urgence) Piste 2","10x20","black", X_var);
		display_window(X_var);
		marge=20;
		display_window(X_var);
		break;
		
		case 6 :
		draw_string(marge,hauteur+60,"(Liste Noire)","10x20","black", X_var);
		display_window(X_var);
		marge=20;
		display_window(X_var);
		break;
		
		case 7 :
		draw_string(marge,hauteur+60,"(Innocupe)","10x20","black", X_var);
		display_window(X_var);
		marge=20;
		display_window(X_var);
		break;
	}
		
	do {
	mouse_wait(&x,&y,X_var);
		
		if (x> bordG1 && x<bordD1 && y>hauteur-20+5+2 && y<hauteur+5+2) {
			draw_filled_rectangle(marge,hauteur+40,535,30,"white",X_var);
			if (La->suivant_compagnie!=NULL) La=La->suivant_compagnie;
			break;
			}
		
		if (x> bordG2 && x<bordD2 && y>hauteur-20+5+2 && y<hauteur+5+2) {
			draw_filled_rectangle(marge,hauteur+40,535,30,"white",X_var);
			if (La->precedent_compagnie!=NULL) La=La->precedent_compagnie;
			break;
			}
		if (x>20+550 && x<20+550+10*lph5 && y>hauteur+40+5+2 && y<hauteur+60+5+2) return;
				
		} while (1);
	}
}

/******************************************************************************/

/******************************************************************************/
/*LES FONCTIONS SERVANT A LA GESTION DES COMPAGNIES*/
/******************************************************************************/

/*************************Creer_Compagnie**************************************/

Compagnie *Creer_Compagnie(MlvType *X_var) {
	Compagnie *Ptrc;
	/* l'objet est créé concretement dans la mémoire: les pointeurs*/ 
	/*pointeront sur lui par la suite*/	
	Ptrc=(Compagnie *)malloc(sizeof(Compagnie));
		if (Ptrc==NULL) {
		fprintf(stderr,"Erreur dans l'allocation d'une zone memoire\n");
		exit(1);
		}
	/*On va lui donner un nom choisi parmi une liste de nom écrit dans un fichier*/
	FILE * Lecture;
	char *Nom_du_Fichier = "LstComp.txt";
	if((Lecture=fopen(Nom_du_Fichier,"rt"))==NULL) {
	fprintf(stderr,"Erreur d'ouverture de fichier\n");
	exit(1);
	}
	/*Voici le moment où l'utilisateur va choisir le nom de la compagnie*/
	Demande_Comp(20,"Pour choisir la compagnie :", Lecture, &Ptrc->nom, Ptrc->acronyme, 3, X_var);
	draw_string(20,100+20,"Vous avez choisi:","10x20","black", X_var);
	if (Ptrc->nom==NULL) draw_string(20,140,"Rien","10x20","black", X_var);
	else  {
	draw_string(20,140,(*Ptrc).nom,"10x20","black", X_var);
	draw_string(20,160,(*Ptrc).acronyme,"10x20","black", X_var);
	}
	display_window(X_var);
	fclose(Lecture);
	Ptrc->avions_compagnie=NULL;
	/*Voici le moment où on invite l'utilisateur à quitter*/ 
	/*le menu de selection de la compagnie*/	
	int x_souris, y_souris;
	draw_string(20,200,"Cliquez n'importe ou dans la fenetre pour quitter","10x20","black", X_var);
	display_window(X_var);
	mouse_wait(&x_souris,&y_souris,X_var);
	/*On retourne la compagnie*/	
	return Ptrc;
	}

/**************************Alloue_Cellule_compagnie****************************/

Liste_compagnie Alloue_Cellule_compagnie(void) {
	Liste_compagnie Lc;
	if((Lc=(Liste_compagnie)malloc(sizeof(Cellule_compagnie)))==NULL) {
	fprintf(stderr,"Erreur dans l'allocation d'une Cellule_Compagnie\n");	
	exit(1);
	}
	return(Lc);
	}

/****************************Creer_Liste_compagnie*****************************/
	
Liste_compagnie Creer_Liste_compagnie(Compagnie c) {
	Liste_compagnie Lc;
	Lc=Alloue_Cellule_compagnie();
	/*Attention: une affectation de structure fait pointer*/ 
	/*les pointeurs de l'un sur les cibles de l'autre donc même s'il y a*/
	/*une affectation on ne libèrera pas la compagnie c */
	Lc->comp=c; 
	Lc->suivant=NULL;
	return Lc;
	}


/***********************Insere_Fin_Liste_compagnie*****************************/

/*Insertion en Fin: permet de controler si c n'existe pas déjà et évite des*/
/*remanipulations de "suivant"*/

Liste_compagnie Insere_Fin_Liste_compagnie(Liste_compagnie *PtrLc, Compagnie c) {
	for(;*PtrLc!=NULL && strcmp((*PtrLc)->comp.nom,c.nom)!=0;PtrLc=&(*PtrLc)->suivant);
	if (*PtrLc==NULL) {
	*PtrLc=Creer_Liste_compagnie(c);
	return *PtrLc;
	}
	else return NULL;
	}

/***************************Liberer_Liste_compagnie****************************/

void Liberer_Liste_compagnie(Liste_compagnie Lc) {
	Liste_compagnie LcTmp;
	while(Lc!=NULL){
	LcTmp=Lc;
	Lc=Lc->suivant;
	free(LcTmp->comp.nom);
/* ici, faire un free(LcTmp) aurait été équivalent*/
/*si on a l'idée de mettre free(LcTmp) à la fin, l'ordinateur nous dit qu'il y*/
/*a "double free or corruption" */
	free(&(LcTmp->comp));
		}
	}

/**************************Identifie_Compagnie*********************************/

int Identifie_Compagnie(Compagnie c1, Compagnie c2) {
	return strcmp(c1.acronyme,c2.acronyme);
	}

/*************************Supprime_Compagnie***********************************/

/*Utilisé pour la Liste Noire*/

Liste_compagnie Supprime_Compagnie(Liste_compagnie *PtrLc, Compagnie c_indice) {
	printf("Supprime_Compagnie \n");	
	Liste_compagnie LcTmp;
	for(;*PtrLc!=NULL && Identifie_Compagnie((*PtrLc)->comp,c_indice)!=0;PtrLc=&(*PtrLc)->suivant);
	if (*PtrLc==NULL) return NULL;
	LcTmp=*PtrLc;
	*PtrLc=LcTmp->suivant;/*le pointeur est utile ici, pour le cas NULL*/
	LcTmp->suivant=NULL;
	return LcTmp;
	}

/************************Change_Compagnie_Liste_Debut**************************/

Liste_compagnie Change_Compagnie_Liste_Debut(Compagnie c, Liste_compagnie *LcSource, Liste_compagnie *LcBut) {
	printf("Change_Compagnie_Liste_Debut \n");
	Liste_compagnie LcTmp;
	if ((LcTmp=Supprime_Compagnie(LcSource, c))==NULL) return NULL;
	LcTmp->suivant=*LcBut;
	*LcBut=LcTmp;
	return LcTmp;
	}

/***********************Recherche_Compagnie************************************/

Liste_compagnie Recherche_Compagnie(Liste_compagnie Lc, Compagnie c) {
	printf("Recherche_Compagnie \n");
	Liste_compagnie LcTmp;
	for(LcTmp=Lc;LcTmp!=NULL && Identifie_Compagnie(LcTmp->comp,c)!=0;LcTmp=LcTmp->suivant);
	return LcTmp;
	}

/******************************************************************************/

/******************************************************************************/
/*LES FONCTIONS SERVANT A LA GESTION DES AVIONS*/
/******************************************************************************/

/******************************Creer_Avion*************************************/

Avion *Creer_Avion(Liste_compagnie Lc, char mode, Horloge Hor, MlvType *X_var) {
	printf("Creer_Avion \n");
	Liste_avions Recherche_Avion_Compagnie_Simple(Liste_avions , Avion );
	Avion *Ptra;
	int num;
	int minutes;
	int heure;
	int x_souris, y_souris;
	Horloge HorTmp;
	char AfficheHeure[6];
/*l'objet est créé concretement dans la mémoire: les pointeurs pointeront'''''*/ 
/*sur lui par la suite''''''''''''''''''''''''''''''''''''''''''''''''''''''''*/	
	Ptra=(Avion *)malloc(sizeof(Avion));
		if (Ptra==NULL) {
		fprintf(stderr,"Erreur dans l'allocation d'une zone memoire\n");
		exit(1);
		}
/*Tout d'abord, On construit le nom de l'avion à partir	de l'acronyme de la'''*/
/*compagnie'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''*/
	sprintf(Ptra->identifiant,"%s", Lc->comp.acronyme);
/*Ensuite l'utilisateur va lui donner un numéro de vol: ici il n'y a pas''''''*/
/*unicité du numero de vol comme le stipule la norme, mais seulement une''''''*/
/*unicité restreinte dans la compagnie (non precisé dans l'énoncé)''''''''''''*/	
	do {
	draw_filled_rectangle(0,100,W,H-100,"white",X_var);
	Choisir_nbr(100+20,"Pour choisir le numero de vol :", &num, 1, 1000, 4, X_var);
	sprintf(Ptra->identifiant+3,"%3d", num);	
	Completion_Numerique(num,3,Ptra->identifiant+3);
	if (Recherche_Avion_Compagnie_Simple(Lc->comp.avions_compagnie, *Ptra)!=NULL) {
	draw_string(20, 200+20,"Cet avion existe deja. ressaisir un numero de vol SVP.","10x20","black", X_var);
	draw_string(20,200+40,"Cliquez n'importe ou dans la fenetre pour continuer.","10x20","black", X_var);
	display_window(X_var);
	mouse_wait(&x_souris,&y_souris,X_var);
	}
	else break;
	} while(1);
/*La facon de remplir la caractéristique "consommation" n'était pas precisé'''*/
/*dans la consigne: on l'a rempli donc de maniere aléatoire dans''''''''''''''*/ 
/*l'intervalle [1;5]''''''''''''''''''''''''''''''''''''''''''''''''''''''''''*/	
	Ptra->consommation=1+random(5);
/*On différencie le mode de l'avion'''''''''''''''''''''''''''''''''''''''''''*/
	switch(mode) {
	case 'a' :
		Choisir_nbr(200+20,"Pour choisir la quant. de carb. :", &Ptra->carburant, 1, 100, 3, X_var);
/*Initialisation temporaire pour le decollage'''''''''''''''''''''''''''''''''*/
		Ptra->mode=2+random(2);
		break;
	
	case 'd' :
		do {	
		draw_string(20, 200+20,"En ce moment, il est       (cette heure sera appele t par la suite).", "10x20", "black", X_var);
		draw_string(20, 200+2*20,"Toute date de decollage est comptee pour le jour meme et doit etre superieure a t+5 min.", "10x20", "black", X_var);
		draw_string(20, 200+3*20,"Le mode de saisie de l'heure de decollage est sous la forme HH:MM .", "10x20", "black", X_var);
		sprintf(AfficheHeure,"%2d:%2d",Hor.Heure,Hor.Minutes);
		Completion_Numerique(Hor.Heure,2,AfficheHeure);
		Completion_Numerique(Hor.Minutes,2,AfficheHeure+3);
		draw_string(20+21*10,220,AfficheHeure,"10x20","brown3", X_var);		
		Choisir_nbr(220+4*20,"Pour choisir l'heure stricte (HH) de decollage :", &heure, 0, 24, 2, X_var);
		Choisir_nbr(320+4*20,"Pour choisir les minutes (MM) :", &minutes, 0, 60, 2, X_var);
		sprintf(Ptra->heure_decollage,"%2d%2d", heure, minutes);
		Completion_Numerique(heure,2,Ptra->heure_decollage);
		Completion_Numerique(minutes,2,Ptra->heure_decollage+2);
		if (Hor.Minutes+5==(Hor.Minutes+5)%60) {
			HorTmp.Minutes=Hor.Minutes+5;
			HorTmp.Heure=Hor.Heure;
			}
		else {
			HorTmp.Minutes=(Hor.Minutes+5)%60;
			HorTmp.Heure=(Hor.Heure+1)%24;
			}
		if (Determine_Retard(HorTmp,Ptra->heure_decollage)==1) break;
		else {
			draw_string(20,420+4*20,"Mauvaise Heure. Ressaisir une Heure SVP.","10x20","black", X_var);
			draw_string(20,440+4*20,"Cliquez n'importe ou dans la fenetre pour recommencer.","10x20","black", X_var);
			display_window(X_var);
			mouse_wait(&x_souris,&y_souris,X_var);
			draw_filled_rectangle(20,440+2*20,1000,100,"white", X_var);
			}
		} while (1);
/*Initialisation temporaire pour le decollage'''''''''''''''''''''''''''''''''*/
		Ptra->mode=random(2);
		break;
	}
	Ptra->compagnie=NULL;
/*Voici le moment où on invite l'utilisateur à quitter le menu de selection'''*/
/*de la compagnie'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''*/	
	draw_string(20,420+4*20,"Cliquez n'importe ou dans la fenetre pour quitter.","10x20","black", X_var);
	display_window(X_var);
	mouse_wait(&x_souris,&y_souris,X_var);
/*On retourne l'avion'''''''''''''''''''''''''''''''''''''''''''''''''''''''''*/	
	return Ptra;
	}

/******************************Initialiser_Avion*************************************/

void Initialiser_Avion(Liste_avions La, char mode, Horloge Hor, MlvType *X_var) {
	printf("Initialiser_Avion \n");
	Liste_avions Recherche_Avion_Compagnie_Double(Liste_avions , Avion );
	int num;
	int minutes;
	int heure;
	int x_souris, y_souris;
	Horloge HorTmp;
	char AfficheHeure[6];
/* l'objet est déjà créé concretement dans la mémoire et est déjà'''''''''''''*/ 
/*affilié à une compagnie L'utilisateur va alors lui donner'''''''''''''''''''*/ 
/*un nouveau numéro de vol''''''''''''''''''''''''''''''''''''''''''''''''''''*/	
	La->avion.identifiant[3]='-';
	La->avion.identifiant[4]='-';
	La->avion.identifiant[5]='-';
	La->avion.identifiant[6]='\0';
	Avion aTmp;
	do {
	draw_filled_rectangle(0,0,W,H,"white",X_var);
	draw_string(20,20,"L'ancien numero de vol de l'avion a ete mis a ---.","10x20","black", X_var);
	Choisir_nbr(40,"Pour choisir le numero de vol :", &num, 1, 1000, 4, X_var);
	strcpy(aTmp.identifiant,La->avion.identifiant);
	sprintf(aTmp.identifiant+3,"%3d", num);
	Completion_Numerique(num,3,aTmp.identifiant+3);
	if (Recherche_Avion_Compagnie_Double(La, aTmp)!=NULL) {
	draw_string(20, 100+40,"Cet avion existe deja. ressaisir un numero de vol SVP.","10x20","black", X_var);
	draw_string(20,100+60,"Cliquez n'importe ou dans la fenetre pour continuer.","10x20","black", X_var);
	display_window(X_var);
	mouse_wait(&x_souris,&y_souris,X_var);
	}
	else break;
	} while(1);
	sprintf(La->avion.identifiant+3,"%3d", num);
	Completion_Numerique(num,3,La->avion.identifiant+3);
/*La facon de remplir la caractéristique "consommation" n'était pas precisé'''*/
/*dans la consigne: on l'a rempli donc de maniere aléatoire dans''''''''''''''*/ 
/*l'intervalle [1;5]''''''''''''''''''''''''''''''''''''''''''''''''''''''''''*/	
	La->avion.consommation=1+random(5);
/*On différencie le mode de l'avion'''''''''''''''''''''''''''''''''''''''''''*/
	switch (mode) {
	
	case 'a' :
		Choisir_nbr(100+20+20,"Pour choisir la quant. de carb. :", &La->avion.carburant,1 , 100, 3, X_var);
		La->avion.mode=2+random(2);
		break;
	
	case 'd' :
		do {	
		draw_string(20, 100+2*20,"En ce moment, il est       (cette heure sera appele t par la suite).", "10x20", "black", X_var);
		draw_string(20, 100+3*20,"Toute date de decollage est comptee pour le jour meme et doit etre superieure a t+5 min.", "10x20", "black", X_var);
		draw_string(20, 100+4*20,"Le mode de saisie de l'heure de decollage est sous la forme HH:MM .", "10x20", "black", X_var);
		sprintf(AfficheHeure,"%2d:%2d",Hor.Heure,Hor.Minutes);
		Completion_Numerique(Hor.Heure,2,AfficheHeure);
		Completion_Numerique(Hor.Minutes,2,AfficheHeure+3);
		draw_string(20+21*10,100+2*20,AfficheHeure,"10x20","brown3", X_var);		
		Choisir_nbr(100+6*20,"Pour choisir l'heure stricte (HH) de decollage :", &heure, 0, 24, 2, X_var);
		Choisir_nbr(200+6*20,"Pour choisir les minutes (MM) :", &minutes, 0, 60, 2, X_var);
		sprintf(La->avion.heure_decollage,"%2d%2d", heure, minutes);
		Completion_Numerique(heure,2,La->avion.heure_decollage);
		Completion_Numerique(minutes,2,La->avion.heure_decollage+2);
		if (Hor.Minutes+5==(Hor.Minutes+5)%60) {
			HorTmp.Minutes=Hor.Minutes+5;
			HorTmp.Heure=Hor.Heure;
			}
		else {
			HorTmp.Minutes=(Hor.Minutes+5)%60;
			HorTmp.Heure=(Hor.Heure+1)%24;
			}
		if (Determine_Retard(HorTmp,La->avion.heure_decollage)==1) break;
		else {
			draw_string(20,300+6*20,"Mauvaise Heure. Ressaisir une Heure SVP.","10x20","black", X_var);
			draw_string(20,300+7*20,"Cliquez n'importe ou dans la fenetre pour recommencer.", "10x20","black", X_var);
			display_window(X_var);
			mouse_wait(&x_souris,&y_souris,X_var);
			draw_filled_rectangle(20,300+5*20,1000,100,"white", X_var);
			}
		} while (1);
		La->avion.mode=random(2);
		break;
	}	
/*Voici le moment où on invite l'utilisateur à quitter le menu de selection'''*/
/*de la compagnie'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''*/	
	draw_string(20,300+6*20,"Cliquez n'importe ou dans la fenetre pour quitter.","10x20","black", X_var);
	display_window(X_var);
	mouse_wait(&x_souris,&y_souris,X_var);
	}

/*****************************Alloue_Cellule_avion*****************************/

Liste_avions Alloue_Cellule_avion(void) {
	printf("Alloue_Cellule_avion \n");
	Liste_avions La;
	if((La=(Liste_avions)malloc(sizeof(Cellule_avion)))==NULL) {
	fprintf(stderr,"Erreur dans l'allocation d'une Cellule_avion");	
	exit(1);
	}
	return La;
	}

/*****************************Creer_Liste_avions*******************************/

Liste_avions Creer_Liste_avions (Avion a) {
	printf("Creer_Liste_avions \n");
	Liste_avions La;
	La=Alloue_Cellule_avion();
/*Attention une affectation de structure fait pointer les pointeurs'''''''''''*/
/*de l'un sur les cibles de l'autre* donc même si s'il y a''''''''''''''''''''*/ 
/*une affectation on ne libèrera pas l'avion a''''''''''''''''''''''''''''''''*/
	La->avion=a;
/*On initialise en NULL tous les pointeurs''''''''''''''''''''''''''''''''''''*/	
	La->avion.compagnie=NULL;
	La->precedent_compagnie=NULL;
	La->suivant_compagnie=NULL;
	La->suivant_attente=NULL;
	return La;
	}

/********************Insere_Fin_Liste_avions_Compagnie*************************/

/*Insertion en Fin: permet de controler si c n'existe pas déjà et évite des*/
/*remanipulations de "suivant"*/

Liste_avions Insere_Fin_Liste_avions_Compagnie(Compagnie *Ptrc, Avion a) {
	printf("Insere_Fin_Liste_avions_Compagnie \n");
	Liste_avions *PtrLa=&Ptrc->avions_compagnie;
	if (*PtrLa==NULL) {
	*PtrLa=Creer_Liste_avions(a);
	(*PtrLa)->avion.compagnie=Ptrc;
	return *PtrLa;
	}
	else {
	if (strcmp((*PtrLa)->avion.identifiant,a.identifiant)==0) return NULL;
	for(;(*PtrLa)->suivant_compagnie!=NULL;PtrLa=&(*PtrLa)->suivant_compagnie)
	if (strcmp((*PtrLa)->suivant_compagnie->avion.identifiant,a.identifiant)==0) return NULL;
	(*PtrLa)->suivant_compagnie =Creer_Liste_avions(a);
	(*PtrLa)->suivant_compagnie->precedent_compagnie =*PtrLa;
	return (*PtrLa)->suivant_compagnie;
	}
	}

/*********************Insere_Debut_Liste_avions_Attente************************/

void Insere_Debut_Liste_avions_Attente(Liste_avions *PtrLa, Liste_avions La_objet) {
	printf("Insere_Debut_Liste_avions_Attente \n");
	La_objet->suivant_attente=*PtrLa; /*le pointeur n'est utile que si la liste est NULL*/
	*PtrLa=La_objet;
	}

/**************************Identifie_Avion*************************************/

int Identifie_Avion(Avion a1, Avion a2) {
	return strcmp(a1.identifiant,a2.identifiant);
	}

/*********************Insere_Avant_avions_Attente******************************/

void Insere_Avant_avions_Attente(Liste_avions *PtrLa, Avion a_indice, Liste_avions La_objet) {
	printf("Insere_Avant_avions_Attente \n");
	for(;*PtrLa!=NULL && Identifie_Avion((*PtrLa)->avion,a_indice)!=0  ;
	PtrLa=&(*PtrLa)->suivant_attente);
	if (*PtrLa==NULL) return;
	La_objet->suivant_attente =*PtrLa;
	*PtrLa= La_objet;
	}

/*************************Supprime_Avion_Attente*******************************/

/*très souvent utilisée*/

Liste_avions Supprime_Avion_Attente(Liste_avions *PtrLa, Avion a_indice) {
	printf("Supprime_Avion_Attente \n");
	Liste_avions LaTmp;
	for(;*PtrLa!=NULL && Identifie_Avion((*PtrLa)->avion,a_indice)!=0;PtrLa=&(*PtrLa)->suivant_attente);
	if (*PtrLa==NULL) return NULL;
	LaTmp=*PtrLa;
	*PtrLa=LaTmp->suivant_attente;/*le pointeur est utile ici, pour le cas NULL*/
	LaTmp->suivant_attente=NULL;
	return LaTmp;
	}

/*************************Supprime_Avion_Compagnie*****************************/

/*La Suppression dans Compagnie: la où on exploite le double chainage*/
/*Utilite: Crash et décollage*/

void Supprime_Avion_Compagnie(Compagnie *Ptrc, Avion a_indice) {
	printf("Supprime_Avion_Compagnie\n");
	Liste_avions LaTmp;
/*Le cas où l'avion à supprimer est en début de liste*/
	if (Ptrc->avions_compagnie!=NULL && Identifie_Avion(Ptrc->avions_compagnie->avion,a_indice)==0) {
		LaTmp=Ptrc->avions_compagnie;
		Ptrc->avions_compagnie=LaTmp->suivant_compagnie;
		if (LaTmp->suivant_compagnie!=NULL) {
		Ptrc->avions_compagnie->precedent_compagnie=NULL;
		Ptrc->avions_compagnie->avion.compagnie=Ptrc;
		}
		LaTmp->avion.compagnie=NULL;
		LaTmp->suivant_compagnie=NULL;
		free(LaTmp);
		return;
		}
/*Dans l'autre cas*/	
	else {
	for(LaTmp=Ptrc->avions_compagnie;LaTmp!=NULL && Identifie_Avion(LaTmp->avion,a_indice)!=0;LaTmp=LaTmp->suivant_compagnie);
	if (LaTmp==NULL) return;
	if (LaTmp->suivant_compagnie!=NULL) LaTmp->suivant_compagnie->precedent_compagnie=LaTmp->precedent_compagnie;
	LaTmp->precedent_compagnie->suivant_compagnie=LaTmp->suivant_compagnie;
	LaTmp->suivant_compagnie=NULL;
	LaTmp->precedent_compagnie=NULL;
	free(LaTmp);
	}
	}

/***********************Recherche_Attente_ParNomComp***************************/

Liste_avions Recherche_Attente_ParNomComp(Liste_avions La, char ACR[]) {
	printf("Recherche_Attente_ParNomComp\n");
	Liste_avions LaTmp;
	for(LaTmp=La;LaTmp!=NULL && strncmp(LaTmp->avion.identifiant,ACR,3)!=0;LaTmp=LaTmp->suivant_attente);
	return LaTmp;
	}

/***********************Recherche_Avion_ParMode********************************/

Liste_avions Recherche_Avion_ParMode(Liste_avions La, int mode) {
	printf("Recherche_Avion_ParMode\n");
	Liste_avions LaTmp;
	for(LaTmp=La;LaTmp!=NULL && LaTmp->avion.mode!=mode ;LaTmp=LaTmp->suivant_compagnie);
	return LaTmp;
	}

/***********************Recherche_Attente_ParMode********************************/

Liste_avions Recherche_Attente_ParMode(Liste_avions La, int mode) {
	printf("Recherche_Attente_ParMode \n");
	Liste_avions LaTmp;
	for(LaTmp=La;LaTmp!=NULL && LaTmp->avion.mode!=mode ;LaTmp=LaTmp->suivant_attente);
	return LaTmp;
	}

/******************************Recherche_Attente*******************************/
	
Liste_avions Recherche_Attente(Liste_avions La, Avion a) {
	printf("Recherche_Attente\n");
	Liste_avions LaTmp;
	for(LaTmp=La;LaTmp!=NULL && Identifie_Avion(LaTmp->avion,a)!=0;LaTmp=LaTmp->suivant_attente);
	return LaTmp;
	}

/**********************Recherche_Avion_Compagnie_Simple************************/

Liste_avions Recherche_Avion_Compagnie_Simple(Liste_avions La, Avion a) {
	printf("Recherche_Avion_Compagnie_Simple \n");
	Liste_avions LaTmp;
	for(LaTmp=La;LaTmp!=NULL && Identifie_Avion(LaTmp->avion,a)!=0;LaTmp=LaTmp->suivant_compagnie);
	return LaTmp;
	}

/********************Recherche_Avion_Compagnie_Double**************************/

Liste_avions Recherche_Avion_Compagnie_Double(Liste_avions La, Avion a) {
	printf("Recherche_Avion_Compagnie_Double \n");
	Liste_avions LaTmp;
	for(LaTmp=La;LaTmp!=NULL && Identifie_Avion(LaTmp->avion,a)!=0;LaTmp=LaTmp->suivant_compagnie);
	if (LaTmp!=NULL) return LaTmp;
	for(LaTmp=La;LaTmp!=NULL && Identifie_Avion(LaTmp->avion,a)!=0;LaTmp=LaTmp->precedent_compagnie);
	return LaTmp;
	}

/************************Liberer_Liste_avions_Attente**************************/

void Liberer_Liste_avions_Attente(Liste_avions La) {
	Liste_avions LaTmp;
	while(La!=NULL){
	LaTmp=La;
	La=La->suivant_attente;
	free(LaTmp);
		}
	}

/************************Change_avion_Liste_Debut******************************/

void Change_avion_Liste_Debut(Avion a, Liste_avions *LaSource, Liste_avions *LaBut, int mode) {
	printf("Change_avion_Liste_Debut \n");
	Liste_avions LaTmp;
	if((LaTmp=Supprime_Avion_Attente(LaSource, a))==NULL) return;
	LaTmp->avion.mode=mode;
	LaTmp->suivant_attente=*LaBut;
	*LaBut=LaTmp;
	}

/***************************Insere_Atterrissage********************************/

/*Et voici l'exception pour les fonctions d'insertion à cause du tri obligatoire*/

void Insere_Atterrissage(Liste_avions *PtrLat, Liste_avions La_objet) {
	printf("Insere_Atterrissage \n");
	int precision=MinutesParTour;
	Liste_avions *PtrLaTmp;
	for(PtrLaTmp=PtrLat;*PtrLat!=NULL; PtrLat=&(*PtrLat)->suivant_attente) 
		if (((La_objet->avion.carburant)/(La_objet->avion.consommation))/precision <=
		    (((*PtrLat)->avion.carburant)/((*PtrLat)->avion.consommation))/precision) break;	
	if (*PtrLat==NULL) *PtrLat=La_objet; /* le pointeur est seulement utile ici*/
	else Insere_Avant_avions_Attente(PtrLaTmp, (*PtrLat)->avion, La_objet);
	}

/*Expliquons la façon dont on trie les avions: on considère qu'un tour fait 4 minutes*/
/*On prends 4 minutes car il importait que le nombre de minutes soit 3 < */
/* afin de donner une certaine difficulté au jeu en n'autorisant pas un rangement strict mais approximatif*/
/*c'est à dire non plus avec une precision par minutes (au lieu de 4 on aurait pris 1)*/
/* mais une precision de 4 minutes (correspondant au tour)*/
/* de plus l'equivalence (x minutes)=(1 tour) (avec x>1) donne tous son sens au bouton de changement de priorité*/
/*car des avions pour un même nombre de tour ne seront pas obligatoirement dans le même état d'urgence*/

/************************Change_Avion_En_Urgence*******************************/

void Change_Avion_En_Urgence(Avion a, Liste_avions *LaSource, Liste_avions *LaBut, int mode) {
	printf("Change_Avion_En_Urgence\n");
	Liste_avions LaTmp;
/*on supprime l'avion de la liste d'atterrissage*/
	if((LaTmp=Supprime_Avion_Attente(LaSource, a))==NULL) return;
	LaTmp->avion.mode=mode;
/*on le réinsere dans la même liste: l'avion sera replacé de manière trié dans la liste*/
/*au mieux il aura avancé de plusieurs places comme on le souhaitait*/
/*en fait il se sera mis devant tous ceux qui avaient le même nombre de tour que lui*/
/*sinon il restera à sa place: l'utilisateur l'aura cependant mis en urgence*/
	Insere_Atterrissage(LaBut, LaTmp); /*le pointeur n'est utile qu'ici*/
	}


/*********************Supprime_Avion_Premier_Queue*****************************/

Liste_avions Supprime_Avion_Premier_Queue(Queue *PtrFq) {
	printf("Supprime_Avion_Premier_Queue \n");
	if (PtrFq->dernier==NULL) return NULL;
	Liste_avions LaTmp=PtrFq->dernier;
	if (LaTmp->suivant_attente!=NULL) {
		for(;LaTmp->suivant_attente->suivant_attente!=NULL;LaTmp=LaTmp->suivant_attente);
		PtrFq->premier=LaTmp;
		LaTmp=Supprime_Avion_Attente(&PtrFq->dernier, LaTmp->suivant_attente->avion);
		}
	else {
		PtrFq->premier=NULL;
		LaTmp=Supprime_Avion_Attente(&PtrFq->dernier, LaTmp->avion);
		}
	return LaTmp;
	}

/*************************Supprime_Avion_Queue*********************************/


Liste_avions Supprime_Avion_Queue(Queue *PtrFq, Avion a_indice) {
	printf("Supprime_Avion_Queue \n");
	Liste_avions LaTmp;
	if ((LaTmp=Recherche_Attente(PtrFq->dernier, a_indice))==NULL) return NULL;
	if (LaTmp->suivant_attente==NULL) LaTmp=Supprime_Avion_Premier_Queue(PtrFq);
	else LaTmp=Supprime_Avion_Attente(&PtrFq->dernier, a_indice);
	return LaTmp;
	}


/***********************Introduire_Avions_Liste_Noire**************************/

/*La fonction qui suit était plus qu'adéquat pour une implementation à arguments variables*/
/*en argument, on y mettra les differentes liste d'embarquement et on terminera la liste par NULL*/

void Introduire_Avions_Liste_Noire(FILE *sauv, Parking *Park[4], Liste_avions Lac, int Nbr_L, int Nbr_Q, int couleur, ...) {
	printf("Introduire_Avions_Liste_Noire \n");
	void Libere_Place_Affichage(Avion , Parking * , int );
	va_list ArgTmp;
	Liste_avions *Arg_Liste;
	Queue *Arg_Queue;
	int i;
	int Cpt_Liste, Cpt_Queue;
	Liste_avions LaTmp;
	
	for(;Lac!=NULL; Lac=Lac->suivant_compagnie) {
	Cpt_Liste=Nbr_L;
	Cpt_Queue=Nbr_Q;	
		
	if (Lac->avion.mode>=2 && Lac->avion.mode<=7) {
		if (Lac->avion.mode!=7) {
		Lac->avion.info[7]='N';
		fprintf(sauv, Lac->avion.info);
		}
		Lac->avion.mode=couleur;
		}
	else {
		for (i=0;i<2;i++) Libere_Place_Affichage(Lac->avion, Park[i], 9);
		for (i=2;i<4;i++) Libere_Place_Affichage(Lac->avion, Park[i], 5);
				
		Lac->avion.mode=couleur;
		va_start(ArgTmp, couleur);
	
	while(Cpt_Liste>0) {
		Arg_Liste=va_arg(ArgTmp,Liste_avions *);
		if ((LaTmp=Supprime_Avion_Attente(Arg_Liste, Lac->avion))!=NULL) break; /*réduit la complexité*/
		Cpt_Liste--;
		}
	
	while(Cpt_Liste==0 && Cpt_Queue>0) {
		Arg_Queue=va_arg(ArgTmp,Queue *);
		if ((LaTmp=Supprime_Avion_Queue(Arg_Queue, Lac->avion))!=NULL) break;
		Cpt_Queue--;
		}
	}
	}
	va_end(ArgTmp);
	}

/***********************Recherche_Compagnie_Affilie****************************/

Liste_compagnie Recherche_Compagnie_Affilie(Liste_compagnie Lc, Avion a_indice) {
	printf("Recherche_Compagnie_Affilie \n");
	Liste_compagnie LcTmp;
	for(LcTmp=Lc;LcTmp!=NULL && strncmp(a_indice.identifiant, LcTmp->comp.acronyme, 3)!=0;LcTmp=LcTmp->suivant);
	return LcTmp;
	}

/**************************Crash_Atterrissage**********************************/

void Crash_Atterrissage(Liste_compagnie Lc, Liste_compagnie Ln, Liste_avions *PtrLat, couple *Graph, char *TabColor[], FILE* sauv, char *infoPiste , MlvStruct *x_var) {
	printf("Crash_Atterrissage \n");
	int i;
	Liste_compagnie LcTmp;
	Liste_avions LaTmp;
	Liste_avions LaTmp_passage;
	for(LaTmp=*PtrLat;LaTmp!=NULL;) {
		if (LaTmp->avion.carburant<=0) {
		for(i=0;i<=6;i++) {
		LaTmp->avion.mode=4*(i%2==0)+2*(i%2==1);
		Affiche_Avion(&LaTmp->avion, Graph, TabColor, x_var);
		display_window(x_var);
		wait_micros(500000);
		printf("\a\n");
			}
		LaTmp->avion.info[7]='C';
		fprintf(sauv, LaTmp->avion.info);
		strncpy(infoPiste, LaTmp->avion.info, 21);
		LaTmp_passage=LaTmp->suivant_attente;
		LaTmp=Supprime_Avion_Attente(PtrLat, LaTmp->avion);
		LcTmp=Recherche_Compagnie_Affilie(Lc, LaTmp->avion);
		if (LcTmp==NULL) LcTmp=Recherche_Compagnie_Affilie(Ln, LaTmp->avion);
		Supprime_Avion_Compagnie(&LcTmp->comp, LaTmp->avion);
		LaTmp=LaTmp_passage;
		}
	else LaTmp=LaTmp->suivant_attente;
	}
	}

/*============================================================================*/
/* Les fonctions qui suivent permettent d'organiser l'affichage des avions    */
/* et de les ranger dans les ordres de priorités imposés par l'énoncé         */ 
/* ainsi que les fonctions d'affichage des évènements.                        */
/*============================================================================*/

/**************************Remplir_Affiche_A***********************************/

void Remplir_Affiche_A(couple *Affiche, int taille, int Dx, int PoleInf, int Gx, int PoleSup) {
	float p=(((float)(1))/((float)(taille+1)));
	int X=Dx;
	int Y=(int)(0.5*(PoleSup+0.5*(PoleInf+PoleSup)));
	int i;
	for(i=0;i<taille;i++) {
	X+=(int)(p*(Gx-Dx));
	Affiche[i].x=X;
	Affiche[i].y=Y;
	}
	}

/**************************Remplir_Affiche_D***********************************/

void Remplir_Affiche_D(Parking *Affiche, int taille, int Dx, int PoleInf, int Gx, int PoleSup) {
	float p=(((float)(1))/((float)(taille+1)));
	int X=Dx;
	int Y=(int)(0.5*(PoleSup+0.5*(PoleInf+PoleSup)));
	int i;
	for(i=0;i<taille;i++) {
	Affiche[i].etat=0; /*inoccupe*/
	X+=(int)(p*(Gx-Dx));
	Affiche[i].place.x=X;
	Affiche[i].place.y=Y;
	}
	}

/************************Ranger_Liste_Avions_A*********************************/

void Ranger_Liste_Avions_A(Liste_avions La, couple *Affiche, int taille, int mode) {
	int i;
	int sauv=mode;
	for(i=0;La!=NULL; La=La->suivant_attente, i++) {
	switch(La->avion.mode) {
	case 4 : mode=4; break; /*on ne re-initialise pas le mode si en Urgence*/
	case 5 : mode=5; break; /*on ne re-initialise pas le mode si en Urgence*/
	case 6 : mode=6; break; /*on ne re-initialise pas le mode si en Liste Noire*/
	default : mode=sauv; break; /*car mode a pu être modifié au cours de la boucle*/
	}
	if (i<taille) Init_Graph_Avion(&La->avion, 0, -100, Affiche[i].x, Affiche[i].y, mode);
	else Init_Graph_Avion(&La->avion, 0, -100, -100, -100, mode); /*hors de vision*/
	}
	}

/************************Ranger_Liste_Avions_Q*********************************/

void Ranger_Liste_Avions_Q(Liste_avions La, Parking *Affiche, int taille, int mode, couple *Graph, char *TabColor[], decors PisteQ, MlvType *X_var) {
	printf("Ranger_Liste_Avions_Q \n");
	int i,j,L,k;
	couple dehors={-100,-100};
	Liste_avions LaTmp;
	if (La==NULL) return;
	for(LaTmp=La,L=0;LaTmp!=NULL;LaTmp=LaTmp->suivant_attente,L++); /*car la liste est à l'envers*/
	
	for(i=0;i<taille;i++) {
	for(LaTmp=La,j=1;j<L-i;LaTmp=LaTmp->suivant_attente, j++);	
	if (norme2(moinsvect(LaTmp->avion.centre,dehors))<2) {
	
	switch(Affiche[taille-1].etat) {
	
	case 0 :
	Init_Graph_Avion(&LaTmp->avion, -100, 0, Affiche[taille-1].place.x, Affiche[taille-1].place.y, mode);
	Affiche[taille-1].etat=1;
			
	for (k=taille-2;k>=0 && Affiche[k].etat==0;k--) {
		Cinematique_Avion(&LaTmp->avion,1, La, 5,  Graph, TabColor, Affiche[k].place, PisteQ, X_var,0);
		Init_Graph_Avion(&LaTmp->avion, -100, 0, Affiche[k].place.x, Affiche[k].place.y, mode);
		Affiche[k+1].etat=0;
		Affiche[k].etat=1;
		}
	
	break;	
	
	case 1 :  return;
	}
	
	}
	else {
	for (j=0; j<taille && norme2(moinsvect(LaTmp->avion.centre,Affiche[j].place))<2 ;j++);
	for (k=j-1;k>=0 && Affiche[k].etat==0;k--) {
		Cinematique_Avion(&LaTmp->avion,1, La, 5, Graph, TabColor, Affiche[k].place, PisteQ, X_var,0);
		Init_Graph_Avion(&LaTmp->avion, -100, 0,Affiche[k].place.x, Affiche[k].place.y, mode);
		Affiche[k+1].etat=0;
		Affiche[k].etat=1;
	}
	}
	
	}
	}

/************************Ranger_Liste_Avions_E*********************************/
	
void Ranger_Liste_Avions_E(Liste_avions La, Parking *Affiche, int taille, int mode) {
	printf("Ranger_Liste_Avions_E \n");
	int i=0;
	couple dehors={-100,-100};
	while(La!=NULL) {
	if (i<taille) {
	switch (Affiche[i].etat) {
		case 0 :
			if (norme2(moinsvect(La->avion.centre,dehors))<2) {
			Init_Graph_Avion(&La->avion, 0, -100, Affiche[i].place.x, Affiche[i].place.y, mode);
			La=La->suivant_attente;
			Affiche[i].etat=1;
			i++;
			}
			else La=La->suivant_attente;
			break;
		
		case 1 :
			i++;
			break;
		}
	}
	else {
	if (abs(La->avion.centre.y-Affiche[0].place.y)> 5) Init_Graph_Avion(&La->avion, 0, -100, -100, -100, mode);
	La=La->suivant_attente;
	}
	}
	}

/**************************Libere_Place_Affichage******************************/


void Libere_Place_Affichage(Avion a_indice, Parking *Affiche, int taille) {
	int i;
	for (i=0; i<taille && norme2(moinsvect(a_indice.centre, Affiche[i].place))>2; i++);
	if (i<taille) Affiche[i].etat=0;
	}


/******************************Mettre_En_Queue*********************************/

void Mettre_En_Queue(Liste_avions *PtrLem, Liste_avions AutreLem, Parking *AfficheE, Queue *PtrFq, Horloge Montre, int mode, int Nb_Max, couple *Graph, char *TabColor[], decors Embarq, MlvStruct *x_var) {
	printf("Mettre_En_Queue \n");
	Liste_avions LaTmp;
	Liste_avions LaTmp_passage;
	for(LaTmp=*PtrLem;LaTmp!=NULL;)
		if(Determine_Retard(Montre, LaTmp->avion.heure_decollage)==-1) {
			if (PtrFq->dernier==NULL) {
			LaTmp_passage=LaTmp->suivant_attente;

			Libere_Place_Affichage(LaTmp->avion, AfficheE, 9);
			
couple direction1={LaTmp->avion.centre.x, 0.5*(Embarq.cadre[0].By+Embarq.cadre[0].Hy)};
Cinematique_Avion(&LaTmp->avion, 1, *PtrLem, Nb_Max, Graph, TabColor, direction1, Embarq, x_var, 1, AutreLem);
couple direction2= {Embarq.cadre[0].Gx+0.95*(Embarq.cadre[0].Dx-Embarq.cadre[0].Gx), LaTmp->avion.centre.y};
Cinematique_Avion(&LaTmp->avion, 1, *PtrLem, Nb_Max, Graph, TabColor, direction2, Embarq, x_var, 1, AutreLem);
			
			PtrFq->dernier=Supprime_Avion_Attente(PtrLem, LaTmp->avion);
			LaTmp->avion.centre.x=-100;
			LaTmp->avion.centre.y=-100;
			PtrFq->premier=PtrFq->dernier;
			LaTmp=LaTmp_passage;
			}
			else {
			LaTmp_passage=LaTmp->suivant_attente;

			Libere_Place_Affichage(LaTmp->avion, AfficheE, 9);
						
couple direction1={LaTmp->avion.centre.x, 0.5*(Embarq.cadre[0].By+Embarq.cadre[0].Hy)};
Cinematique_Avion(&LaTmp->avion, 1, *PtrLem, Nb_Max, Graph, TabColor, direction1, Embarq, x_var, 1, AutreLem);
couple direction2= {Embarq.cadre[1].Gx+0.95*(Embarq.cadre[0].Dx-Embarq.cadre[0].Gx), LaTmp->avion.centre.y};
Cinematique_Avion(&LaTmp->avion, 1, *PtrLem, Nb_Max, Graph, TabColor, direction2, Embarq, x_var, 1, AutreLem);

			Change_avion_Liste_Debut(LaTmp->avion, PtrLem, &PtrFq->dernier, mode);
			LaTmp->avion.centre.x=-100;
			LaTmp->avion.centre.y=-100;
			LaTmp=LaTmp_passage;
			}
		}
		else LaTmp=LaTmp->suivant_attente;
	}

/************************Affiche_Evenements_Suivants1**************************/
	
void Affiche_Evenements_Suivants1(decors *LeDecors , Queue *Fq, Liste_avions Lat, Liste_avions Lem,  int decal) {
	printf("Affiche_Evenements_Suivants1 \n");
	int i,j,L;
	int cpt;
	int sortir=0;
	int passage=0;
	int passage2=0;
	Liste_avions LaTmp;	
	Liste_avions LaTmp2;
	int Choix;
	
	char *messageNULL= "                     ";
	int i_ev;
	for (i_ev=12; i_ev<19; i_ev++) strncpy(LeDecors->nom[i_ev]+decal, messageNULL, 21);
	
	for(LaTmp=Fq->dernier,L=0;LaTmp!=NULL;LaTmp=LaTmp->suivant_attente,L++);
	for(i=0, cpt=12, LaTmp=Lat; cpt<19;) {
	
	if (LaTmp!=NULL) Choix=LaTmp->avion.mode;
	else {
	if (Fq->premier==NULL) break;
	else Choix=2;
	}
	
	switch (Choix) {
	
	case 4 : 
		strncpy(LeDecors->nom[cpt]+decal, LaTmp->avion.info, 21); 
		cpt++;
		LaTmp=LaTmp->suivant_attente;
		break;
			
	case 6 : 	
		if (passage) {
		LaTmp=LaTmp->suivant_attente;
		break;
		}
		else {
		strncpy(LeDecors->nom[cpt]+decal, LaTmp->avion.info, 21); 
		cpt++;
		LaTmp=LaTmp->suivant_attente;
		}
		break;
	
	case 2 : 		
		if(!passage) {
		for(LaTmp2=LaTmp; LaTmp2!=NULL; LaTmp2=LaTmp2->suivant_attente) 
		if(LaTmp2->avion.mode==6 && cpt<19) {
		strncpy(LeDecors->nom[cpt]+decal, LaTmp2->avion.info, 21);
		cpt++;
		LaTmp=LaTmp->suivant_attente;
		}
		passage=1;
		}	
		else {	
		if (passage2 || Fq->premier==NULL) {
		if (passage2 && LaTmp==NULL) {
		sortir=1;
		break;
		}
		strncpy(LeDecors->nom[cpt]+decal, LaTmp->avion.info, 21);
		cpt++;
		LaTmp=LaTmp->suivant_attente;
		}
		else {
		for(LaTmp2=Fq->dernier,j=1;j<L-i;LaTmp2=LaTmp2->suivant_attente, j++);	
		if (L-i<1) {
		passage2=1;
		break;
		}
		 /*si cpt=12, cpt-1 =11 et LeDecors->nom[11] existe : c'est l'horloge et on aura donc tjrs strcmp!=0*/
		if (strcmp(LeDecors->nom[cpt-1]+decal ,LaTmp2->avion.info)!=0 ) {
		strncpy(LeDecors->nom[cpt]+decal, LaTmp2->avion.info, 21);
		}
		cpt++;
		i++;
		}
		}
		break;
		}
	if (sortir) break;
	}

	if (cpt>=19 || Lem==NULL) return;

	for(L=0, LaTmp=Lem; LaTmp!=NULL; L++, LaTmp=LaTmp->suivant_attente);
	char **Chaine;
	Chaine=(char **)malloc((L)*sizeof(char *));	
	if (Chaine==NULL) {
	fprintf(stderr,"Erreur d'allocation d'espace mémoire\n");
	exit (1);
	}
	for(i=0;i<L;i++) {
	Chaine[i]=(char *)malloc(22*sizeof(char));	
	if (Chaine[i]==NULL) {
	fprintf(stderr,"Erreur d'allocation d'espace mémoire\n");
	exit (1);
	}
	}
	for(i=0, LaTmp=Lem;LaTmp!=NULL && i<L; i++, LaTmp=LaTmp->suivant_attente) 
	strncpy(Chaine[i], LaTmp->avion.info, 21);

	Horloge HorMinPrecedent;

	for(j=cpt;j<19;j++) {

	for(i=-1, HorMinPrecedent.Heure=98, HorMinPrecedent.Minutes=98, i_ev=L-1; i_ev>=0 ; i_ev--) {
	if (Determine_Retard(HorMinPrecedent, Chaine[i_ev]+9)==-1) {
	i=i_ev;
	HorMinPrecedent.Heure=10*(Chaine[i_ev][9]-'0')+(Chaine[i_ev][10]-'0'); 
	HorMinPrecedent.Minutes=10*(Chaine[i_ev][11]-'0')+(Chaine[i_ev][12]-'0');
	}
	}
	if (i==-1) break;
	strncpy(LeDecors->nom[j]+decal, Chaine[i], 21);
	Chaine[i][9]='0'+9;
	Chaine[i][10]='0'+9;
	Chaine[i][11]='0'+9;
	Chaine[i][12]='0'+9;
	}
	for(i=0;i<L; i++) free(Chaine[i]);
	free(Chaine);
	
	}


/************************Affiche_Evenements_Suivants2**************************/
	
void Affiche_Evenements_Suivants2(decors *LeDecors , Queue *Fq, Liste_avions Lat, Liste_avions Lem, int decal) {
	printf("Affiche_Evenements_Suivants2 \n");
	int i,j,L;
	int cpt;
	int sortir=0;
	int passage=0;
	int passage2=0;
	Liste_avions LaTmp;	
	Liste_avions LaTmp2;
	int Choix;
	
	char *messageNULL= "                     ";
	int i_ev;
	for (i_ev=12; i_ev<19; i_ev++) strncpy(LeDecors->nom[i_ev]+decal, messageNULL, 21);
	
	for(LaTmp=Fq->dernier,L=0;LaTmp!=NULL;LaTmp=LaTmp->suivant_attente,L++);
	for(i=0, cpt=12, LaTmp=Lat; cpt<19;) {
	
	if (LaTmp!=NULL) Choix=LaTmp->avion.mode;
	else {
	if (Fq->premier==NULL) break;
	else Choix=3;
	}
	
	switch (Choix) {
	
	case 5 : 
		strncpy(LeDecors->nom[cpt]+decal, LaTmp->avion.info, 21); 
		cpt++;
		LaTmp=LaTmp->suivant_attente;
		break;
			
	case 6 : 
		if (passage) {
		LaTmp=LaTmp->suivant_attente;
		break;
		} 
		else {
		strncpy(LeDecors->nom[cpt]+decal, LaTmp->avion.info, 21); 
		cpt++;
		LaTmp=LaTmp->suivant_attente;
		}
		break;
	
	case 3 : 
		if(!passage) {
		for(LaTmp2=LaTmp; LaTmp2!=NULL; LaTmp2=LaTmp2->suivant_attente) 
		if(LaTmp2->avion.mode==6 && cpt<19) {
		strncpy(LeDecors->nom[cpt]+decal, LaTmp2->avion.info, 21);
		cpt++;
		LaTmp=LaTmp->suivant_attente;
		}
		passage=1;
		}	
		else {	
		if (passage2 || Fq->premier==NULL) {
		if (passage2 && LaTmp==NULL) {
		sortir=1;
		break;
		}
		strncpy(LeDecors->nom[cpt]+decal, LaTmp->avion.info, 21);
		cpt++;
		LaTmp=LaTmp->suivant_attente;
		}
		else {
		for(LaTmp2=Fq->dernier,j=1;j<L-i;LaTmp2=LaTmp2->suivant_attente, j++);	
		if (L-i<1) {
		passage2=1;
		break;
		}
		if (strcmp(LeDecors->nom[cpt-1]+decal ,LaTmp2->avion.info)!=0 )
		strncpy(LeDecors->nom[cpt]+decal, LaTmp2->avion.info, 21);
		cpt++;
		i++;
		}
		}
		break;
		}
	if (sortir) break;
	}
	
	if (cpt>=19 || Lem==NULL) return;
	
	for(L=0, LaTmp=Lem; LaTmp!=NULL; L++, LaTmp=LaTmp->suivant_attente);
	char **Chaine;
	Chaine=(char **)malloc((L)*sizeof(char *));	
	if (Chaine==NULL) {
	fprintf(stderr,"Erreur d'allocation d'espace mémoire\n");
	exit (1);
	}
	for(i=0;i<L;i++) {
	Chaine[i]=(char *)malloc(22*sizeof(char));	
	if (Chaine[i]==NULL) {
	fprintf(stderr,"Erreur d'allocation d'espace mémoire\n");
	exit (1);
	}
	}
	for(i=0, LaTmp=Lem;LaTmp!=NULL && i<L; i++, LaTmp=LaTmp->suivant_attente) 
	strncpy(Chaine[i], LaTmp->avion.info, 21);

	Horloge HorMinPrecedent;
	
	for(j=cpt;j<19;j++) {
	for(i=-1, HorMinPrecedent.Heure=98, HorMinPrecedent.Minutes=98, i_ev=L-1; i_ev>=0 ; i_ev--) {
	if (Determine_Retard(HorMinPrecedent, Chaine[i_ev]+9)==-1) {
	i=i_ev;
	HorMinPrecedent.Heure=10*(Chaine[i_ev][9]-'0')+(Chaine[i_ev][10]-'0'); 
	HorMinPrecedent.Minutes=10*(Chaine[i_ev][11]-'0')+(Chaine[i_ev][12]-'0');
	}
	}
	if (i==-1) break;
	strncpy(LeDecors->nom[j]+decal, Chaine[i], 21);
	Chaine[i][9]='0'+9;
	Chaine[i][10]='0'+9;
	Chaine[i][11]='0'+9;
	Chaine[i][12]='0'+9;
	}
	for(i=0;i<L; i++) free(Chaine[i]);
	free(Chaine);
	
	
	}
	
/******************************************************************************/

/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
/*FIN DU CHAPITRE II*/
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
/*CHAPITRE III : LA FONCTION PRINCIPALE ("MAIN")*/
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

/*Mettre en argument le nom du programme puis le nom du fichier de sauvegarde*/

int main(int argc, char *argv[]) {
	
	int test=-1;
	
	if (argv[2]==NULL || argv[1]==NULL) {
	printf("Rappel: la commande de lancement du programme doit se faire sous le format suivant :\n");
	printf("[Nom du programme] [Nom du ficher de sauvegarde] [Mode de jeu: test/normal]\n");
	return 1;
	}	
	
	if (strcmp("test",argv[2])==0) test=1;
	if (strcmp("normal",argv[2])==0) test=0;
	if (test==-1) {
	printf("Rappel: la commande de lancement du programme doit se faire sous le format suivant :\n");
	printf("[Nom du programme] [Nom du ficher de sauvegarde] [Mode de jeu: test/normal]\n");
	return 1;
	}
		
	FILE *Sauvegarde;
	
	char rep[8];
	int option=0;
	if ((Sauvegarde=fopen(argv[1],"a+"))==NULL) {
	fprintf(stderr,"Erreur d'ouverture de fichier\n");
	return 1;
	}
	fseek(Sauvegarde, 0, SEEK_END);
	
	if (ftell(Sauvegarde)!=0) {
	do {
	printf("Le fichier %s existe deja, que voulez vous entreprendre? (ecraser/etendre/quitter)\n", argv[1]);
	scanf("%s",rep);
	} while(strcmp("ecraser",rep)!=0 && strcmp("etendre",rep)!=0 && strcmp("quitter",rep)!=0);
	if (strcmp("quitter",rep)==0) return 1;
	if (strcmp("etendre",rep)==0) option=1;
	if (strcmp("ecraser",rep)==0) option=2;
	}
	if (option==0 || option==1) fclose(Sauvegarde);
	if (option==2) {
	fclose(Sauvegarde);
	if ((Sauvegarde=fopen(argv[1],"w+"))==NULL) {
	fprintf(stderr,"Erreur d'ouverture de fichier\n");
	return 1;
	}
	fclose(Sauvegarde);
	}
	
	
	int width = W;
	int hight = H;
	char *display_name=NULL;
		
	MlvType *X_piste;
	X_piste=(MlvType *)malloc(sizeof(MlvType));
	if (X_piste==NULL) return 1;
	create_window (display_name, width, hight, X_piste);
	name_window ("SIMULATION DU TRAFIC D'UN AEROPORT",X_piste);
	
	Horloge Montre={6,30}; /*on commence à 6:30 du matin*/

	int Round=0;
	

	int debut=0; /*pour commencer le jeu*/
	int fin=0; /*pour quitter le jeu*/
	int aleat=0; /*pour le generateur aleatoire*/	
	
	MlvType *X_menu;
	Compagnie *CompTmp;
	Avion *AvionTmp;
	Liste_compagnie LcTmp;
	Liste_avions LaTmp;
	
	int Choix;
	
	char mode;
	
	/*l'indice 1 ou 2 indique le numero de piste*/
	Liste_compagnie Lc=NULL; /*LISTE COMPAGNIE*/ 
	Liste_compagnie Ln=NULL; /*LISTE NOIRE*/ 
	Liste_avions Lat1=NULL; /*LISTE ATTERRISSAGE PISTE 1*/ 
	Liste_avions Lem1=NULL; /*LISTE EMBARQUEMENT PISTE 1*/ 
	Liste_avions Lat2=NULL; /*LISTE ATTERRISSAGE PISTE 2*/
	Liste_avions Lem2=NULL; /*LISTE EMBARQUEMENT PISTE 2*/ 
	Liste_avions Li=NULL; /*UNE AMELIORATION DE MA PART : LISTE DES INOCCUPÉS*/ 
	Queue Fq1; Fq1.premier=NULL; Fq1.dernier=NULL; /*SUR LA PISTE 1*/ 
	Queue Fq2; Fq2.premier=NULL; Fq2.dernier=NULL; /*SUR LA PISTE 2*/
		
	int Num_Piste=1;
	
	int r=-1;
	int x_souris, y_souris; /*Trés utilisés!!*/
	float x_precision, y_precision; /*car Valide_Clic est construite avec des float*/

	couple direction;

	char *TabColor[8]={"gray","gray","gray","gray","red","red","black","gray"};
	float a1=0.0, a2=0.1500, a3=0.4200, a4=1.9547, a5=2.0420, a6=2.3561, a7=3.07,a8=2.9480, dpi=6.2831,pi=3.1415;
 
	/*Ici on fonctionne en coordonnées polaires: Graph.x =longeur, Graph.y = rad*/
	couple Graph[NPointAvion] = {{16.5,a1},{13.0,a2},{6.0,a3},{20.0,a4},{22.0,a5},{3.0,a6},{16.0,a7},{21.0,a8},{19.5,pi},{21.0,dpi-a8},{16.0,dpi-a7},{3.0,dpi-a6},{22.0,dpi-a5},{20.0,dpi-a4},{6.0,dpi-a3},{13.0,dpi-a2},{16.5,dpi-a1}};

	float Dbord = dim_w(0.008), Dgtab=dim_w(0.75);
	float sep1=dim_h(0.3), sep2=sep1+Dbord+dim_h(0.15), sep3=sep2+Dbord+dim_h(0.15);
	float Dge=dim_w(0.75),Dde=dim_w(0.99);
	float Dbe=dim_h(0.1), Die=dim_h(0.05), Dbeh=dim_h(0.46);
	float DgB=Dge+dim_w(0.005),DhB=dim_h(0.7);
	float decW1=dim_w(0.004901), decW2=dim_w(0.003267),decH=dim_h(0.008333);
	float buzw =dim_w(0.058823), buzh = dim_h(0.0333333);

decors LeDecors={	34,

			{
			{ Dbord , Dbord , Dgtab , sep1 },/*ciel*/
			{ Dbord , sep1 , Dgtab , sep1+Dbord },/*ligne separatrice 1*/
			{ Dbord , sep1+Dbord , Dgtab , sep2 },/*piste1*/
			{ Dbord , sep2 , Dgtab , sep2+Dbord },/*ligne separatrice 2*/
			{ Dbord , sep2+Dbord , Dgtab , sep3 },/*piste2*/
			{ Dbord , sep3 , Dgtab , sep3+Dbord },/*ligne separatrice 3*/
			{ Dbord , sep3+Dbord , Dgtab , dim_h(0.95) },/*embarquement*/
			{ dim_w(0.0) , dim_h(0.95) , Dgtab , dim_h(1.0) },/*message*/
			
			{ dim_w(0.0) , dim_h(0.0) , Dgtab , Dbord },/*bord haut*/
			{ dim_w(0.0) , dim_h(0.0) , Dbord ,dim_h(0.95) },/*bord gauche*/
			{ Dgtab-Dbord , dim_h(0.0) , dim_w(1.0) , dim_h(1.0) },/*tableau de bord*/
			
			{ Dge, Dbord , Dde , Dbe-Dbord },/*horloge*/
			
			{ Dge, Dbe , Dde , Dbe+Die },/*evenements à venir*/
			{ Dge, Dbe+Die , Dde , Dbe+2*Die },/*evenements à venir*/
			{ Dge, Dbe+2*Die , Dde , Dbe+3*Die },/*evenements à venir*/
			{ Dge, Dbe+3*Die , Dde , Dbe+4*Die },/*evenements à venir*/
			{ Dge, Dbe+4*Die , Dde , Dbe+5*Die },/*evenements à venir*/
			{ Dge, Dbe+5*Die , Dde , Dbe+6*Die },/*evenements à venir*/
			{ Dge, Dbe+6*Die , Dde , Dbe+7*Die },/*evenements à venir*/
			
			{ Dge, Dbeh , Dde , Dbeh+Die },/*evenements precedents*/
			{ Dge, Dbeh+Die , Dde ,Dbeh+2*Die },/*evenements precedents*/
			{ Dge, Dbeh+2*Die , Dde , Dbeh+3*Die },/*evenements precedents*/
			
			/*BOUTONS*/
			{ DgB, DhB , DgB+buzw , DhB+buzh },
			{ DgB+1.5*buzw , DhB , DgB+2.5*buzw  , DhB+buzh },
			{ DgB+3*buzw , DhB , DgB+4*buzw  , DhB+buzh },
			
			{ DgB, DhB+2*buzh , DgB+buzw , DhB+3*buzh },
			{ DgB+1.5*buzw , DhB+2*buzh , DgB+2.5*buzw  , DhB+3*buzh }, 
			{ DgB+3*buzw , DhB+2*buzh , DgB+4*buzw  , DhB+3*buzh },
			
			{ DgB, DhB+4*buzh , DgB+buzw , DhB+5*buzh },
			{ DgB+1.5*buzw , DhB+4*buzh , DgB+2.5*buzw  , DhB+5*buzh }, 
			{ DgB+3*buzw , DhB+4*buzh , DgB+4*buzw  , DhB+5*buzh }, 
			
			{ DgB, DhB+6*buzh , DgB+buzw , DhB+7*buzh },
			{ DgB+1.5*buzw , DhB+6*buzh , DgB+2.5*buzw  , DhB+7*buzh }, 
			{ DgB+3*buzw , DhB+6*buzh , DgB+4*buzw  , DhB+7*buzh }
			},
			
			{
			 "black",/*ciel*/
			 "black",/*ligne separatrice 1*/
			 "black",/*piste1*/
			 "black",/*ligne separatrice 2*/
			 "black",/*piste2*/
			 "black",/*ligne separatrice 3*/
			 "black",/*embarquement*/
			 "white",/*message*/
			 
			 "black",/*bord haut*/
			 "black",/*bord gauche*/
			 "black",/*tableau de bord*/
			 
			 "black",/*horloge*/
			 
			 "black",/*evenements à venir*/
			 "black",/*evenements à venir*/
			 "black",/*evenements à venir*/
			 "black",/*evenements à venir*/
			 "black",/*evenements à venir*/
			 "black",/*evenements à venir*/
			 "black",/*evenements à venir*/
			 
			 "black",/*evenements precedents*/
			 "black",/*evenements precedents*/
			 "black",/*evenements precedents*/
			 
			 "black",
			 "black",
			 "black",
			 "black",
			 "black",
			 "black",
			 "black",
			 "black",
			 "black",
			 "black",
			 "black",
			 "black"
			},
			
			{ 
			 
			 "LightSkyBlue",/*ciel*/
			 "black",/*ligne separatrice 1*/
			 "LightSlateGrey",/*piste1*/
			 "black",/*ligne separatrice 2*/
			 "LightSlateGrey",/*piste2*/
			 "black",/*ligne separatrice 3*/
			 "LightSlateGrey",/*embarquement*/
			 "black",/*message*/
			 
			 "black",/*bord haut*/
			 "black",/*bord gauche*/
			 "black",/*tableau de bord*/
			 
			 "DarkOliveGreen3",/*horloge*/
			 
			 "khaki2",/*evenements à venir*/
			 "khaki2",/*evenements à venir*/
			 "khaki2",/*evenements à venir*/
			 "khaki2",/*evenements à venir*/
			 "khaki2",/*evenements à venir*/
			 "khaki2",/*evenements à venir*/
			 "khaki2",/*evenements à venir*/
			 
			 "khaki3",/*evenements precedents*/
			 "khaki3",/*evenements precedents*/
			 "khaki3",/*evenements precedents*/
			 
			 "chocolate",
			 "chocolate",
			 "chocolate",
			 "chocolate",
			 "chocolate",
			 "chocolate",
			 "chocolate",
			 "chocolate",
			 "chocolate",
			 "chocolate",
			 "chocolate",
			 "chocolate"
			 },
			
			{
			 "10x20",/*ciel*/
			 "10x20",/*ligne separatrice 1*/
			 "10x20",/*piste1*/
			 "10x20",/*ligne separatrice 2*/
			 "10x20",/*piste2*/
			 "10x20",/*ligne separatrice 3*/
			 "10x20",/*embarquement*/
			 "10x20",/*message*/
			 
			 "10x20",/*bord haut*/
			 "10x20",/*bord gauche*/
			 "10x20",/*tableau de bord*/
			 
			 "10x20",/*horloge*/
			 
			 "5x8",/*evenements à venir*/
			 "5x8",/*evenements à venir*/
			 "5x8",/*evenements à venir*/
			 "5x8",/*evenements à venir*/
			 "5x8",/*evenements à venir*/
			 "5x8",/*evenements à venir*/
			 "5x8",/*evenements à venir*/
			 
			 "5x8",/*evenements precedents*/
			 "5x8",/*evenements precedents*/
			 "5x8",/*evenements precedents*/
			 			 
			 "6x13",
			 "6x13",
			 "6x13",
			 "6x13",
			 "6x13",
			 "6x13",
			 "6x13",
			 "6x13",
			 "6x13",
			 "6x13",
			 "6x13",
			 "6x13"
			 },
					
			{
			 "",/*ciel*/
			 "",/*ligne separatrice 1*/
			 "",/*piste1*/
			 "",/*ligne separatrice 2*/
			 "",/*piste2*/
			 "",/*ligne separatrice 3*/
			 "",/*embarquement*/
			 "",/*message*/
			 
			 "",/*bord haut*/
			 "",/*bord gauche*/
			 "",/*tableau de bord*/
			 
			 "",/*horloge*/
			 
			 "",/*evenements à venir*/
			 "",/*evenements à venir*/
			 "",/*evenements à venir*/
			 "",/*evenements à venir*/
			 "",/*evenements à venir*/
			 "",/*evenements à venir*/
			 "",/*evenements à venir*/
			 
			 "",/*evenements precedents*/
			 "",/*evenements precedents*/
			 "",/*evenements precedents*/
			 
			 "COMPAGNIE",/*AJOUT d'une compagnie*/
			 "DEC. AV.",/*AJOUT d'un avion au decollage*/
			 "ATT. AV.",/*AJOUT d'un avion a l'atterrissage*/
			 "GARAGE",/*SUPPRESSION d'une avion au décolage*/
			 "URGENCE",/*CHANGE LA PRIORITÉ par mesure d'urgence*/
			 "Lst N.",/*met une compagnie sur LISTE NOIRE*/
			 "ALEAT.",/*decision aléatoire*/
			 "STATUT",/*DEMANDER LE STATUT d'une compagnie*/
			 "HISTOR.",/*HISTORIQUE*/
			 "DEBUTER",/*COMMENCER*/
			 "QUITTER",/*QUITTER*/
			 "PASSER"/*ne rien faire*/
			 },
			 
			 {
			  {0,0},/*ciel*/
			  {0,0},/*ligne separatrice 1*/
			  {0,0},/*piste1*/
			  {0,0},/*ligne separatrice 2*/
			  {0,0},/*piste2*/
			  {0,0},/*ligne separatrice 3*/
			  {0,0},/*embarquement*/
			  {dim_w(0.01),dim_h(0.015)},/*message*/
			  
			  {0,0},/*bord haut*/
			  {0,0},/*bord gauche*/
			  {0,0},/*tableau de bord*/
			   
			  {dim_w(0.1),dim_h(0.025)},/*horloge*/
			  
			  {decW1,decH},/*evenements à venir*/
			  {decW1,decH},/*evenements à venir*/
			  {decW1,decH},/*evenements à venir*/
			  {decW1,decH},/*evenements à venir*/
			  {decW1,decH},/*evenements à venir*/
			  {decW1,decH},/*evenements à venir*/
			  {decW1,decH},/*evenements à venir*/
			  
			  {decW1,decH},/*evenements precedents*/
			  {decW1,decH},/*evenements precedents*/
			  {decW1,decH},/*evenements precedents*/
			  
			  {decW2,decH},
			  {decW2,decH},
			  {decW2,decH},
			  {decW2,decH},
			  {decW2,decH},
			  {decW2,decH},
			  {decW2,decH},
			  {decW2,decH},
			  {decW2,decH},
			  {decW2,decH},
			  {decW2,decH},
			  {decW2,decH},
			 }
			};


decors LaPiste1={	3,
			
			{
			{ Dbord , sep1 , Dgtab-Dbord-1 , sep1+Dbord },/*ligne separatrice 1*/
			{ Dbord , sep1+Dbord , Dgtab-Dbord-1 , sep2 },/*piste1*/
			{ Dbord , sep2 , Dgtab-Dbord-1 , sep2/*+Dbord*/ }/*ligne separatrice 2*/
			},
			
			{
			 "black",/*ligne separatrice 1*/
			 "black",/*piste1*/
			 "black"/*ligne separatrice 2*/
			},
			
			{ 
			 "black",/*ligne separatrice 1*/
			 "LightSlateGrey",/*piste1*/
			 "black"/*ligne separatrice 2*/
			 },
			
			{
			 "10x20",/*ligne separatrice 1*/
			 "10x20",/*piste1*/
			 "10x20"/*ligne separatrice 2*/
			 },
					
			{
			 "",/*ligne separatrice 1*/
			 "",/*piste1*/
			 ""/*ligne separatrice 2*/
			 },
			 
			 {
			  {0,0},/*ligne separatrice 1*/
			  {0,0},/*piste1*/
			  {0,0}/*ligne separatrice 2*/
			 }
			};


decors LaPiste2={	3,
			
			{
			{ Dbord , sep2 , Dgtab-Dbord-1 , sep2+Dbord },/*ligne separatrice 2*/
			{ Dbord , sep2+Dbord , Dgtab-Dbord-1 , sep3 },/*piste2*/
			{ Dbord , sep3 , Dgtab-Dbord-1 , sep3/*+Dbord*/ }/*ligne separatrice 3*/
			},
			
			{
			 "black",/*ligne separatrice 2*/
			 "black",/*piste2*/
			 "black"/*ligne separatrice 3*/
			},
			
			{ 
			 "black",/*ligne separatrice 2*/
			 "LightSlateGrey",/*piste2*/
			 "black"/*ligne separatrice 3*/
			 },
			
			{
			 "10x20",/*ligne separatrice 2*/
			 "10x20",/*piste2*/
			 "10x20"/*ligne separatrice 3*/
			 },
					
			{
			 "",/*ligne separatrice 2*/
			 "",/*piste2*/
			 ""/*ligne separatrice 3*/
			 },
			 
			 {
			  {0,0},/*ligne separatrice 2*/
			  {0,0},/*piste2*/
			  {0,0}/*ligne separatrice 3*/
			 }
			};


decors Embarq={	2,

			{
			{ Dbord , sep3+Dbord , Dgtab-Dbord-1 , dim_h(0.95) },/*embarquement*/
			{ dim_w(0.0) , dim_h(0.95) , Dgtab , dim_h(1.0) },/*message*/
			},
			
			{
			 "black",/*embarquement*/
			 "white",/*message*/			 
			},
			
			{ 	 
			 "LightSlateGrey",/*embarquement*/
			 "black",/*message*/
			  },
			
			{
			 "10x20",/*embarquement*/
			 "10x20",/*message*/
			 },
					
			{
			 "",/*embarquement*/
			 "MESSAGE : Passage des avions devant decoller en bout de piste." ,/*message*/
			 },
			 
			 {
			  {0,0},/*embarquement*/
			  {dim_w(0.01),dim_h(0.015)},/*message*/
			 }
			};

rectangle Zone_Boutons[12]={
			{ DgB, DhB , DgB+buzw , DhB+buzh },
			{ DgB+1.5*buzw , DhB , DgB+2.5*buzw  , DhB+buzh },
			{ DgB+3*buzw , DhB , DgB+4*buzw  , DhB+buzh },
			
			{ DgB, DhB+2*buzh , DgB+buzw , DhB+3*buzh },
			{ DgB+1.5*buzw , DhB+2*buzh , DgB+2.5*buzw  , DhB+3*buzh }, 
			{ DgB+3*buzw , DhB+2*buzh , DgB+4*buzw  , DhB+3*buzh },
			
			{ DgB, DhB+4*buzh , DgB+buzw , DhB+5*buzh },
			{ DgB+1.5*buzw , DhB+4*buzh , DgB+2.5*buzw  , DhB+5*buzh }, 
			{ DgB+3*buzw , DhB+4*buzh , DgB+4*buzw  , DhB+5*buzh }, 
			
			{ DgB, DhB+6*buzh , DgB+buzw , DhB+7*buzh },
			{ DgB+1.5*buzw , DhB+6*buzh , DgB+2.5*buzw  , DhB+7*buzh }, 
			{ DgB+3*buzw , DhB+6*buzh , DgB+4*buzw  , DhB+7*buzh }
			};

int Nbr_Boutons=12;

couple AfficheA1[9];
couple AfficheA2[9];
Parking AfficheE1[9];
Parking AfficheE2[9];
Parking AfficheQ1[5];
Parking AfficheQ2[5];

Remplir_Affiche_A (AfficheA1, 9, Dbord, sep1, Dgtab, Dbord);
Remplir_Affiche_A (AfficheA2, 9, Dbord, Dbord, Dgtab, sep1);
Remplir_Affiche_D (AfficheQ1, 5, (int)(0.5*(Dbord+Dgtab)), sep2, Dgtab, sep1+Dbord);
Remplir_Affiche_D (AfficheQ2, 5, (int)(0.5*(Dbord+Dgtab)), sep3, Dgtab, sep2+Dbord);
Remplir_Affiche_D (AfficheE1, 9, Dbord, dim_h(0.95), Dgtab, Dbord+sep3);
Remplir_Affiche_D (AfficheE2, 9, Dbord, Dbord+sep3, Dgtab, dim_h(0.95));


Parking *Park[4]={AfficheE1, AfficheE2, AfficheQ1, AfficheQ2};

char *message01= "MESSAGE : Cliquer sur DEBUTER pour commencer.";
char *message02= "MESSAGE : Vous pouvez Jouer.";
char *message03= "MESSAGE : Veuillez patienter : un evenement se produit.";


char *messageNULL= "                     ";
char infoPiste1[22], infoPiste2[22];
int compteur,ecart=4;
int i_ev;

for (i_ev=12; i_ev<22;i_ev++) {
LeDecors.nom[i_ev]=(char *)malloc((21+ecart+21+1)*sizeof(char));
if (LeDecors.nom[i_ev]==NULL) exit(1);

/*strncpy(LeDecors.nom[i_ev], messageNULL,21);
strncpy(LeDecors.nom[i_ev]+ecart+21, messageNULL,21);*/

for(compteur=0;compteur<21+ecart+21;compteur++)  LeDecors.nom[i_ev][compteur]=' '; LeDecors.nom[i_ev][compteur]='\0';

}

strcpy(infoPiste1, messageNULL);
strcpy(infoPiste2, messageNULL);

char HorlogeTmp[6];

do { /*la grande boucle du jeu*/

/*on affiche l'heure*/	
	sprintf(HorlogeTmp,"%2d:%2d",Montre.Heure,Montre.Minutes);
	Completion_Numerique(Montre.Heure,2,HorlogeTmp);
	Completion_Numerique(Montre.Minutes,2,HorlogeTmp+3);
	LeDecors.nom[11]=HorlogeTmp;
	
	if (debut==0) {
	LeDecors.nom[7]=message01;
	Affiche_Decors(LeDecors, X_piste); 
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);	
	}
	else {
	LeDecors.nom[7]=message02;
	Affiche_Decors(LeDecors, X_piste); 
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	}
	
	
	if (aleat) {
	r=1+random(6);
	aleat=0;
	}
	else do {
		r=Valide_Clic(&x_precision,&y_precision,Zone_Boutons,Nbr_Boutons,X_piste);
		} while(r<1 && r>12);

/*r va constituer un indicateur pour savoir si qqch a été choisi: si r=-1, l'utilIsateur doit rejouer*/
	switch(r) 
	{




/*********************DEBUT AJOUT D'UNE COMPAGNIE******************************/
case 1 :	
	if (!test) {
	if (!debut) break;
	}
	
	X_menu=(MlvType *)malloc(sizeof(MlvType));
	if (X_menu==NULL) return 1;
	create_window (display_name, width, hight, X_menu);
	name_window ("AJOUT D'UNE COMPAGNIE",X_menu);

	CompTmp=Creer_Compagnie(X_menu);
	if (CompTmp->nom==NULL) {
	r=-1;
	draw_filled_rectangle(0,0,W,H,"white", X_menu); /*on efface ce qu'il y avait d'écrit*/
	draw_string(20,20,"Vous n'avez effectue aucune action.","10x20","black", X_menu);
	draw_string(20,40,"Cliquez n'importe ou dans la fenetre pour quitter.","10x20","black", X_menu);
	display_window(X_menu);
	mouse_wait(&x_souris,&y_souris, X_menu);
	}
	else {
	if((LcTmp=Recherche_Compagnie(Ln, *CompTmp))!=NULL) {
	r=-1;
	draw_filled_rectangle(0,0,W,H,"white", X_menu); /*on efface ce qu'il y avait d'écrit*/
	draw_string(20,20,"Cette compagnie existe deja dans la Liste Noire: veuillez en creer une autre SVP.","10x20","black", X_menu);
	draw_string(20,40,"Cliquez n'importe ou dans la fenetre pour quitter.","10x20","black", X_menu);
	display_window(X_menu);
	mouse_wait(&x_souris,&y_souris, X_menu);
	}
	else {
	if((LcTmp=Insere_Fin_Liste_compagnie(&Lc,*CompTmp))==NULL) {
	r=-1;
	draw_filled_rectangle(0,0,W,H,"white", X_menu); /*on efface ce qu'il y avait d'écrit*/
	draw_string(20,20,"Cette compagnie existe deja: veuillez en creer une autre SVP.","10x20","black", X_menu);
	draw_string(20,40,"Cliquez n'importe ou dans la fenetre pour quitter.","10x20","black", X_menu);
	display_window(X_menu);
	mouse_wait(&x_souris,&y_souris, X_menu);
	}
	else;
	}
	}
	/*Terminé!*/
		
	close_window(X_piste);
	free(X_piste);
	X_piste=(MlvType *)malloc(sizeof(MlvType));
	if (X_piste==NULL) return 1;
	create_window (display_name, width, hight, X_piste);
	name_window ("SIMULATION DU TRAFIC D'UN AEROPORT",X_piste);
	
	Affiche_Decors(LeDecors, X_piste); 
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	close_window(X_menu);
	free(X_menu);	
	break;
/***********************FIN AJOUT D'UNE COMPAGNIE******************************/



/*******************DEBUT AJOUT D'UN AVION AU DECOLLAGE************************/
case 2 :	
	
	if (!test) {
	if (!debut) break;
	}
	
	mode='d';
	
	X_menu=(MlvType *)malloc(sizeof(MlvType));
	if (X_menu==NULL) return 1;
	create_window (display_name, width, hight, X_menu);
	name_window ("AJOUT D'UN AVION AU DECOLLAGE",X_menu);
	
	/*Tout dabord on choisit la compagnie à laquelle on veut affilier l'avion */	
	LcTmp=Choisir_Comp(20,"Pour choisir la compagnie de l'avion :", Lc, X_menu);
	if (LcTmp==NULL) { /*s'il n'y a rien dans la liste_compagnie*/
	r=-1;
	draw_filled_rectangle(0,0,W,H,"white", X_menu); /*on efface ce qu'il y avait d'écrit*/
	draw_string(20,20,"Creer une compagnie SVP.","10x20","black", X_menu);
	draw_string(20,40,"Cliquez n'importe ou dans la fenetre pour quitter.","10x20","black", X_menu);
	display_window(X_menu);
	mouse_wait(&x_souris,&y_souris, X_menu);
	
	close_window(X_piste);
	free(X_piste);
	X_piste=(MlvType *)malloc(sizeof(MlvType));
	if (X_piste==NULL) return 1;
	create_window (display_name, width, hight, X_piste);
	name_window ("SIMULATION DU TRAFIC D'UN AEROPORT",X_piste);
	
	Affiche_Decors(LeDecors, X_piste); 
	
	close_window(X_menu);
	free(X_menu);
	break;
	
	}
	else { 
	/*Ensuite on regarde si on n'a pas un avion d'inoccupé de cette compagnie*/
	if ((LaTmp=Recherche_Avion_ParMode(LcTmp->comp.avions_compagnie, 7))!=NULL) {
	draw_string(20,100+20,"Des avions de cette compagnie sont inoccupes.","10x20","black", X_menu);
	display_window(X_menu);	
	LaTmp=Choisir_Avion_ParMode(120+20, "Pour choisir un avion inoccupe :", LcTmp->comp.avions_compagnie, 7, X_menu);
	draw_string(20,220+20,"Cliquez n'importe ou dans la fenetre pour continuer.","10x20","black", X_menu);
	display_window(X_menu);
	mouse_wait(&x_souris,&y_souris, X_menu);	
	Initialiser_Avion(LaTmp, mode, Montre, X_menu);
	}
	/*Sinon on en crée un*/	
	else {
	AvionTmp=Creer_Avion(LcTmp, mode, Montre, X_menu);
	if ((LaTmp=Insere_Fin_Liste_avions_Compagnie(&LcTmp->comp, *AvionTmp))==NULL) { 
	
/*ICI CA NE SERT A RIEN DE VÉRIFIER CAR IL Y A DÉJÀ UNE VERIFICATION DANS Creer_Avion*/	
	
	r=-1;
	draw_filled_rectangle(0,0,W,H,"white", X_menu); /*on efface ce qu'il y avait d'écrit*/
	draw_string(20,20,"Cet avion existe deja : veuillez en creer un autre SVP.","10x20","black", X_menu);
	draw_string(20,40,"Cliquez n'importe ou dans la fenetre pour quitter.","10x20","black", X_menu);
	display_window(X_menu);
	mouse_wait(&x_souris,&y_souris, X_menu);
	
	close_window(X_piste);
	free(X_piste);
	X_piste=(MlvType *)malloc(sizeof(MlvType));
	if (X_piste==NULL) return 1;
	create_window (display_name, width, hight, X_piste);
	name_window ("SIMULATION DU TRAFIC D'UN AEROPORT",X_piste);
	
	Affiche_Decors(LeDecors, X_piste); 
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	
	close_window(X_menu);
	free(X_menu);
	break;
	
	}
	}
	/*Dans tous les cas on a récupéré notre Cellule_avion avec LaTmp...*/
	if (r!=-1) {
	/*Enfin, on demande sur quelle piste ira l'avion*/
	draw_filled_rectangle(0,0,W,H,"white",X_menu);/*on efface ce qu'il y avait d'écrit*/
	Choisir_nbr(20,"Pour choisir le numero de la piste: ", &Num_Piste, 1, 3, 2, X_menu);
/*maintenant on peut inserer notre avion dans la "Lem1"ou "Lem2" (le delai vaut 5 donc jamais dans queue!!)*/	
	switch (Num_Piste) {
/*Ici on insère au début pour des raisons de complexité et puis tout a déjà été vérifié plus haut: il n'y a pas de pb*/
		case 1 : 
		Insere_Debut_Liste_avions_Attente(&Lem1, LaTmp); 
		LaTmp->avion.centre.x=-100;
		LaTmp->avion.centre.y=-100;
		Ranger_Liste_Avions_E(Lem1, AfficheE1, 9, 0);
		break;
		case 2 : 
		Insere_Debut_Liste_avions_Attente(&Lem2, LaTmp); 
		LaTmp->avion.centre.x=-100;
		LaTmp->avion.centre.y=-100;		
		Ranger_Liste_Avions_E(Lem2, AfficheE2, 9, 1);
		break;
		}
	sprintf(LaTmp->avion.info, "%s-D-%s-------%d\n",LaTmp->avion.identifiant ,LaTmp->avion.heure_decollage, Num_Piste);
/*et voici la sauvegarde dans l'historique comme demandé dans la consigne*/	
	if ((Sauvegarde=fopen(argv[1],"a+t"))==NULL) { /*"a+" pour se positionner en fin de fichier et "t" pour texte*/
	fprintf(stderr,"Erreur d'ouverture de fichier\n");
	return 1;
	}
	fprintf(Sauvegarde,LaTmp->avion.info);
	fclose(Sauvegarde);
	}
	}
/*Terminé! ouf...*/	
	close_window(X_piste);
	free(X_piste);
	X_piste=(MlvType *)malloc(sizeof(MlvType));
	if (X_piste==NULL) return 1;
	create_window (display_name, width, hight, X_piste);
	name_window ("SIMULATION DU TRAFIC D'UN AEROPORT",X_piste);
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	close_window(X_menu);
	free(X_menu);
	break;
/*********************FIN AJOUT D'UN AVION AU DECOLLAGE************************/	



/*****************DEBUT AJOUT D'UN AVION A L'ATTERRISSAGE**********************/
case 3 :	
	
	if (!test) {
	if (!debut) break;
	}
	
	mode='a';
	
	X_menu=(MlvType *)malloc(sizeof(MlvType));
	if (X_menu==NULL) return 1;
	create_window (display_name, width, hight, X_menu);
	name_window ("AJOUT D'UN AVION A L'ATTERRISSAGE",X_menu);
	
/*Tout dabord on choisit la compagnie à laquelle on veut affilier l'avion */	
	LcTmp=Choisir_Comp(20,"Pour choisir la compagnie de l'avion :", Lc, X_menu);
	if (LcTmp==NULL)	{ /*s'il n'y a rien dans la liste_compagnie*/
	r=-1;
	draw_filled_rectangle(0,0,W,H,"white", X_menu); /*on efface ce qu'il y avait d'écrit*/
	draw_string(20,20,"Creer une compagnie SVP.","10x20","black", X_menu);
	draw_string(20,40,"Cliquez n'importe ou dans la fenetre pour quitter.","10x20","black", X_menu);
	display_window(X_menu);
	mouse_wait(&x_souris,&y_souris, X_menu);
	
	close_window(X_piste);
	free(X_piste);
	X_piste=(MlvType *)malloc(sizeof(MlvType));
	if (X_piste==NULL) return 1;
	create_window (display_name, width, hight, X_piste);
	name_window ("SIMULATION DU TRAFIC D'UN AEROPORT",X_piste);
	
	Affiche_Decors(LeDecors, X_piste); 
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);	
	
	close_window(X_menu);
	free(X_menu);
	break;
	
	}
	else {
/*Ici on ne se pose pas de question, on crée directement un avion*/	
	AvionTmp=Creer_Avion(LcTmp, mode, Montre, X_menu);
	if ((LaTmp=Insere_Fin_Liste_avions_Compagnie(&LcTmp->comp, *AvionTmp))==NULL) { 
	r=-1;

/*ICI CA NE SERT A RIEN DE VÉRIFIER CAR IL Y A DÉJÀ UNE VERIFICATION DANS Creer_Avion*/	
	
	draw_filled_rectangle(0,0,W,H,"white", X_menu); /*on efface ce qu'il y avait d'écrit*/
	draw_string(20,20,"Cet avion existe deja : veuillez en creer un autre SVP.","10x20","black", X_menu);
	draw_string(20,40,"Cliquez n'importe ou dans la fenetre pour quitter.","10x20","black", X_menu);
	display_window(X_menu);
	mouse_wait(&x_souris,&y_souris, X_menu);
	
	close_window(X_piste);
	free(X_piste);
	X_piste=(MlvType *)malloc(sizeof(MlvType));
	if (X_piste==NULL) return 1;
	create_window (display_name, width, hight, X_piste);
	name_window ("SIMULATION DU TRAFIC D'UN AEROPORT",X_piste);
	
	Affiche_Decors(LeDecors, X_piste); 
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	close_window(X_menu);
	free(X_menu);
	break;
	
	}
/*Dans tous les cas on a récupéré notre Cellule_avion avec LaTmp...*/
	else {
/*Enfin, on demande sur quelle piste ira l'avion*/
	draw_filled_rectangle(0,0,W,H,"white",X_menu); /*on efface ce qu'il y avait d'écrit*/
	Choisir_nbr(20,"Pour choisir le numero de la piste: ", &Num_Piste, 1,3, 2, X_menu);
/*maintenant on peut inserer notre avion dans la "Lat1"ou "Lat2"*/	
	switch (Num_Piste) {
		case 1: 
		Insere_Atterrissage(&Lat1, LaTmp); 
		LaTmp->avion.centre.x=-100;
		LaTmp->avion.centre.y=-100;		
		Ranger_Liste_Avions_A(Lat1,AfficheA1, 9, 2);
		break; /*le petit truc ici c'est que ca doit être trié*/
		case 2: 
		Insere_Atterrissage(&Lat2, LaTmp); 
		LaTmp->avion.centre.x=-100;
		LaTmp->avion.centre.y=-100;		
		Ranger_Liste_Avions_A(Lat2,AfficheA2, 9, 3);
		break; /*le petit truc ici c'est que ca doit être trié*/
		}
	sprintf(LaTmp->avion.info,"%s-A------%2d-%2d-%d\n",AvionTmp->identifiant, AvionTmp->carburant, AvionTmp->consommation, Num_Piste);
	Completion_Numerique(AvionTmp->carburant,2, LaTmp->avion.info+14);
	Completion_Numerique(AvionTmp->consommation,2, LaTmp->avion.info+17);
/*et voici la sauvegarde dans l'historique comme demandé dans la consigne*/	
	if ((Sauvegarde=fopen(argv[1],"a+t"))==NULL) { /*"a+" pour se positionner en fin de fichier et "t" pour texte*/
	fprintf(stderr,"Erreur d'ouverture de fichier\n");
	return 1;
	}
	fprintf(Sauvegarde,LaTmp->avion.info);
	fclose(Sauvegarde);
	}
	}
/*Terminé! ouf...*/
	
	close_window(X_piste);
	free(X_piste);
	X_piste=(MlvType *)malloc(sizeof(MlvType));
	if (X_piste==NULL) return 1;
	create_window (display_name, width, hight, X_piste);
	name_window ("SIMULATION DU TRAFIC D'UN AEROPORT",X_piste);	
	
	Affiche_Decors(LeDecors, X_piste); 
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	close_window(X_menu);
	free(X_menu);
	break;
/*******************FIN AJOUT D'UN AVION A L'ATTERRISSAGE**********************/



/****************DEBUT SUPPRESSION D'UN AVION AU DECOLLAGE*********************/
case 4 :
	if (!debut) break;
	
	X_menu=(MlvType *)malloc(sizeof(MlvType));
	if (X_menu==NULL) return 1;
	create_window (display_name, width, hight, X_menu);
	name_window ("SUPPRESSION D'UN AVION AU DECOLLAGE",X_menu);

/*On choisit sur laquelle piste on va supprimer un avion*/
	Choisir_nbr(20,"Pour choisir le numero de la piste: ", &Num_Piste, 1, 3, 2, X_menu);
/*Ici on ne supprime pas, on met juste dans la liste des Inoccupés*/ 	
	switch(Num_Piste) {
	case 1 : 
		LaTmp=Choisir_Avion(100+20,"Pour supprimer un avion sur P1 :", Lem1, X_menu); 
		if (LaTmp==NULL) {
		r=-1;
		draw_filled_rectangle(0,0,W,H,"white", X_menu); /*on efface ce qu'il y avait d'écrit*/
		draw_string(20,20,"Aucun avion sur cette piste: veuillez choisir une autre piste SVP.", "10x20", "black", X_menu);
		draw_string(20,40,"Cliquez n'importe ou dans la fenetre pour quitter.","10x20","black", X_menu);
		display_window(X_menu);
		mouse_wait(&x_souris,&y_souris, X_menu);
		}
		else {
		Change_avion_Liste_Debut(LaTmp->avion, &Lem1, &Li, 7); /*on insere au début de Li*/
		Libere_Place_Affichage(LaTmp->avion, AfficheE1, 9);
		LaTmp->avion.centre.x=-100;
		LaTmp->avion.centre.y=-100;
		Ranger_Liste_Avions_E(Lem1, AfficheE1, 9, 0);
		}
		break;
	
	case 2 : 
		LaTmp=Choisir_Avion(100+20,"Pour supprimer un avion sur P2 :", Lem2, X_menu);
		if (LaTmp==NULL) {
		r=-1;
		draw_filled_rectangle(0,0,W,H,"white", X_menu); /*on efface ce qu'il y avait d'écrit*/
		draw_string(20,20,"Aucun avion sur cette piste: veuillez choisir une autre piste SVP.","10x20","black", X_menu);
		draw_string(20,40,"Cliquez n'importe ou dans la fenetre pour quitter.","10x20","black", X_menu);
		display_window(X_menu);
		mouse_wait(&x_souris,&y_souris, X_menu);
		}
		else {
		Change_avion_Liste_Debut(LaTmp->avion, &Lem2, &Li, 7); /*on insere au début de Li*/
		Libere_Place_Affichage(LaTmp->avion, AfficheE2, 9);
		LaTmp->avion.centre.x=-100;
		LaTmp->avion.centre.y=-100;		
		Ranger_Liste_Avions_E(Lem2, AfficheE2, 9, 1);
		}
		break;
	}
/*Terminé! ouf...*/	
	
	close_window(X_piste);
	free(X_piste);
	X_piste=(MlvType *)malloc(sizeof(MlvType));
	if (X_piste==NULL) return 1;
	create_window (display_name, width, hight, X_piste);
	name_window ("SIMULATION DU TRAFIC D'UN AEROPORT",X_piste);
	
	Affiche_Decors(LeDecors, X_piste); 
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	close_window(X_menu);
	free(X_menu);
	break;
/******************FIN SUPPRESSION D'UN AVION AU DECOLLAGE*********************/



/**************DEBUT CHANGER LA PRIORITE D'UN AVION AU ATTERRISSAGE***************/
case 5 :
	
	if (!test) {
	if (!debut) break;
	}
	
	X_menu=(MlvType *)malloc(sizeof(MlvType));
	if (X_menu==NULL) return 1;
	create_window (display_name, width, hight, X_menu);
	name_window ("CHANGER LA PRIORITE D'UN AVION AU ATTERRISSAGE",X_menu);

/*On choisit sur quelle piste on va supprimer un avion*/
	Choisir_nbr(20,"Pour choisir le numero de la piste: ", &Num_Piste, 1, 3, 2, X_menu);	
	switch(Num_Piste) {
	case 1 : 
		LaTmp=Choisir_Avion(100+20,"Pour choisir un avion vers P1 :", Lat1, X_menu); 
		if (LaTmp==NULL) {
		r=-1;
		draw_filled_rectangle(0,0,W,H,"white", X_menu); /*on efface ce qu'il y avait d'écrit*/
		draw_string(20,20,"Aucun avion pour cette piste: veuillez choisir une autre piste SVP.","10x20","black", X_menu);
		draw_string(20,40,"Cliquez n'importe ou dans la fenetre pour quitter.","10x20","black", X_menu);
		display_window(X_menu);
		mouse_wait(&x_souris,&y_souris, X_menu);
		}
		else {
/*Une amélioration de ma part: en plus de changer la priorité, on change''''''*/
/*l'ordre parmi les avions ayant un même nombre de tour (plus logique)''''''''*/
		Change_Avion_En_Urgence(LaTmp->avion, &Lat1, &Lat1, 4);		
		Ranger_Liste_Avions_A(Lat1,AfficheA1, 9, 2);		
		
		}
		break;
	
	case 2 : 
		LaTmp=Choisir_Avion(100+20,"Pour choisir un avion vers P2 :", Lat2, X_menu);
		if (LaTmp==NULL) {
		r=-1;
		draw_filled_rectangle(0,0,W,H,"white", X_menu); /*on efface ce qu'il y avait d'écrit*/
		draw_string(20,20,"Aucun avion pour cette piste: veuillez choisir une autre piste SVP.","10x20","black", X_menu);
		draw_string(20,40,"Cliquez n'importe ou dans la fenetre pour quitter.","10x20","black", X_menu);
		display_window(X_menu);
		mouse_wait(&x_souris,&y_souris, X_menu);
		}
		else {
/*Une amélioration de ma part: en plus de changer la priorité, on change''''''*/
/*l'ordre parmi les avions ayant un même nombre de tour (plus logique)''''''''*/
		Change_Avion_En_Urgence(LaTmp->avion, &Lat2, &Lat2, 5);
		Ranger_Liste_Avions_A(Lat2, AfficheA2, 9, 3);
		}
		break;
	}
	if (r!=-1) {	
	LaTmp->avion.info[7]='U';
/*et voici la sauvegarde dans l'historique comme demandé dans la consigne pour la mise en urgence*/	
	if ((Sauvegarde=fopen(argv[1],"a+t"))==NULL) { /*"a+" pour se positionner en fin de fichier et "t" pour texte*/
	fprintf(stderr,"Erreur d'ouverture de fichier\n");
	return 1;
	}
	fprintf(Sauvegarde,LaTmp->avion.info);
	fclose(Sauvegarde);
	}
/*Terminé! ouf...*/
	
	close_window(X_piste);
	free(X_piste);
	X_piste=(MlvType *)malloc(sizeof(MlvType));
	if (X_piste==NULL) return 1;
	create_window (display_name, width, hight, X_piste);
	name_window ("SIMULATION DU TRAFIC D'UN AEROPORT",X_piste);
	
	Affiche_Decors(LeDecors, X_piste); 
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);	
	
	close_window(X_menu);
	free(X_menu);
	break;
/****************FIN CHANGER LA PRIORITE D'UN AVION AU DECOLLAGE***************/



/****************DEBUT INTRODUIRE UNE COMPAGNIE DANS LA LISTE NOIRE**********************/
case 6 :
	
	
	if (!test) {
	if (!debut) break;
	}
	
	X_menu=(MlvType *)malloc(sizeof(MlvType));
	if (X_menu==NULL) return 1;
	create_window (display_name, width, hight, X_menu);
	name_window ("INTRODUIRE UNE COMPAGNIE DANS LA LISTE NOIRE",X_menu);
	
/*Tout dabord on choisit la compagnie que l'on veut exclure */	
	LcTmp=Choisir_Comp(20,"Pour choisir la compagnie :", Lc, X_menu);
	if (LcTmp==NULL) { /*s'il n'y a rien dans la liste_compagnie*/
	r=-1;
	draw_filled_rectangle(0,0,W,H,"white", X_menu); /*on efface ce qu'il y avait d'écrit*/
	draw_string(20,20,"Creer une compagnie SVP.","10x20","black", X_menu);
	draw_string(20,40,"Cliquez n'importe ou dans la fenetre pour quitter.","10x20","black", X_menu);
	display_window(X_menu);
	mouse_wait(&x_souris,&y_souris, X_menu);
	}
	else {
/*Ensuite on la met dans la liste noire*/
	LcTmp=Change_Compagnie_Liste_Debut(LcTmp->comp, &Lc, &Ln);
/*Apres on met tous les avions au decollage de la compagnie sur la Liste NULL et on les colorie tous en NOIR*/
	draw_filled_rectangle(0,0,W,H,"white", X_menu); /*on efface ce qu'il y avait d'écrit*/
	draw_string(20,60,"Vous avez Choisi :","10x20","black", X_menu);
	draw_string(20+19*10,60,LcTmp->comp.nom,"10x20","black", X_menu);
	draw_string(20,100,"Cette Compagnie va etre mise dans la \"Liste Noire des Compagnies\".","10x20","black", X_menu);
	draw_string(20,200,"Cliquez n'importe ou dans la fenetre pour quitter.","10x20","black", X_menu);
	display_window(X_menu);
	mouse_wait(&x_souris,&y_souris, X_menu);
	
	if ((Sauvegarde=fopen(argv[1],"a+t"))==NULL) { /*"a+" pour se positionner en fin de fichier et "t" pour texte*/
	fprintf(stderr,"Erreur d'ouverture de fichier\n");
	return 1;
	}
	Introduire_Avions_Liste_Noire(Sauvegarde, Park, LcTmp->comp.avions_compagnie, 2, 2, 6, &Lem1 , &Lem2, &Fq1, &Fq2); 
	fclose(Sauvegarde);
	}
/*Terminé! ouf...*/	
	
	close_window(X_piste);
	free(X_piste);
	X_piste=(MlvType *)malloc(sizeof(MlvType));
	if (X_piste==NULL) return 1;
	create_window (display_name, width, hight, X_piste);
	name_window ("SIMULATION DU TRAFIC D'UN AEROPORT",X_piste);
	
	Affiche_Decors(LeDecors, X_piste); 
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	close_window(X_menu);
	free(X_menu);
	break;
	
/******************FIN INTRODUIRE UN AVION EN LISTE NOIRE**********************/



/****************DEBUT GENERATEUR ALÉATOIRE D'EVENEMENT************************/
case 7 :
	if (!debut) break;
	
	X_menu=(MlvType *)malloc(sizeof(MlvType));
	if (X_menu==NULL) return 1;
	create_window (display_name, width, hight, X_menu);
	name_window ("GENERATEUR ALÉATOIRE D'EVENEMENT",X_menu);
	
	draw_string(20,20,"Bienvenue dans le generateur aleatoire d'evenement.","10x20","black", X_menu);
	draw_string(20,40,"Cliquez n'importe ou dans la fenetre pour lancer la generation aleatoire","10x20","black", X_menu);
	display_window(X_menu);
	mouse_wait(&x_souris,&y_souris, X_menu);
	aleat=1;
	draw_string(20,80,"Merci de patienter.","10x20","black", X_menu);
	
/*Ceci est une animation de chargement*/
	int i;
	int temps=1000;
	draw_string(W/3+100+15,H/6+40,"Loading...","10x20","black", X_menu);
	for (i=0;i<temps;i++) {
	draw_filled_rectangle(W/3,H/6,(int)(((float)i/(float)temps)*300),20,"red",X_menu);
	display_window(X_menu);
	wait_micros(1);
	}
		
	draw_string(20,220,"Le resultat a ete obtenu avec succes : Vous allez etre redirige vers une autre option", "10x20","black", X_menu);
	draw_string(20,240,"Cliquez n'importe ou dans la fenetre pour quitter.","10x20","black", X_menu);
	display_window(X_menu);
	mouse_wait(&x_souris,&y_souris, X_menu);
/*Terminé!*/	
	
	close_window(X_piste);
	free(X_piste);
	X_piste=(MlvType *)malloc(sizeof(MlvType));
	if (X_piste==NULL) return 1;
	create_window (display_name, width, hight, X_piste);
	name_window ("SIMULATION DU TRAFIC D'UN AEROPORT",X_piste);
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	close_window(X_menu);
	free(X_menu);
	break;
/******************FIN GENERATEUR ALÉATOIRE D'EVENEMENT************************/



/*************DEBUT INFORMATIONS GENERALES SUR UNE COMPAGNIE*******************/
case 8 :
	if (!debut) break;
	
	X_menu=(MlvType *)malloc(sizeof(MlvType));
	if (X_menu==NULL) return 1;
	create_window (display_name, width, hight, X_menu);
	name_window ("INFORMATIONS GENERALES SUR UNE COMPAGNIE",X_menu);

	Choisir_nbr(20,"1.Liste Compagnies / 2.Liste Noire :", &Choix, 1, 3, 1, X_menu);
	draw_filled_rectangle(0,0,W,H,"white", X_menu);
	switch(Choix) {
/*Tout dabord on choisit la compagnie qui nous intéresse */	
case 1 : 
LcTmp=Choisir_Comp(20,"Pour choisir la compagnie :", Lc, X_menu);
	if (LcTmp==NULL) { /*s'il n'y a rien dans la liste_compagnie*/
	r=-1;
	draw_filled_rectangle(0,0,W,H,"white", X_menu); /*on efface ce qu'il y avait d'écrit*/
	draw_string(20,100+20,"Creer une compagnie SVP.","10x20","black", X_menu);
	draw_string(20,100+40,"Cliquez n'importe ou dans la fenetre pour quitter.","10x20","black", X_menu);
	display_window(X_menu);
	mouse_wait(&x_souris,&y_souris, X_menu);
	}
	else Voir_Etat_Avions_Compagnie(100+20,"Pour voir les avions de la Compagnie :", LcTmp->comp.avions_compagnie, Montre, X_menu);
	break;
case 2 :
LcTmp=Choisir_Comp(20,"Pour choisir la compagnie :", Ln, X_menu);
	if (LcTmp==NULL) { /*s'il n'y a rien dans la liste_compagnie*/
	r=-1;
	draw_filled_rectangle(0,0,W,H,"white", X_menu); /*on efface ce qu'il y avait d'écrit*/
	draw_string(20,100+20,"Aucune compagnie sur liste noire (cette action ne compte pas).","10x20","black", X_menu);
	draw_string(20,100+40,"Cliquez n'importe ou dans la fenetre pour quitter.","10x20","black", X_menu);
	display_window(X_menu);
	mouse_wait(&x_souris,&y_souris, X_menu);
	}
	else Voir_Etat_Avions_Compagnie(100+20,"Pour voir les avions de la Compagnie :", LcTmp->comp.avions_compagnie, Montre, X_menu);
	break;
	}

/*Terminé!*/	
	
	close_window(X_piste);
	free(X_piste);
	X_piste=(MlvType *)malloc(sizeof(MlvType));
	if (X_piste==NULL) return 1;
	create_window (display_name, width, hight, X_piste);
	name_window ("SIMULATION DU TRAFIC D'UN AEROPORT",X_piste);
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	close_window(X_menu);
	free(X_menu);
	break;
/****************FIN INFORMATIONS GENERALES SUR UNE COMPAGNIE******************/



/*****************************DEBUT HISTORIQUE*********************************/
case 9 :
	if (!debut) break;
	
	X_menu=(MlvType *)malloc(sizeof(MlvType));
	if (X_menu==NULL) return 1;
	create_window (display_name, width, hight, X_menu);
	name_window ("INFORMATIONS GENERALES SUR UNE COMPAGNIE",X_menu);

	if((Sauvegarde=fopen(argv[1],"rt"))==NULL) {
	fprintf(stderr,"Erreur d'ouverture de fichier\n");
	return 1;
	}
	Voir_Historique(20,"Pour voir l'historique :", Sauvegarde, X_menu);
	fclose(Sauvegarde);
	draw_string(20,300,"Cliquez n'importe ou dans la fenetre pour quitter","10x20","black", X_menu);
	display_window(X_menu);
	mouse_wait(&x_souris,&y_souris, X_menu);

/*Terminé!*/	

	close_window(X_piste);
	free(X_piste);
	X_piste=(MlvType *)malloc(sizeof(MlvType));
	if (X_piste==NULL) return 1;
	create_window (display_name, width, hight, X_piste);
	name_window ("SIMULATION DU TRAFIC D'UN AEROPORT",X_piste);
	
	Affiche_Decors(LeDecors, X_piste); 
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	close_window(X_menu);
	free(X_menu);
	break;
/******************************FIN HISTORIQUE**********************************/



/*****************************DEBUT COMMENCER**********************************/
case 10 :
	
	if (!debut) {
	X_menu=(MlvType *)malloc(sizeof(MlvType));
	if (X_menu==NULL) return 1;
	create_window (display_name, width, hight, X_menu);
	name_window ("START",X_menu);
	
		
	debut=1;
	
	int i,j;
	char compt[3];compt[1]='\0';
	int temps=40; /*temps laissé à l'utilisateur pour lire les indications*/
	if (test==1) temps=5;
	
	draw_string(W/3,H/4,"Le Jeu commencera dans    secondes.","10x20","black", X_menu);
	
	draw_string(10,H/4+3*20,"Quelques conseils avant de commencer a jouer:","10x20","red", X_menu);
	draw_string(20,H/4+5*20,"1/ Les boutons sont presentes sous la forme :","9x15","chocolate4", X_menu);
	draw_filled_rectangle(20+43*10,H/4+20*5-20+5,7*10,20,"gray",X_menu);
	draw_string(20+43*10,H/4+5*20,"boutons","10x20","black", X_menu);
	draw_string(20,H/4+7*20,"Par exemple, la phrase :","9x15","chocolate4", X_menu);
	draw_filled_rectangle(20+(52-5)*10,H/4+7*20-20+5,10*10,20,"gray",X_menu);
	draw_filled_rectangle(20+(69-5)*10,H/4+7*20-20+5,9*10,20,"gray",X_menu);
	draw_string(20+(30-5)*10,H/4+7*20,"Parcourez la liste en descendant ou en remontant","10x20","black", X_menu);
	draw_string(20,H/4+9*20,"vous invite a parcourir une liste d'informations (Noms des compagnies...etc.) en cliquant","9x15","chocolate4", X_menu);
	draw_string(20,H/4+10*20,"sur les boutons \"descendant\" et \"remontant\" pour respectivement descendre et remonter dans la liste.", "9x15","chocolate4", X_menu);
	draw_string(20,H/4+13*20,"2/ Vous aurez souvent besoin d'utiliser successivement un meme bouton pour parcourir les listes :", "9x15","chocolate4", X_menu);
	draw_string(20,H/4+14*20,"Cela peut-etre facilite par la roulette de votre souris - si vous en avez une - qui compte", "9x15","chocolate4", X_menu);
	draw_string(20,H/4+15*20,"pour plusieurs clics.", "9x15","chocolate4", X_menu);
	
/*Animation de Chargement*/
	for (i=0;i<temps;i++) {
	draw_filled_rectangle(W/3,H/6,(int)(((float)i/(float)temps)*350),20,"red",X_menu);
	sprintf(compt,"%2d",temps-i);
	draw_filled_rectangle(W/3+10*22,H/4-25,30,40,"white",X_menu);
	draw_string(W/3+10*23,H/4,compt,"10x20","black", X_menu);
	for (j=0;j<100;j++) {
	draw_filled_rectangle(W/3,H/6,(int)((((float)j)/((float)100))*((float)1/(float)temps)*350 +((float)i/(float)temps)*350),20, "red", X_menu);
	display_window(X_menu);
	wait_micros(10000);
	}
	}
	
	/*Terminé!*/	
	close_window(X_piste);
	free(X_piste);
	X_piste=(MlvType *)malloc(sizeof(MlvType));
	if (X_piste==NULL) return 1;
	create_window (display_name, width, hight, X_piste);
	name_window ("SIMULATION DU TRAFIC D'UN AEROPORT",X_piste);
	
	
	Affiche_Decors(LeDecors, X_piste); 
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);

	
	close_window(X_menu);
	free(X_menu);
	}
	else r=-1;
	
	break;
/******************************FIN COMMENCER***********************************/



/*******************************DEBUT SORTIR***********************************/
case 11 :
	fin=1;
	break;
/*********************************FIN SORTIR***********************************/
	
	

/***************************DEBUT Ne rien faire********************************/
case 12 : 
	if (test && !debut) Mise_A_Jour (&Montre, DELAI);
	break;
/*****************************FIN Ne rien faire********************************/



/*******************************DEBUT DEFAULT**********************************/
default :
	r=-1; /*normalement ne sera jamais utilisé du aux controles fait en debut de main*/
	break;
/*********************************FIN DEFAULT**********************************/
	}

printf("fin du Menu\n");

Round=0;

while (debut && r!=-1 && !aleat && !fin && Round!=DELAI) {

printf("debut de la boucle\n");

	if (Lc==NULL && Ln==NULL && Lat1==NULL && Lat2==NULL && Lem1==NULL && Lem2==NULL) {
	Mise_A_Jour (&Montre, DELAI);
	break;
	}	

	if (r!=12) wait_micros(100000);

	Affiche_Evenements_Suivants1(&LeDecors , &Fq1, Lat1, Lem1, 0);
	Affiche_Evenements_Suivants2(&LeDecors , &Fq2, Lat2, Lem2, 21+ecart);
	LeDecors.nom[7]=message03;

	Affiche_Decors(LeDecors, X_piste); 
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);

/*Pour la Piste 1*/

/*la queue*/
/*on introduit au debut de Fq.dernier et on fait pointer Fq.premier a chaque fois sur le dernier introduit*/
	Mettre_En_Queue(&Lem1, Lem2, AfficheE1, &Fq1, Montre, 0, 9, Graph, TabColor,  Embarq, X_piste);

	Ranger_Liste_Avions_E(Lem1, AfficheE1, 9, 0);
	
	Affiche_Evenements_Suivants1(&LeDecors , &Fq1, Lat1, Lem1, 0);
	Affiche_Evenements_Suivants2(&LeDecors , &Fq2, Lat2, Lem2, 21+ecart);	
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);

	Ranger_Liste_Avions_Q(Fq1.dernier, AfficheQ1, 5, 0, Graph, TabColor, LaPiste1, X_piste);

	Affiche_Evenements_Suivants1(&LeDecors , &Fq1, Lat1, Lem1, 0);
	Affiche_Evenements_Suivants2(&LeDecors , &Fq2, Lat2, Lem2, 21+ecart);

	if (Lat1!=NULL) Choix=Lat1->avion.mode;
	else {
	if (Fq1.premier==NULL) Choix=-1;
	else Choix=2;
	}

switch (Choix) {

case 4 :
/*Urgence*/ 
/*La mise en urgence ne depend que de l'utilisateur: à lui de bien gérer son aéroport*/
/*reconnaissance avec mode*/
	printf("Choix P1 m=4\n");
	Lat1->avion.axe.x=100;
	Lat1->avion.axe.y=0;
	Lat1->avion.centre.x=0.1*(Dbord+Dgtab);
	Lat1->avion.centre.y=0.5*(0.5*(sep1+Dbord+sep2)+sep2);
	direction.x= 0.95*(Dbord+Dgtab);
	direction.y=0.5*(0.5*(sep1+Dbord+sep2)+sep2);
/*(animation: Atterrissage)*/	
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	Cinematique_Avion(&Lat1->avion, 0, Fq1.dernier, 5, Graph, TabColor, direction, LaPiste1, X_piste,0);
	strncpy(infoPiste1, Lat1->avion.info, 21);
	Change_avion_Liste_Debut(Lat1->avion, &Lat1, &Li, 7);
	Ranger_Liste_Avions_A(Lat1, AfficheA1, 9, 2);
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	break;

case 6 :
/*Liste Noire*/
/*(animation: Atterrissage)*/
	printf("Choix P1 m=6\n");
	Lat1->avion.axe.x=100;
	Lat1->avion.axe.y=0;
	Lat1->avion.centre.x=0.1*(Dbord+Dgtab);
	Lat1->avion.centre.y=0.5*(0.5*(sep1+Dbord+sep2)+sep2);
	direction.x= 0.95*(Dbord+Dgtab);
	direction.y=0.5*(0.5*(sep1+Dbord+sep2)+sep2);
/*(animation: Atterrissage)*/	
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	Cinematique_Avion(&Lat1->avion, 0, Fq1.dernier, 5, Graph, TabColor, direction, LaPiste1, X_piste,0);
	strncpy(infoPiste1, Lat1->avion.info, 21);
	Supprime_Avion_Attente(&Lat1, Lat1->avion);
	Ranger_Liste_Avions_A(Lat1,AfficheA1, 9, 2);
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	break;

case 2 :

/*ATTENTION IL FAUT RECHERCHER EN FAIT LES AVIONS EN LISTE NOIR: S'IL Y A EN A
FAIRE PASSER DABORD CEUX EN URGENCE PUIS EN LISTE NOIR*/

/*tout dabord on Recherche_Attente_ParMode les avions en liste noir*/
/*s'il y en a on le fait atterir (comme en case 6) */
/*break;*/
	printf("Choix P1 m=2\n");
	if ((LaTmp=Recherche_Attente_ParMode(Lat1, 6))!=NULL) {
	LaTmp->avion.axe.x=100;
	LaTmp->avion.axe.y=0;
	LaTmp->avion.centre.x=0.1*(Dbord+Dgtab);
	LaTmp->avion.centre.y=0.5*(0.5*(sep1+Dbord+sep2)+sep2);
	direction.x= 0.95*(Dbord+Dgtab);
	direction.y=0.5*(0.5*(sep1+Dbord+sep2)+sep2);
/*(animation: Atterrissage)*/
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	Cinematique_Avion(&LaTmp->avion, 0, Fq1.dernier, 5, Graph, TabColor, direction, LaPiste1, X_piste,0);
	strncpy(infoPiste1, Lat1->avion.info, 21);
	Supprime_Avion_Attente(&Lat1, LaTmp->avion);
	Ranger_Liste_Avions_A(Lat1,AfficheA1, 9, 2);
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	break;
	}
	else {
/*Atterrisage*/
	if (Fq1.premier==NULL) {
/*Atterrisage*/	
	Lat1->avion.axe.x=100;
	Lat1->avion.axe.y=0;
	Lat1->avion.centre.x=0.1*(Dbord+Dgtab);
	Lat1->avion.centre.y=0.5*(0.5*(sep1+Dbord+sep2)+sep2);
	direction.x= 0.95*(Dbord+Dgtab);
	direction.y=0.5*(0.5*(sep1+Dbord+sep2)+sep2);
/*(animation: Atterrissage)*/	
		
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
		
	Cinematique_Avion(&Lat1->avion,0, Fq1.dernier, 5, Graph, TabColor, direction, LaPiste1, X_piste,0);
	strncpy(infoPiste1, Lat1->avion.info, 21);
	Change_avion_Liste_Debut(Lat1->avion, &Lat1, &Li, 7);
	Ranger_Liste_Avions_A(Lat1,AfficheA1, 9, 2);
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
		
		break;
		}
	else {;
	Fq1.premier->avion.axe.x=-100;
	Fq1.premier->avion.axe.y=0;
	
	direction.x= 0.1*(Dbord+Dgtab);
	direction.y=0.5*(0.5*(sep1+Dbord+sep2)+sep1+Dbord);
/*(animation: decollage)*/	
		
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
		
	Libere_Place_Affichage(Fq1.premier->avion, AfficheQ1, 5);
	Cinematique_Avion(&Fq1.premier->avion, 1, Fq1.dernier, 5, Graph, TabColor, direction, LaPiste1, X_piste,0);
	LaTmp=Supprime_Avion_Premier_Queue(&Fq1);
	LcTmp=Recherche_Compagnie_Affilie(Lc, LaTmp->avion);
	strncpy(infoPiste1, LaTmp->avion.info, 21);
	Supprime_Avion_Compagnie(&LcTmp->comp, LaTmp->avion);
	Ranger_Liste_Avions_E(Lem1, AfficheE1, 9, 0);
		
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	Ranger_Liste_Avions_Q(Fq1.dernier, AfficheQ1, 5, 0, Graph, TabColor, LaPiste1, X_piste);
	
	break;
	}
	}
	break;/*juste par precaution*/
	}/*FIN DU SWITCH*/

printf("Affiche Evenements Passes \n");

/*Calcul des événements passés*/	
	if (strncmp(LeDecors.nom[19], infoPiste1, 21)!=0) { 
	strncpy(LeDecors.nom[21], LeDecors.nom[20], 21);
	strncpy(LeDecors.nom[20], LeDecors.nom[19], 21);
	strncpy(LeDecors.nom[19], infoPiste1, 21);
	}
	if (strncmp(LeDecors.nom[19]+21+ecart, infoPiste2, 21)!=0) { 
	strncpy(LeDecors.nom[21]+21+ecart, LeDecors.nom[20]+21+ecart,21);
	strncpy(LeDecors.nom[20]+21+ecart, LeDecors.nom[19]+21+ecart,21);
	strncpy(LeDecors.nom[19]+21+ecart, infoPiste2, 21);
	}

	Affiche_Evenements_Suivants1(&LeDecors , &Fq1, Lat1, Lem1, 0);
	Affiche_Evenements_Suivants2(&LeDecors , &Fq2, Lat2, Lem2, 21+ecart);
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);

printf("Consommation P1\n");
	
/*Mise à jour de la consommation du carburant pour la Lat1*/
for (LaTmp=Lat1; LaTmp!=NULL; LaTmp=LaTmp->suivant_attente) LaTmp->avion.carburant-=LaTmp->avion.consommation;

printf("Crash P1 m=4\n");
	
/*crash a regarder apres chaque atterrissage : i.e. c'est celui qui n'a pas pu aterrir et n'a plus de carb.*/
	if ((Sauvegarde=fopen(argv[1],"a+t"))==NULL) { 
	fprintf(stderr,"Erreur d'ouverture de fichier\n");
	return 1;
	}
	Crash_Atterrissage(Lc, Ln, &Lat1, Graph, TabColor, Sauvegarde, infoPiste1, X_piste);	
	fclose(Sauvegarde);

	Ranger_Liste_Avions_A(Lat1,AfficheA1, 9, 2);
		
	Affiche_Evenements_Suivants1(&LeDecors , &Fq1, Lat1, Lem1, 0);
	Affiche_Evenements_Suivants2(&LeDecors , &Fq2, Lat2, Lem2, 21+ecart);
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);

	Affiche_Evenements_Suivants1(&LeDecors , &Fq1, Lat1, Lem1, 0);
	Affiche_Evenements_Suivants2(&LeDecors , &Fq2, Lat2, Lem2, 21+ecart);

printf("Affiche Evenements Passes \n");

/*Calcul des événements passés*/	
	if (strncmp(LeDecors.nom[19], infoPiste1, 21)!=0) { 
	strncpy(LeDecors.nom[21], LeDecors.nom[20], 21);
	strncpy(LeDecors.nom[20], LeDecors.nom[19], 21);
	strncpy(LeDecors.nom[19], infoPiste1, 21);
	}
	if (strncmp(LeDecors.nom[19]+21+ecart, infoPiste2, 21)!=0) { 
	strncpy(LeDecors.nom[21]+21+ecart, LeDecors.nom[20]+21+ecart,21);
	strncpy(LeDecors.nom[20]+21+ecart, LeDecors.nom[19]+21+ecart,21);
	strncpy(LeDecors.nom[19]+21+ecart, infoPiste2, 21);
	}


/*Pour la Piste 2*/
/*la queue*/
/*on introduit au debut de Fq.dernier et on fait pointer Fq.premier a chaque fois sur le dernier introduit*/
	Mettre_En_Queue(&Lem2, Lem1, AfficheE2, &Fq2, Montre, 1, 9, Graph, TabColor, Embarq, X_piste);	

	Ranger_Liste_Avions_E(Lem2, AfficheE2, 9, 0);

	Affiche_Evenements_Suivants1(&LeDecors , &Fq1, Lat1, Lem1, 0);
	Affiche_Evenements_Suivants2(&LeDecors , &Fq2, Lat2, Lem2, 21+ecart);	
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);

	Ranger_Liste_Avions_Q(Fq2.dernier, AfficheQ2, 5, 0, Graph, TabColor, LaPiste2, X_piste);

if (Lat2!=NULL) Choix=Lat2->avion.mode;
else {
if (Fq2.premier==NULL) Choix=-1;
else Choix=3;
}

switch (Choix) {

case 5 :
/*Urgence*/ 
/*La mise en urgence ne depend que de l'utilisateur: à lui de bien gérer son aéroport*/
/*reconnaissance avec mode*/
	printf("Choix P2 m=5\n");
	Lat2->avion.axe.x=100;
	Lat2->avion.axe.y=0;
	Lat2->avion.centre.x=0.1*(Dbord+Dgtab);
	Lat2->avion.centre.y=0.5*(0.5*(sep2+Dbord+sep3)+sep3);
	direction.x= 0.95*(Dbord+Dgtab);
	direction.y=0.5*(0.5*(sep2+Dbord+sep3)+sep3);
/*(animation: Atterrissage)*/	
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	Cinematique_Avion(&Lat2->avion, 0, Fq2.dernier, 5, Graph, TabColor, direction, LaPiste2, X_piste,0);	
	strncpy(infoPiste2, Lat2->avion.info, 21);
	Change_avion_Liste_Debut(Lat2->avion, &Lat2, &Li, 7);
	Ranger_Liste_Avions_A(Lat2, AfficheA2, 9, 3);
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	break;

case 6 :
/*Liste Noire*/
/*(animation: Atterrissage)*/
	printf("Choix P2 m=6\n");
	Lat2->avion.axe.x=100;
	Lat2->avion.axe.y=0;
	Lat2->avion.centre.x=0.1*(Dbord+Dgtab);
	Lat2->avion.centre.y=0.5*(0.5*(sep2+Dbord+sep3)+sep3);
	direction.x= 0.95*(Dbord+Dgtab);
	direction.y=0.5*(0.5*(sep2+Dbord+sep3)+sep3);
/*(animation: Atterrissage)*/	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	Cinematique_Avion(&Lat2->avion, 0, Fq2.dernier, 5, Graph, TabColor, direction, LaPiste2, X_piste,0);
	strncpy(infoPiste2, Lat2->avion.info, 21);
	Supprime_Avion_Attente(&Lat2, Lat2->avion);
	Ranger_Liste_Avions_A(Lat2, AfficheA2, 9, 3);	
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	break;

case 3 :

/*ATTENTION IL FAUT RECHERCHER EN FAIT LES AVIONS EN LISTE NOIR: S'IL Y A EN A
FAIRE PASSER DABORD CEUX EN URGENCE PUIS EN LISTE NOIR*/

/*tout dabord on Recherche_Attente_ParMode les avions en liste noir*/
/*s'il y en a on le fait atterir (comme en case 6) */
/*break;*/
	printf("Choix P2 m=3\n");
	if ((LaTmp=Recherche_Attente_ParMode(Lat2, 6))!=NULL) {
	LaTmp->avion.axe.x=100;
	LaTmp->avion.axe.y=0;
	LaTmp->avion.centre.x=0.1*(Dbord+Dgtab);
	LaTmp->avion.centre.y=0.5*(0.5*(sep2+Dbord+sep3)+sep3);
	direction.x= 0.95*(Dbord+Dgtab);
	direction.y=0.5*(0.5*(sep2+Dbord+sep3)+sep3);
/*(animation: Atterrissage)*/	
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	Cinematique_Avion(&LaTmp->avion, 0, Fq2.dernier, 5, Graph, TabColor, direction, LaPiste2, X_piste,0);
	strncpy(infoPiste2, Lat2->avion.info, 21);
	Supprime_Avion_Attente(&Lat2, LaTmp->avion);
	Ranger_Liste_Avions_A(Lat2, AfficheA2, 9, 3);
	
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	break;
	}
	else {
/*Atterrisage*/
	if (Fq2.premier==NULL) {
	/*Atterrisage*/	
	Lat2->avion.axe.x=100;
	Lat2->avion.axe.y=0;
	Lat2->avion.centre.x=0.1*(Dbord+Dgtab);
	Lat2->avion.centre.y=0.5*(0.5*(sep2+Dbord+sep3)+sep3);
	direction.x= 0.95*(Dbord+Dgtab);
	direction.y=0.5*(0.5*(sep2+Dbord+sep3)+sep3);
/*(animation: Atterrissage)*/	
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	Cinematique_Avion(&Lat2->avion, 0, Fq2.dernier, 5, Graph, TabColor, direction, LaPiste2, X_piste,0);
	strncpy(infoPiste2, Lat2->avion.info, 21);
	Change_avion_Liste_Debut(Lat2->avion, &Lat2, &Li, 7);
	Ranger_Liste_Avions_A(Lat2, AfficheA2, 9, 3);
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	break;
	}
	else {
	Fq2.premier->avion.axe.x=-100;
	Fq2.premier->avion.axe.y=0;

	direction.x= 0.1*(Dbord+Dgtab);
	direction.y=0.5*(0.5*(sep2+Dbord+sep3)+sep2+Dbord);
/*(animation: decollage)*/	
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);

	Libere_Place_Affichage(Fq2.premier->avion, AfficheQ2, 5);
	Cinematique_Avion(&Fq2.premier->avion, 1, Fq2.dernier, 5, Graph, TabColor, direction, LaPiste2, X_piste,0);
	LaTmp=Supprime_Avion_Premier_Queue(&Fq2);
	LcTmp=Recherche_Compagnie_Affilie(Lc, LaTmp->avion);
	strncpy(infoPiste2, LaTmp->avion.info, 21);
	Supprime_Avion_Compagnie(&LcTmp->comp, LaTmp->avion);
	
	Ranger_Liste_Avions_E(Lem2,AfficheE2, 9, 1);
		
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);
	
	Ranger_Liste_Avions_Q(Fq2.dernier, AfficheQ2, 5, 1, Graph, TabColor, LaPiste2, X_piste);
	
	break;
	}
	
	break;
	}
	}/*FIN DU SWITCH*/

printf("Affiche Evenements Passes \n");

/*Calcul des évènements passés*/	
	if (strncmp(LeDecors.nom[19], infoPiste1, 21)!=0) { 
	strncpy(LeDecors.nom[21], LeDecors.nom[20], 21);
	strncpy(LeDecors.nom[20], LeDecors.nom[19], 21);
	strncpy(LeDecors.nom[19], infoPiste1, 21);
	}
	if (strncmp(LeDecors.nom[19]+21+ecart, infoPiste2, 21)!=0) { 
	strncpy(LeDecors.nom[21]+21+ecart, LeDecors.nom[20]+21+ecart,21);
	strncpy(LeDecors.nom[20]+21+ecart, LeDecors.nom[19]+21+ecart,21);
	strncpy(LeDecors.nom[19]+21+ecart, infoPiste2, 21);
	}

	Affiche_Evenements_Suivants1(&LeDecors , &Fq1, Lat1, Lem1, 0);
	Affiche_Evenements_Suivants2(&LeDecors , &Fq2, Lat2, Lem2, 21+ecart);
	
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);

printf("Consommation P2 \n");	
/*Mise à jour de la consommation du carburant pour la Lat1*/
for (LaTmp=Lat2; LaTmp!=NULL; LaTmp=LaTmp->suivant_attente) LaTmp->avion.carburant-=LaTmp->avion.consommation;
printf("Crash P2 \n");	
/*crash a regarder apres chaque atterrissage : i.e. c'est celui qui n'a pas pu aterrir et n'a plus de carb.*/
	if ((Sauvegarde=fopen(argv[1],"a+t"))==NULL) { 
	fprintf(stderr,"Erreur d'ouverture de fichier\n");
	return 1;
	}
	Crash_Atterrissage(Lc, Ln, &Lat2, Graph, TabColor, Sauvegarde, infoPiste2, X_piste);
	fclose(Sauvegarde);

	Ranger_Liste_Avions_A(Lat2, AfficheA2, 9, 3);
	
	Affiche_Evenements_Suivants1(&LeDecors , &Fq1, Lat1, Lem1, 0);
	Affiche_Evenements_Suivants2(&LeDecors , &Fq2, Lat2, Lem2, 21+ecart);
		
	Affiche_Decors(LeDecors, X_piste);
	Affiche_Tous_Les_Avions(Lat1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lat2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem1, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Lem2, 9, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq1.dernier, 5, Graph, TabColor, X_piste);
	Affiche_Tous_Les_Avions(Fq2.dernier, 5, Graph, TabColor, X_piste);
	display_window(X_piste);

	Affiche_Evenements_Suivants1(&LeDecors , &Fq1, Lat1, Lem1, 0);
	Affiche_Evenements_Suivants2(&LeDecors , &Fq2, Lat2, Lem2, 21+ecart);

printf("Affiche Evenements Passes \n");	

/*Calcul des évènements passés*/	
	if (strncmp(LeDecors.nom[19], infoPiste1, 21)!=0) { 
	strncpy(LeDecors.nom[21], LeDecors.nom[20], 21);
	strncpy(LeDecors.nom[20], LeDecors.nom[19], 21);
	strncpy(LeDecors.nom[19], infoPiste1, 21);
	}
	if (strncmp(LeDecors.nom[19]+21+ecart, infoPiste2, 21)!=0) { 
	strncpy(LeDecors.nom[21]+21+ecart, LeDecors.nom[20]+21+ecart,21);
	strncpy(LeDecors.nom[20]+21+ecart, LeDecors.nom[19]+21+ecart,21);
	strncpy(LeDecors.nom[19]+21+ecart, infoPiste2, 21);
	}

printf("Mise a jour : horloge \n");	

/*On met à Jour l'heure*/
	Mise_A_Jour (&Montre, 1);
	Round++;
	sprintf(HorlogeTmp,"%2d:%2d",Montre.Heure,Montre.Minutes);
	Completion_Numerique(Montre.Heure,2,HorlogeTmp);
	Completion_Numerique(Montre.Minutes,2,HorlogeTmp+3);
	LeDecors.nom[11]=HorlogeTmp;

}
	r=-1; /*i.e. = à l'utilisateur de jouer*/

} while(!fin);

/*Avant de quitter on libère ce qui n'a pas déjà été libéré*/

for (i_ev=12; i_ev<22;i_ev++) free(LeDecors.nom[i_ev]);
Liberer_Liste_avions_Attente(Lat1);
Liberer_Liste_avions_Attente(Lat2);
Liberer_Liste_avions_Attente(Lem1);
Liberer_Liste_avions_Attente(Lem2);
Liberer_Liste_avions_Attente(Fq1.dernier);
Liberer_Liste_avions_Attente(Fq2.dernier);
Liberer_Liste_compagnie(Lc);
Liberer_Liste_compagnie(Ln);
close_window(X_piste);
free(X_piste);
return 0;
}

/******************************************************************************/
/*FIN*/
/******************************************************************************/
