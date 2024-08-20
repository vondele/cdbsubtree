# cdbsubtree

count the number of positions reachable from a specific fen (at a specific depth) in a cdb data dump.

for `1. g4`:

```
Exploring fen: rnbqkbnr/pppppppp/8/8/6P1/8/PPPPPP1P/RNBQKBNR b KQkq - 0 1
Max depth: 20
Opening DB
sequential prepare
Reserving map of size 1073741824
Exploring tree
Done!
  Total number of DB gets: 2322436826
  Duration (sec) 3274.51
  DB gets per second: 709247
Number of cdb positions reachable from fen: 572502885
Detailed stats:
 ply           count  cumulative
   0 :            1            1
   1 :           20           21
   2 :          421          442
   3 :         5656         6098
   4 :        76958        83056
   5 :       419539       502595
   6 :      1604297      2106892
   7 :      4291986      6398878
   8 :      9270410     15669288
   9 :     17507041     33176329
  10 :     25802691     58979020
  11 :     33601388     92580408
  12 :     38368241    130948649
  13 :     43951732    174900381
  14 :     48696959    223597340
  15 :     55411197    279008537
  16 :     58134141    337142678
  17 :     57116278    394258956
  18 :     55847130    450106086
  19 :     59516674    509622760
  20 :     62880125    572502885
```

for `1. e4`:
```
Exploring fen: rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1
Max depth: 14
Opening DB
sequential prepare
Reserving map of size 1073741824
Exploring tree
Done!
  Total number of DB gets: 3607845267
  Duration (sec) 6084.23
  DB gets per second: 592982
Number of cdb positions reachable from fen: 1779890740
Detailed stats:
ply           count  cumulative
   0 :            1            1
   1 :           20           21
   2 :          600          621
   3 :         8062         8683
   4 :       142005       150688
   5 :       865620      1016308
   6 :      4120597      5136905
   7 :     13041195     18178100
   8 :     33268259     51446359
   9 :     70469664    121916023
  10 :    129981739    251897762
  11 :    210995621    462893383
  12 :    314935228    777828611
  13 :    435087217   1212915828
  14 :    566974912   1779890740
```

This tool requires a working instance of `cdbdirect`. See the
[cdbdirect](https://github.com/vondele/cdbdirect) repo for a description of the
[Chess Cloud Database (cdb)](https://chessdb.cn/queryc_en/) and how to access a
local copy.
