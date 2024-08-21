# cdbsubtree

count the number of positions reachable from a specific fen (at a specific depth) in a cdb data dump.

for `1. g4`:

```
Exploring fen: rnbqkbnr/pppppppp/8/8/6P1/8/PPPPPP1P/RNBQKBNR b KQkq - 0 1
Max depth: 14
Allowing captures: 0
Opening DB
sequential prepare
Exploring tree

Iteration : 1 starting from 440 fens with 32 pieces
             iter time        iter count        total time       total count
               236.652         101118900           236.652         101118900
          iter DB gets    iter DB gets/s     total DB gets   total DB gets/s
             231925710            980029         231926172            980031
 ply        iter count   iter cumulative       total count  total cumulative
   0                 1                 1                 1                 1
   1                20                21                20                21
   2               419               440               419               440
   3              5489              5929              5489              5929
   4             72949             78878             72949             78878
   5            358662            437540            358662            437540
   6           1272576           1710116           1272576           1710116
   7           3009673           4719789           3009673           4719789
   8           5810106          10529895           5810106          10529895
   9           9745276          20275171           9745276          20275171
  10          12771540          33046711          12771540          33046711
  11          15150084          48196795          15150084          48196795
  12          16208403          64405198          16208403          64405198
  13          17835362          82240560          17835362          82240560
  14          18878340         101118900          18878340         101118900

Iteration : 2 starting from 23056232 fens with 31 pieces
             iter time        iter count        total time       total count
               155.949          60670421           392.988         161789321
          iter DB gets    iter DB gets/s     total DB gets   total DB gets/s
             135499981            868874         367426153            934956
 ply        iter count   iter cumulative       total count  total cumulative
   0                 0                 0                 1                 1
   1                 0                 0                20                21
   2                 2                 2               421               442
   3               166               168              5655              6097
   4              3938              4106             76887             82984
   5             57387             61493            416049            499033
   6            294848            356341           1567424           2066457
   7           1055335           1411676           4065008           6131465
   8           2644475           4056151           8454581          14586046
   9           5394936           9451087          15140212          29726258
  10           8337753          17788840          21109293          50835551
  11          10287433          28076273          25437517          76273068
  12          10968682          39044955          27177085         103450153
  13          10818584          49863539          28653946         132104099
  14          10806882          60670421          29685222         161789321

Iteration : 3 starting from 19568267 fens with 30 pieces
             iter time        iter count        total time       total count
               88.6883          40426263           482.604         202215584
          iter DB gets    iter DB gets/s     total DB gets   total DB gets/s
              75075480            846509         442501633            916904
 ply        iter count   iter cumulative       total count  total cumulative
   0                 0                 0                 1                 1
   1                 0                 0                20                21
   2                 0                 0               421               442
   3                 1                 1              5656              6098
   4                71                72             76958             83056
   5              3449              3521            419498            502554
   6             34613             38134           1602037           2104591
   7            206013            244147           4271021           6375612
   8            705178            949325           9159759          15535371
   9           1930383           2879708          17070595          32605966
  10           3577590           6457298          24686883          57292849
  11           5801844          12259142          31239361          88532210
  12           7349569          19608711          34526654         123058864
  13           9551154          29159865          38205100         161263964
  14          11266398          40426263          40951620         202215584

Iteration : 4 starting from 9896333 fens with 29 pieces
             iter time        iter count        total time       total count
               35.7174          15483705           519.193         217699289
          iter DB gets    iter DB gets/s     total DB gets   total DB gets/s
              28393133            794938         470894766            906973
 ply        iter count   iter cumulative       total count  total cumulative
   0                 0                 0                 1                 1
   1                 0                 0                20                21
   2                 0                 0               421               442
   3                 0                 0              5656              6098
   4                 0                 0             76958             83056
   5                41                41            419539            502595
   6              2205              2246           1604242           2106837
   7             19952             22198           4290973           6397810
   8            102321            124519           9262080          15659890
   9            390998            515517          17461593          33121483
  10            960578           1476095          25647461          58768944
  11           1933294           3409389          33172655          91941599
  12           2951651           6361040          37478305         129419904
  13           4069279          10430319          42274379         171694283
  14           5053386          15483705          46005006         217699289

Iteration : 5 starting from 3415870 fens with 28 pieces
             iter time        iter count        total time       total count
               10.5908           4598237           530.295         222297526
          iter DB gets    iter DB gets/s     total DB gets   total DB gets/s
               8009381            756261         478904147            903089
 ply        iter count   iter cumulative       total count  total cumulative
   0                 0                 0                 1                 1
   1                 0                 0                20                21
   2                 0                 0               421               442
   3                 0                 0              5656              6098
   4                 0                 0             76958             83056
   5                 0                 0            419539            502595
   6                55                55           1604297           2106892
   7               991              1046           4291964           6398856
   8              7954              9000           9270034          15668890
   9             42235             51235          17503828          33172718
  10            139376            190611          25786837          58959555
  11            367785            558396          33540440          92499995
  12            728083           1286479          38206388         130706383
  13           1314867           2601346          43589246         174295629
  14           1996891           4598237          48001897         222297526

Iteration : 6 starting from 877854 fens with 27 pieces
             iter time        iter count        total time       total count
               2.54959           1091572           533.279         223389098
          iter DB gets    iter DB gets/s     total DB gets   total DB gets/s
               1862756            730610         480766903            901529
 ply        iter count   iter cumulative       total count  total cumulative
   0                 0                 0                 1                 1
   1                 0                 0                20                21
   2                 0                 0               421               442
   3                 0                 0              5656              6098
   4                 0                 0             76958             83056
   5                 0                 0            419539            502595
   6                 0                 0           1604297           2106892
   7                22                22           4291986           6398878
   8               368               390           9270402          15669280
   9              3097              3487          17506925          33176205
  10             14911             18398          25801748          58977953
  11             55669             74067          33596109          92574062
  12            143232            217299          38349620         130923682
  13            308819            526118          43898065         174821747
  14            565454           1091572          48567351         223389098

Iteration : 7 starting from 165444 fens with 26 pieces
             iter time        iter count        total time       total count
              0.512325            180785           533.961         223569883
          iter DB gets    iter DB gets/s     total DB gets   total DB gets/s
                304436            594224         481071339            900947
 ply        iter count   iter cumulative       total count  total cumulative
   0                 0                 0                 1                 1
   1                 0                 0                20                21
   2                 0                 0               421               442
   3                 0                 0              5656              6098
   4                 0                 0             76958             83056
   5                 0                 0            419539            502595
   6                 0                 0           1604297           2106892
   7                 0                 0           4291986           6398878
   8                 8                 8           9270410          15669288
   9               115               123          17507040          33176328
  10               907              1030          25802655          58978983
  11              4963              5993          33601072          92580055
  12             17056             23049          38366676         130946731
  13             47149             70198          43945214         174891945
  14            110587            180785          48677938         223569883

Iteration : 8 starting from 23628 fens with 25 pieces
             iter time        iter count        total time       total count
             0.0960406             25408           534.121         223595291
          iter DB gets    iter DB gets/s     total DB gets   total DB gets/s
                 41304            430068         481112643            900755
 ply        iter count   iter cumulative       total count  total cumulative
   0                 0                 0                 1                 1
   1                 0                 0                20                21
   2                 0                 0               421               442
   3                 0                 0              5656              6098
   4                 0                 0             76958             83056
   5                 0                 0            419539            502595
   6                 0                 0           1604297           2106892
   7                 0                 0           4291986           6398878
   8                 0                 0           9270410          15669288
   9                 1                 1          17507041          33176329
  10                36                37          25802691          58979020
  11               307               344          33601379          92580399
  12              1484              1828          38368160         130948559
  13              6120              7948          43951334         174899893
  14             17460             25408          48695398         223595291

Iteration : 9 starting from 2192 fens with 24 pieces
             iter time        iter count        total time       total count
             0.0350777              1973           534.206         223597264
          iter DB gets    iter DB gets/s     total DB gets   total DB gets/s
                  3377             96272         481116020            900618
 ply        iter count   iter cumulative       total count  total cumulative
   0                 0                 0                 1                 1
   1                 0                 0                20                21
   2                 0                 0               421               442
   3                 0                 0              5656              6098
   4                 0                 0             76958             83056
   5                 0                 0            419539            502595
   6                 0                 0           1604297           2106892
   7                 0                 0           4291986           6398878
   8                 0                 0           9270410          15669288
   9                 0                 0          17507041          33176329
  10                 0                 0          25802691          58979020
  11                 9                 9          33601388          92580408
  12                76                85          38368236         130948644
  13               385               470          43951719         174900363
  14              1503              1973          48696901         223597264

Iteration : 10 starting from 97 fens with 23 pieces
             iter time        iter count        total time       total count
             0.0157288                63           534.264         223597327
          iter DB gets    iter DB gets/s     total DB gets   total DB gets/s
                   113              7184         481116133            900521
 ply        iter count   iter cumulative       total count  total cumulative
   0                 0                 0                 1                 1
   1                 0                 0                20                21
   2                 0                 0               421               442
   3                 0                 0              5656              6098
   4                 0                 0             76958             83056
   5                 0                 0            419539            502595
   6                 0                 0           1604297           2106892
   7                 0                 0           4291986           6398878
   8                 0                 0           9270410          15669288
   9                 0                 0          17507041          33176329
  10                 0                 0          25802691          58979020
  11                 0                 0          33601388          92580408
  12                 5                 5          38368241         130948649
  13                11                16          43951730         174900379
  14                47                63          48696948         223597327

Iteration : 11 starting from 13 fens with 22 pieces
             iter time        iter count        total time       total count
             0.0107763                11            534.31         223597338
          iter DB gets    iter DB gets/s     total DB gets   total DB gets/s
                    21              1948         481116154            900443
 ply        iter count   iter cumulative       total count  total cumulative
   0                 0                 0                 1                 1
   1                 0                 0                20                21
   2                 0                 0               421               442
   3                 0                 0              5656              6098
   4                 0                 0             76958             83056
   5                 0                 0            419539            502595
   6                 0                 0           1604297           2106892
   7                 0                 0           4291986           6398878
   8                 0                 0           9270410          15669288
   9                 0                 0          17507041          33176329
  10                 0                 0          25802691          58979020
  11                 0                 0          33601388          92580408
  12                 0                 0          38368241         130948649
  13                 2                 2          43951732         174900381
  14                 9                11          48696957         223597338

Iteration : 12 starting from 3 fens with 21 pieces
             iter time        iter count        total time       total count
            0.00992937                 2           534.355         223597340
          iter DB gets    iter DB gets/s     total DB gets   total DB gets/s
                     3               302         481116157            900368
 ply        iter count   iter cumulative       total count  total cumulative
   0                 0                 0                 1                 1
   1                 0                 0                20                21
   2                 0                 0               421               442
   3                 0                 0              5656              6098
   4                 0                 0             76958             83056
   5                 0                 0            419539            502595
   6                 0                 0           1604297           2106892
   7                 0                 0           4291986           6398878
   8                 0                 0           9270410          15669288
   9                 0                 0          17507041          33176329
  10                 0                 0          25802691          58979020
  11                 0                 0          33601388          92580408
  12                 0                 0          38368241         130948649
  13                 0                 0          43951732         174900381
  14                 2                 2          48696959         223597340

Finished all iterations! 
Closing DB
```

This tool requires a working instance of `cdbdirect`. See the
[cdbdirect](https://github.com/vondele/cdbdirect) repo for a description of the
[Chess Cloud Database (cdb)](https://chessdb.cn/queryc_en/) and how to access a
local copy.
