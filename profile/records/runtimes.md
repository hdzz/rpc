# Log of profiling runtime data.

This file contains records for runtime data of test parsers.

## 4th of October, 2015 4:08pm

Test runs of `sentence_parser.cpp` in `rpc` master branch running on OS X 10.11 w/ 2.5 GHz Intel Core i5; 8 GB 1600 MHz
DDR3. Compiled with `clang++ -std=c++14 -g0 -O2`.

- (4.4 KB file) Results from 10 runs of `$ ./profile/build/sentence_parser.out ./profile/data/sentences/sentences_small.txt`

```
4130 microsec.;
5025 microsec.;
4998 microsec.;
5212 microsec.;
5212 microsec.;
5184 microsec.;
5379 microsec.;
4858 microsec.;
5241 microsec.;
5358 microsec.;
```

- (43 KB file) Results from 10 runs of `$ ./profile/build/sentence_parser.out ./profile/data/sentences/sentences_medium.txt`

```
55097 microsec.;
54361 microsec.;
57488 microsec.;
51948 microsec.;
49991 microsec.;
50102 microsec.;
55661 microsec.;
54201 microsec.;
51496 microsec.;
51434 microsec.;
```

- (434 KB file) Results from 10 runs of `$ ./profile/build/sentence_parser.out ./profile/data/sentences/sentences_large.txt`

```
428140 microsec.;
441626 microsec.;
447848 microsec.;
435748 microsec.;
446460 microsec.;
448174 microsec.;
450047 microsec.;
457663 microsec.;
445190 microsec.;
441791 microsec.;
```

- (42 MB file) Results from 10 runs of `$ ./profile/build/sentence_parser.out ./profile/data/sentences/sentences_huge.txt`

```
42508120 microsec.;
42427265 microsec.;
40539093 microsec.;
40961065 microsec.;
40449992 microsec.;
40001358 microsec.;
42606130 microsec.;
40267996 microsec.;
42463641 microsec.;
41875144 microsec.;
```

Summary: Perfectly linear behavior in file size (~ number of sentences to parse).

