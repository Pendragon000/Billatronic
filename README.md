# Project de session: Remise 1
Le code commence à ./main/billatronic.c dans la fonction main. 
## Explication simple
Je fais un simple poll sur le keyboard pour savoir qu'elle touche est actuellement pesé et selon quel touche est pesé trois actions peuvent arriver:
- '*' ça réinitialise la string, 
- '#' ça trigger la vérification avec le mot de passe en allumant la LED appropriée
- Tous autre caratère se fait rajouter à la string
## Structure
Dans components j'ai:
 - Keypad: le code que j'ai écrit pour le clavier à membrane
 - led: le code du prof pour les led
Dans main j'ai:
 - Billatronic.c: c'est mon point entré pour mon code ou se trouve ma fonction main
## Autre doc
Le reste de la doc du code se trouve sur les fichiers respectif ou j'explique chaque fonction que j'ai écrit.