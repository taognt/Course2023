

# TP2 de NOM Prénom

`pandoc -s --toc tp2.md --css=./github-pandoc.css -o tp1.html`





## lscpu

```
coller ici le résultats de lscpu. 
```

*Des infos utiles s'y trouvent : nb core, taille de cache*


Architecture:            x86_64

  CPU op-mode(s):        32-bit, 64-bit

  Address sizes:         39 bits physical, 48 bits virtual

  Byte Order:            Little Endian

CPU(s):                  8

  On-line CPU(s) list:   0-7

Vendor ID:               GenuineIntel

  Model name:            11th Gen Intel(R) Core(TM) i7-1165G7 @ 2.80GHz

    CPU family:          6

    Model:               140

    Thread(s) per core:  2

    Core(s) per socket:  4

    Socket(s):           1

    Stepping:            1

    CPU max MHz:         4700,0000

    CPU min MHz:         400,0000

    BogoMIPS:            5606.40

    Flags:               fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov p
                         at pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscal
                         l nx pdpe1gb rdtscp lm constant_tsc art arch_perfmon pebs bts r
                         ep_good nopl xtopology nonstop_tsc cpuid aperfmperf tsc_known_f
                         req pni pclmulqdq dtes64 monitor ds_cpl vmx est tm2 ssse3 sdbg 
                         fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_d
                         eadline_timer aes xsave avx f16c rdrand lahf_lm abm 3dnowprefet
                         ch cpuid_fault epb cat_l2 invpcid_single cdp_l2 ssbd ibrs ibpb 
                         stibp ibrs_enhanced tpr_shadow vnmi flexpriority ept vpid ept_a
                         d fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid rdt_a av
                         x512f avx512dq rdseed adx smap avx512ifma clflushopt clwb intel
                         _pt avx512cd sha_ni avx512bw avx512vl xsaveopt xsavec xgetbv1 x
                         saves split_lock_detect dtherm ida arat pln pts hwp hwp_notify 
                         hwp_act_window hwp_epp hwp_pkg_req avx512vbmi umip pku ospke av
                         x512_vbmi2 gfni vaes vpclmulqdq avx512_vnni avx512_bitalg avx51
                         2_vpopcntdq rdpid movdiri movdir64b fsrm avx512_vp2intersect md
                         _clear flush_l1d arch_capabilities

Virtualization features: 

  Virtualization:        VT-x

Caches (sum of all):     

  L1d:                   192 KiB (4 instances)

  L1i:                   128 KiB (4 instances)

  L2:                    5 MiB (4 instances)

  L3:                    12 MiB (1 instance)

NUMA:                    
  NUMA node(s):          1

  NUMA node0 CPU(s):     0-7

Vulnerabilities:         
  Itlb multihit:         Not affected

  L1tf:                  Not affected

  Mds:                   Not affected

  Meltdown:              Not affected

  Mmio stale data:       Not affected

  Retbleed:              Not affected

  Spec store bypass:     Mitigation; Speculative Store Bypass disabled via prctl and sec

                         comp

  Spectre v1:            Mitigation; usercopy/swapgs barriers and __user pointer sanitiz

                         ation

  Spectre v2:            Mitigation; Enhanced IBRS, IBPB conditional, RSB filling, PBRSB

                         -eIBRS SW sequence

  Srbds:                 Not affected

  Tsx async abort:       Not affected




## Produit matrice-matrice



### Permutation des boucles

*Expliquer comment est compilé le code (ligne de make ou de gcc) : on aura besoin de savoir l'optim, les paramètres, etc. Par exemple :*

Par défaut, la commande make execute "help" qui affiche les différents modes de compilation (avec ou sans debug) et les différentes compilations.

Make all compilera TestProductMatrix.exe test_product_matrice_blas.exe calcul_pi.exe sous certaines configurations spécifiées (CXXFLAGS, ...)


#### Différence temps de calcul 1023, 1024, 1025

Le calcul sur les dimensions 1024 et 2048 (et leurs multiples) sont plus longues. Cela est dû à la taille du cache L3 (8 Mo) qui associe une adresse mémoire à un object modulo 2048. En supposant lire les valeurs d'une matrice en ligne, les valeurs de la dexième matrice servant à l'opération produit sont les valeurs sur les colonnes, deux valeurs consécutives en colonnes sont espacées de 1024 o. Alors, en 3e ligne, l'adresse mémoire associée est l'adresse 2048 équivalent à l'adresse 0 modulo 2048, on paye environ 20 coups d'horloge supplémentaire pour sauver cette valeur.

Il semble en réalité que les matrices soient stockées en colonnes (voir plus loin), mais cela ne change rien au raisonnement, le même problème se pose en permutant "colonnes" et "lignes".




