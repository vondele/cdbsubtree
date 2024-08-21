# cdbsubtree

count the number of positions reachable from a specific fen (at a specific depth) in a cdb data dump.

for `1. g4`:

```
Exploring fen: rnbqkbnr/pppppppp/8/8/6P1/8/PPPPPP1P/RNBQKBNR b KQkq - 0 1
Max depth: 40
Opening DB
sequential prepare
Reserving map of size 3221225472
Bucket count: 4294967040
Estimated memory use: 73014439680
Exploring tree
Done!
  Total number of DB gets: 22506118976
  Duration (sec) 27240.3
  DB gets per second: 826207
Number of cdb positions reachable from fen: 2572711175
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
  21 :     67325555    639828440
  22 :     71182001    711010441
  23 :     75569266    786579707
  24 :     78899369    865479076
  25 :     82071557    947550633
  26 :     84634885   1032185518
  27 :     86851692   1119037210
  28 :     89374628   1208411838
  29 :     93165035   1301576873
  30 :     96620357   1398197230
  31 :    100390202   1498587432
  32 :    104255033   1602842465
  33 :    108224128   1711066593
  34 :    112555174   1823621767
  35 :    116175032   1939796799
  36 :    119932966   2059729765
  37 :    119713945   2179443710
  38 :    123601223   2303044933
  39 :    130548826   2433593759
  40 :    139117416   2572711175
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
