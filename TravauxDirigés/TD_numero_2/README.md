# TP2 de NOM Prénom

`pandoc -s --toc tp2.md --css=./github-pandoc.css -o tp2.html`

## Questions de cours

La partie du code d'Alice exécuté en parallèle représente en temps de traitement 90% du temps en séquentiel.

Quelle est l'accélération maximale que pourra obtenir Alice avec son code ? (n>>1)

Le meilleur *Speedup* est : $S(n) = \frac{n}{1+(n-1)f} \rightarrow \frac{1}{f}$

Avec f la fraction du code non parallélisable.

Dans le cas d'Alice, f = 0.9, alors $\frac{1}{f} = \frac{1}{0.9} = 1.11$

Alors, l'accélération maximale que pourra obtenir Alice est de 1.11



## Mandelbrot 

*Expliquer votre stratégie pour faire une partition équitable des lignes de l'image entre chaque processus*

           | Taille image : 800 x 600 | 
-----------+---------------------------
séquentiel |              
1          |              
2          |              
3          |              
4          |              
8          |              


*Discuter sur ce qu'on observe, la logique qui s'y cache.*

*Expliquer votre stratégie pour faire une partition dynamique des lignes de l'image entre chaque processus*

           | Taille image : 800 x 600 | 
-----------+---------------------------
séquentiel |              
1          |              
2          |              
3          |              
4          |              
8          |              



## Produit matrice-vecteur



*Expliquer la façon dont vous avez calculé la dimension locale sur chaque processus, en particulier quand le nombre de processus ne divise pas la dimension de la matrice.*
