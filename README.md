## Before compiling Gestion_aeroport.c
1. Download the fime ```compi_c``` by following the instruction here (in french):
 * ```http://www-igm.univ-mlv.fr/ens/Common/libMlv/install.php```
2. copy the file ```compi_c``` in the directory where ```Percolation.c``` is saved.
3. run ```chmod +x compi_c ```

## To run Gestion_aeroport.c (in french -- from original documentation)
	
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
	
![GitHub Logo](/game1.png)
![GitHub Logo](/game2.png)
![GitHub Logo](/game3.png)
		