`make TestProduct.exe && ./TestProduct.exe 1024`


  ordre           | time    | MFlops  | MFlops(n=2048) 
------------------|---------|---------|----------------
i,j,k (origine)   | 2.22192 | 966.5   |     44.7445            
j,i,k             | 2.30184 | 932.944 |     64.9367 
i,k,j             | 5.30343 | 404.923 |     150.641
k,i,j             | 5.21896 | 411.478 |     156.914
j,k,i             | 0.49080 | 4375.42 |     4.92595
k,j,i             | 0.51312 | 4185.09 |     6.38672


*Discussion des résultats*

La permutation optimale semble être (j,k,i) produisant un MFlops supérieur aux autres. Il semble plus généralement qu'exécuter la boucle sur i en dernière instance augpente largement les performances (performances 10x supérieures). 

Il apparaît que faire varier l'indice des lignes (indice i) de la première matrice en fixant en amont j et k donne une meilleure performance. Cela s'explique du fait que les matrices sont stockées en colonnes, le parcours des colonnes est donc moins coûteux (pas besoin d'accéder à la mémoire RAM à chaque fois, seulement au cache L3).

### OMP sur la meilleure boucle 

`make TestProduct.exe && OMP_NUM_THREADS=8 ./TestProduct.exe 1024`

  OMP_NUM         | MFlops  | MFlops(n=2048) | MFlops(n=512)  | MFlops(n=4096)
------------------|---------|----------------|----------------|---------------
1                 | 4125.84 |     2778.86    |    4346.07     |   2736.9
2                 | 8360.19 |     3203.97    |    8459.57     |   3400.08
3                 | 11745.2 |     3660.51    |1   1745.8      |   3326.34
4                 | 15686.2 |     3896.16    |    14490.9     |   3238.67
5                 | 10541.3 |     3452.65    |    10238.1     |   3180.22      
6                 | 12557.5 |     3574.15    |    12121.3     |   3172.24
7                 | 14498.6 |     3501.45    |    13628.3     |   3129.01
8                 | 16208.7 |     3311.5     |    12630.9     |   2997.31


L'acceleration moyenne (calculée sur Num_Thread pour n=1024) est de 1726.12 MFlops/Num_Threads.

#pragma omp for réparti les boucles dans les threads créés par #pragma omp parallel. Plus de threads disponible mène alors à plus de calcul parallélisé et donc une performance plus accrue.



### Produit par blocs

`make all && ./TestProductMatrix.exe 1024`

  szBlock         | MFlops  | MFlops(n=2048) | MFlops(n=512)  | MFlops(n=4096)
------------------|---------|----------------|----------------|---------------
ogine (=max)      | 3932.55 |     3394.42    |    3666.96     |   2997.31
32                | 3932.55 |     3394.42    |    3666.96     |   3249.52
64                | 4024.15 |    3855.39     |    4159.97     |   2903.13
128               | 4385.92 |   4284.03      |    4389.77     |   3286.98
256               | 4382.76 |   4394.29      |    4445.35     |   3801.09
512               | 4602.67 |   4361.46      |    4295.4      |   3827.63
1024              | 4584.03 |   3811.54      |    4619.23     |   3579.89


On constante, pour une dimension 1024 par exemple, une augmentation du MFlops pour des tailles de blocs aux alentour de 128, 256, on minimise le nombre de bloc créé.


### Bloc + OMP
  szBlock         | OMP_NUM |  MFlops  | MFlops(n=2048) | MFlops(n=512)  | MFlops(n=4096)
------------------|---------|----------|----------------|----------------|---------------
A.nbCols          |    1    | 4357.59  |  3494.9        |    4128.3      |    3488.6  
512               |    8    | 16192.5  |  15886.6       |    15715.7     |   12856.9
Speed-up          |         |    3.7   |  4.41          |     3.8        |      3.7    

Ainsi, en optimisant à la fois la taille des blocs et la parallélisation, on se se retrouve avec une accéleration 4 fois supérieure en moyenne. Là où pour une dimension 1024 l'impact est peut important ( MFlops de 16208.7 avec seulement la parallélisation), cette augmentation est conséquente pour les dimensions supérieures.


### Comparaison with BLAS

  szBlock         | OMP_NUM |  MFlops  | MFlops(n=2048) | MFlops(n=512)  | MFlops(n=4096)
------------------|---------|----------|----------------|----------------|---------------
512               |    8    | 5043.15  |  3645.23       |    5171.28     |    3585.89 

La meilleure solution semble être la première

# Tips 

```
	env 
	OMP_NUM_THREADS=4 ./produitMatriceMatrice.exe
```

```
    $ for i in $(seq 1 4); do elap=$(OMP_NUM_THREADS=$i ./TestProductOmp.exe|grep "Temps CPU"|cut -d " " -f 7); echo -e "$i\t$elap"; done > timers.out
```
