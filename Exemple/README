Comanda 1: ./quadtree -c 0 test0.ppm compress0_0.out
Output-ul este: compress0_0.out

Comanda 2: ./quadtree -c 370 test1.ppm compress1_370.out
Output-ul este: compress1_370.out

Comanda 3: ./quadtree -d compress1_370.out decompress1.ppm
Output-ul este: decompress1.ppm

Comanda 4: ./quadtree -m h 0 test0.ppm mirror0_h_0.ppm
Output-ul este: mirror0_h_0.ppm

Comanda 5: ./quadtree -m v 0 test0.ppm mirror0_v_0.ppm
Output-ul este: mirror0_v_0.ppm

Comanda 6: ./quadtree -m h 370 test1.ppm mirror1_h_370.ppm
Output-ul este: mirror1_h_370.ppm

Comanda 7: ./quadtree -m v 370 test1.ppm mirror1_v_370.ppm
Output-ul este: mirror1_v_370.ppm

Pentru a compara rezultatele, checker-ul foloseste utilitarul diff. Fisierele trebuie
sa fie identice la cerintele 2, 3, 4, iar la cerinta 1 se realizeaza decompresia,
pornind de la fisierul comprimat generat de executabilul vostru, si se compara cu imaginea
referinta.

Pentru primul exemplu, continutul text al fisierului binar este următorul:
colors=4
size=5
________________________________
index=0
146 108 99
65536
1 2 3 4
________________________________
index=1
97 57 105
16384
-1 -1 -1 -1
________________________________
index=2
184 246 67
16384
-1 -1 -1 -1
________________________________
index=3
99 69 59
16384
-1 -1 -1 -1
________________________________
index=4
204 60 167
16384
-1 -1 -1 -1
________________________________

Pentru fiecare element din vector sunt oferite informatiile in urmatoarea ordine:
indexul
red green blue
area
top_left top_right bottom_right bottom_left
