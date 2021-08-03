## To compile Percolation.c
1. Download the fime ```compi_c``` by following the instruction here (in french):
 * ```http://www-igm.univ-mlv.fr/ens/Common/libMlv/install.php```
2. copy the file ```compi_c``` in the directory where ```Percolation.c``` is saved.
3. run ```chmod +x compi_c ```
4. compile using ```./compi_c Percolation.c```

## EXECUTION DU PROJET:
	
1. Ouvrir le terminal.

2. Aller dans le répertoire où se trouvent les fichiers:
	```Gestion_aeroport.c```
	```LstComp.txt```
	```compi_c``` (obligatoire pour la compilation en mode graphique)

3. Taper la commande: ```./compi_c Gestion_aeroport.c -o [Nom de l'executable]```

	Remarque : il y existe un ```FLAGS="-Wall -ansi"``` dans le ```./compi_c.```

4. Puis taper ensuite ```./[Nom de l'executable] [Nom du Fichier de Sauvegarde] [Mode de Jeu]```

	1. Le ```[Nom du Fichier de Sauvegarde]``` est imposé par l'énoncé : ```aeroport.log```
	
	2. Le ```[Mode de Jeu]``` est un mot clef spécifiant si l'on veut utiliser 
	l'executable en mode "test" ou en mode "normal".
	
	Pour le mode "test", le ```[Mode de Jeu]``` est "test" et pour le mode 
	"normal", le ```[Mode de Jeu]``` est "normal".
	
	Le mode "normal" permet à l'utilisateur de débuter la simulation d'une manière
	tout à fait classique : sans avion et sans compagnie.
	Le mode "test" permet à l'utilisateur de commencer dans n'importe quelle 
	situation qu'il souhaite. Ce mode remplace les fichiers de test à fournir.
	(le mode "test" sera plus explicitement exposé dans le rapport).
		
## LES ERREURS CONNUES:
	
	Il s'est trouvé un jour que le programme est tombé dans une boucle infinie.
	Depuis, le programme a subi des modifications et l'erreur n'a plus été rencontrée par la suite.
	Cependant celle-ci n'a pu être concrètement identifée et l'on ne peut
	affirmer à 100% qu'elle n'y soit plus.
	
		Pour precision : cette erreur est survenue après la réutilisation d'un avion
		placé au GARAGE (Liste des inoccupés) dans le but d'un décollage. L'erreur est alors
		survenue après le choix de la piste à laquelle il était destiné.
	
## LES AMELIORATIONS POSSIBLES:

*	AMELIORATION EN MODE TEXTE: Etant donné qu'une certaine logique a été imposée
	par l'énoncé, il est difficile de voir quelle(s) amélioration(s) pourrai(en)t être
	envisagé(es). Manque d'imagination, nous n'en avons trouvé aucune.
	
*	AMELIORATION EN MODE GRAPHIQUE: Pour ce que la MlvLib propose en tant que 
	fonctions élémentaires, il est difficile de voir quelle(s) amélioration(s)
	pourrai(en)t être envisagé(es). Cependant avec quelques fonctions de transformations
	de la dimension 2 à la dimension 3 on pourrait penser à mettre en place 
	une interface graphique 3D : un travail long et surement trop laborieux.
		
