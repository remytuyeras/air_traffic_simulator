## Before compiling Gestion_aeroport.c
1. Download the fime ```compi_c``` by following the instruction here (in french):
 * ```http://www-igm.univ-mlv.fr/ens/Common/libMlv/install.php```
2. copy the file ```compi_c``` in the directory where ```Percolation.c``` is saved.
3. run ```chmod +x compi_c ```

## To run Gestion_aeroport.c
	
1. Open the terminal

2. GO to the directory containing the folliowing files:
	```Gestion_aeroport.c```
	```LstComp.txt```
	```compi_c``` (obligatoire pour la compilation en mode graphique)

3. Enter the folloiwng command in the terminal: 

* ```./compi_c Gestion_aeroport.c -o [Name of the output program]```

Note: the file ``./compi_c.``` contains the flag ```FLAGS="-Wall -ansi"```

4. Then enter ```./[Name of the output program] [Save file name] [Game mode]```
	
To test the game, the mode ```[Game mode]``` is ```test``` and to play the game, the mode ```[Game mode]``` is ```normal```.
	
The mode ```normal``` allows the user to start a game from scratch (air plane companies need to be created).  The mode ```test``` allows the user to start a game from a constructed situation (events can be scheduled before the start).
	
![GitHub Logo](/game1.png)
![GitHub Logo](/game2.png)
![GitHub Logo](/game3.png)
		
