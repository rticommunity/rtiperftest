# RTI Perftest Output Formats

## Introduction and goals

The aim of this project is to change the view of all the data printed during the execution of
 RTI Perftest in the stdout. This can be very useful because
 it's easier to work with differents formats, such as csv or json.

There are new flags for manage:
- `-outputFormat {csv, json, legacy}`
- `-noPrintHeaders`. Delete the row with headers of intervals and summaries.

Examples:

## LEGACY

## Regular Test -- printing intervals and only one size (no scan)

### Publisher side

```
********** New data length is 100
One way Latency:     69 us Ave    287 us Std  218.0 us Min     69 us Max    505 us
One way Latency:     92 us Ave    222 us Std  200.3 us Min     69 us Max    505 us
One way Latency:     47 us Ave    178 us Std  189.3 us Min     47 us Max    505 us
One way Latency:     53 us Ave    153 us Std  176.6 us Min     47 us Max    505 us
One way Latency:     53 us Ave    136 us Std  165.5 us Min     47 us Max    505 us
One way Latency:    138 us Ave    137 us Std  153.2 us Min     47 us Max    505 us
One way Latency:     53 us Ave    126 us Std  146.0 us Min     47 us Max    505 us
Length:   100 Latency: Ave    126 us Std  146.0 us Min     47 us Max    505 us 50%     69 us 90%    505 us 99%    505 us 99.99%    505 us 99.9999%    505 us
```

### Subscriber side

```
********** New data length is 100
Packets:  2336445  Packets/s: 2209783  Packets/s(ave): 2209783  Mbps:  1767.8  Mbps(ave):  1767.8  Lost:     0 (0.00%)
Packets:  4418793  Packets/s: 2082150  Packets/s(ave): 2145966  Mbps:  1665.7  Mbps(ave):  1716.8  Lost:     0 (0.00%)
Length:   100  Packets:  6163858  Packets/s(ave): 2055011  Mbps(ave):  1644.0  Lost:     0 (0.00%)

```

## Scan Test -- NO printing intervals

### Publisher side

```
Length:    32 Latency: Ave   1189 us Std 2337.3 us Min     94 us Max   6414 us 50%    174 us 90%   6414 us 99%   6414 us 99.99%   6414 us 99.9999%   6414 us
Length:    64 Latency: Ave    296 us Std  238.3 us Min     72 us Max    636 us 50%    404 us 90%    636 us 99%    636 us 99.99%    636 us 99.9999%    636 us
```

### Subscriber side

```
Length:    32  Packets:  7680006  Packets/s(ave): 2406024  Mbps(ave):   615.9  Lost:     0 (0.00%)
Length:    64  Packets:  5120004  Packets/s(ave): 1882454  Mbps(ave):   963.8  Lost:     0 (0.00%)
```

## Scan Test -- printing intervals + -scan

### Publisher side

```
********** New data length is 32
One way Latency:    180 us Ave    548 us Std  367.5 us Min    180 us Max    915 us
One way Latency:    101 us Ave    399 us Std  366.5 us Min    101 us Max    915 us
One way Latency:     97 us Ave    323 us Std  343.2 us Min     97 us Max    915 us
One way Latency:     94 us Ave    277 us Std  320.4 us Min     94 us Max    915 us
One way Latency:    101 us Ave    248 us Std  299.8 us Min     94 us Max    915 us
One way Latency:    101 us Ave    227 us Std  282.3 us Min     94 us Max    915 us
One way Latency:    100 us Ave    211 us Std  267.4 us Min     94 us Max    915 us
One way Latency:     95 us Ave    198 us Std  254.7 us Min     94 us Max    915 us
One way Latency:     98 us Ave    188 us Std  243.5 us Min     94 us Max    915 us
One way Latency:    188 us Ave    188 us Std  232.2 us Min     94 us Max    915 us
Length:    32 Latency: Ave    188 us Std  232.2 us Min     94 us Max    915 us 50%    101 us 90%    188 us 99%    915 us 99.99%    915 us 99.9999%    915 us


********** New data length is 64
One way Latency:     81 us Ave    266 us Std  184.5 us Min     81 us Max    450 us
One way Latency:    662 us Ave    398 us Std  240.1 us Min     81 us Max    662 us
One way Latency:     77 us Ave    318 us Std  250.0 us Min     77 us Max    662 us
One way Latency:    148 us Ave    284 us Std  233.7 us Min     77 us Max    662 us
One way Latency:     80 us Ave    250 us Std  226.4 us Min     77 us Max    662 us
One way Latency:     79 us Ave    225 us Std  217.9 us Min     77 us Max    662 us
One way Latency:     79 us Ave    207 us Std  209.5 us Min     77 us Max    662 us
Length:    64 Latency: Ave    207 us Std  209.5 us Min     77 us Max    662 us 50%     81 us 90%    662 us 99%    662 us 99.99%    662 us 99.9999%    662 us
```

### Subscriber side

```
********** New data length is 32
Packets:  5286144  Packets/s: 2678745  Packets/s(ave): 2678745  Mbps:   685.8  Mbps(ave):   685.8  Lost:     0 (0.00%) CPU 12.94 (%)
Packets:  7986688  Packets/s: 2700298  Packets/s(ave): 2689522  Mbps:   691.3  Mbps(ave):   688.5  Lost:     0 (0.00%) CPU 24.25 (%)
Packets: 10574848  Packets/s: 2587774  Packets/s(ave): 2655606  Mbps:   662.5  Mbps(ave):   679.8  Lost:     0 (0.00%) CPU 24.00 (%)
Packets: 13273600  Packets/s: 2698514  Packets/s(ave): 2666333  Mbps:   690.8  Mbps(ave):   682.6  Lost:     0 (0.00%) CPU 24.25 (%)
Length:    32  Packets: 14080011  Packets/s(ave): 2641911  Mbps(ave):   676.3  Lost:     0 (0.00%) CPU 21.36 (%)


********** New data length is 64
Packets:  4451456  Packets/s: 2275954  Packets/s(ave): 2275954  Mbps:  1165.3  Mbps(ave):  1165.3  Lost:     0 (0.00%) CPU 18.21 (%)
Packets:  6843648  Packets/s: 2391974  Packets/s(ave): 2333964  Mbps:  1224.7  Mbps(ave):  1195.0  Lost:     0 (0.00%) CPU 23.75 (%)
Packets:  9304960  Packets/s: 2461092  Packets/s(ave): 2376340  Mbps:  1260.1  Mbps(ave):  1216.7  Lost:     0 (0.00%) CPU 23.25 (%)
Length:    64  Packets: 10240008  Packets/s(ave): 2338005  Mbps(ave):  1197.1  Lost:     0 (0.00%) CPU 21.74 (%)
```

# CSV

## Regular Test -- printing intervals and only one size (no scan)

### Publisher side

```
Intervals One-way Latency for 100 Bytes:
Length (Bytes), Latency (us), Ave (us), Std (us), Min (us), Max (us)
           100,           92,      320,    228.5,       92,      549
           100,           54,      232,    224.9,       54,      549
           100,           51,      186,    209.9,       51,      549
           100,           54,      160,    195.1,       51,      549
           100,           88,      148,    180.1,       51,      549
           100,           51,      134,    170.2,       51,      549
           100,          902,      230,    299.7,       51,      902

One-way Latency Summary:
Length (Bytes), Ave (us), Std (us), Min (us), Max (us), 50% (us), 90% (us), 99% (us), 99.99% (us), 99.9999% (us)
           100,      230,    299.7,       51,      902,       88,      902,      902,         902,           902
```

### Subscriber side

```
Interval Throughput for 100 Bytes:
Length (Bytes), Total Samples,  Samples/s, Ave Samples/s,     Mbps,  Ave Mbps, Lost Samples, Lost Samples (%)
           100,       3583926,    2046014,       2046014,   1636.8,    1636.8,            0,             0.00
           100,       5656959,    2072794,       2059404,   1658.2,    1647.5,            0,             0.00

Throughput Summary:
Length (Bytes), Total Samples, Ave Samples/s,    Ave Mbps, Lost Samples, Lost Samples (%)
           100,       5887953,       1963048,      1570.4,            0,             0.00
```

## Regular Test -- NOT printing intervals and only one size (no scan)

### Publisher side

```
One-way Latency Summary:
Length (Bytes), Ave (us), Std (us), Min (us), Max (us), 50% (us), 90% (us), 99% (us), 99.99% (us), 99.9999% (us)
           100,       88,     86.8,       49,      317,       54,      317,      317,         317,           317
```

### Subscriber side

```
Throughput Summary:
Length (Bytes), Total Samples, Ave Samples/s,    Ave Mbps, Lost Samples, Lost Samples (%)
           100,       5729374,       1910033,      1528.0,            0,             0.00
```

## Scan Test -- printing intervals + -scan

### Publisher side

```
Intervals One-way Latency for 32 Bytes:
Length (Bytes), Latency (us), Ave (us), Std (us), Min (us), Max (us)
            32,          101,      580,    479.0,      101,     1059
            32,          156,      439,    439.2,      101,     1059
            32,          762,      520,    405.3,      101,     1059
            32,          100,      436,    399.5,      100,     1059
            32,          102,      380,    385.3,      100,     1059
            32,           96,      339,    370.3,       96,     1059
            32,          115,      311,    354.2,       96,     1059
            32,          102,      288,    340.4,       96,     1059
            32,           94,      269,    328.1,       94,     1059
            32,          191,      262,    313.7,       94,     1059
            32,          100,      248,    303.6,       94,     1059
            32,           94,      236,    294.6,       94,     1059
            32,           93,      226,    286.3,       93,     1059
            32,          114,      219,    278.0,       93,     1059
            32,          904,      261,    316.2,       93,     1059
            32,          204,      258,    307.0,       93,     1059
            32,           94,      249,    300.7,       93,     1059
            32,          109,      242,    294.4,       93,     1059

One-way Latency Summary:
Length (Bytes), Ave (us), Std (us), Min (us), Max (us), 50% (us), 90% (us), 99% (us), 99.99% (us), 99.9999% (us)
            32,      242,    294.4,       93,     1059,      102,      904,     1059,        1059,          1059

Intervals One-way Latency for 64 Bytes:
Length (Bytes), Latency (us), Ave (us), Std (us), Min (us), Max (us)
            64,           88,      216,    128.5,       88,      345
            64,           85,      173,    121.9,       85,      345
            64,           60,      144,    116.3,       60,      345
            64,           68,      129,    108.4,       60,      345
            64,           64,      118,    101.9,       60,      345
            64,           63,      110,     96.3,       60,      345
            64,           62,      104,     91.5,       60,      345
            64,           81,      102,     86.6,       60,      345
            64,           66,       98,     82.8,       60,      345
            64,           59,       95,     79.8,       59,      345
            64,           60,       92,     77.0,       59,      345
            64,           61,       89,     74.4,       59,      345
            64,           58,       87,     72.2,       58,      345
            64,          136,       90,     70.8,       58,      345
            64,           90,       90,     68.5,       58,      345
            64,           61,       89,     66.8,       58,      345

One-way Latency Summary:
Length (Bytes), Ave (us), Std (us), Min (us), Max (us), 50% (us), 90% (us), 99% (us), 99.99% (us), 99.9999% (us)
            64,       89,     66.8,       58,      345,       64,      136,      345,         345,           345

Intervals One-way Latency for 128 Bytes:
Length (Bytes), Latency (us), Ave (us), Std (us), Min (us), Max (us)
           128,           47,      138,     90.5,       47,      228
           128,          125,      133,     74.1,       47,      228
           128,          125,      131,     64.3,       47,      228
           128,           92,      123,     59.6,       47,      228
           128,           84,      117,     56.4,       47,      228
           128,           47,      107,     57.6,       47,      228
           128,           47,       99,     57.4,       47,      228
           128,           83,       98,     54.4,       47,      228
           128,           46,       92,     53.9,       46,      228
           128,          375,      118,     96.1,       46,      375
           128,           46,      112,     94.2,       46,      375
           128,           44,      107,     92.3,       44,      375

One-way Latency Summary:
Length (Bytes), Ave (us), Std (us), Min (us), Max (us), 50% (us), 90% (us), 99% (us), 99.99% (us), 99.9999% (us)
           128,      107,     92.3,       44,      375,       83,      228,      375,         375,           375
```

### Subscriber side

```
Interval Throughput for 32 Bytes:
Length (Bytes), Total Samples,  Samples/s, Ave Samples/s,     Mbps,  Ave Mbps, Lost Samples, Lost Samples (%)
            32,       5262592,    2712394,       2712394,    694.4,     694.4,            0,             0.00
            32,       7854080,    2591060,       2651727,    663.3,     678.8,            0,             0.00
            32,      10150912,    2296600,       2533351,    587.9,     648.5,            0,             0.00

Throughput Summary:
Length (Bytes), Total Samples, Ave Samples/s,    Ave Mbps, Lost Samples, Lost Samples (%)
            32,      12160019,       2529739,       647.6,            0,             0.00

Interval Throughput for 64 Bytes:
Length (Bytes), Total Samples,  Samples/s, Ave Samples/s,     Mbps,  Ave Mbps, Lost Samples, Lost Samples (%)
            64,       4301440,    2286340,       2286340,   1170.6,    1170.6,            0,             0.00
            64,       6589568,    2287860,       2287100,   1171.4,    1171.0,            0,             0.00
            64,       8836864,    2247093,       2273764,   1150.5,    1164.2,            0,             0.00

Throughput Summary:
Length (Bytes), Total Samples, Ave Samples/s,    Ave Mbps, Lost Samples, Lost Samples (%)
            64,      10880017,       2198978,      1125.9,            0,             0.00

Interval Throughput for 128 Bytes:
Length (Bytes), Total Samples,  Samples/s, Ave Samples/s,     Mbps,  Ave Mbps, Lost Samples, Lost Samples (%)
           128,       3066496,    1540065,       1540065,   1577.0,    1577.0,            0,             0.00
           128,       4843904,    1777235,       1658650,   1819.9,    1698.5,            0,             0.00
           128,       6534464,    1690319,       1669206,   1730.9,    1709.3,            0,             0.00
           128,       8275712,    1741059,       1687170,   1782.8,    1727.7,            0,             0.00

Throughput Summary:
Length (Bytes), Total Samples, Ave Samples/s,    Ave Mbps, Lost Samples, Lost Samples (%)
           128,       8320013,       1652036,      1691.7,            0,             0.00
```

## Scan Test -- NOT printing intervals + -scan

### Publisher side

```
One-way Latency Summary:
Length (Bytes), Ave (us), Std (us), Min (us), Max (us), 50% (us), 90% (us), 99% (us), 99.99% (us), 99.9999% (us)
            32,      386,    791.8,       86,     3398,       99,     1677,     3398,        3398,          3398
            64,       77,     49.4,       59,      287,       62,      109,      287,         287,           287
           128,       75,     70.3,       43,      316,       50,       77,      316,         316,           316
```

### Subscriber side

```
Throughput Summary:
Length (Bytes), Total Samples, Ave Samples/s,    Ave Mbps, Lost Samples, Lost Samples (%)
            32,      12160019,       2507555,       641.9,            0,             0.00
            64,      12800020,       2548247,      1304.7,            0,             0.00
           128,       8320013,       1923979,      1970.2,            0,             0.00
```

## Regular Test -- printing intervals and show cpu

### Publisher side

```
ntervals One-way Latency for 100 Bytes:
Length (Bytes), Latency (us), Ave (us), Std (us), Min (us), Max (us), CPU %
           100,           67,      306,    238.5,       67,      544, 10.37
           100,           52,      221,    228.5,       52,      544, 28.12
           100,           45,      177,    212.0,       45,      544, 27.84
           100,           50,      152,    196.3,       45,      544, 28.05
           100,           82,      140,    181.1,       45,      544, 27.91
           100,           51,      127,    170.5,       45,      544, 27.84
           100,           54,      118,    161.3,       45,      544, 28.29
           100,           51,      111,    153.6,       45,      544, 28.12
           100,           67,      106,    146.3,       45,      544, 28.95
           100,           74,      103,    139.8,       45,      544, 27.56
           100,           55,       99,    134.5,       45,      544, 28.38
           100,           88,       98,    129.3,       45,      544, 28.21

One-way Latency Summary:
Length (Bytes), Ave (us), Std (us), Min (us), Max (us), 50% (us), 90% (us), 99% (us), 99.99% (us), 99.9999% (us), CPU %
           100,       98,    129.3,       45,      544,       55,       88,      544,         544,           544, 26.64
```

### Subscriber side

```
Interval Throughput for 100 Bytes:
Length (Bytes), Total Samples,  Samples/s, Ave Samples/s,     Mbps,  Ave Mbps, Lost Samples, Lost Samples (%), CPU %
           100,       3591398,    1908340,       1908340,   1526.7,    1526.7,            0,             0.00, 12.12
           100,       5548986,    1957341,       1932840,   1565.9,    1546.3,            0,             0.00, 22.75
           100,       7576092,    2026889,       1964190,   1621.5,    1571.4,            0,             0.00, 23.50
           100,       9729992,    2153671,       2011560,   1722.9,    1609.2,            0,             0.00, 23.00

Throughput Summary:
Length (Bytes), Total Samples, Ave Samples/s,    Ave Mbps, Lost Samples, Lost Samples (%), CPU %
           100,       9772745,       1954063,      1563.3,            0,             0.00, 20.34
```

## Regular Test -- printing intervals, show cpu and show serialization time

### Publisher side

```
Intervals One-way Latency for 100 Bytes:
Length (Bytes), Latency (us), Ave (us), Std (us), Min (us), Max (us), CPU %
           100,           74,      142,     68.0,       74,      210,  9.27
           100,           51,      112,     70.2,       51,      210, 27.50
           100,           62,       99,     64.5,       51,      210, 28.38
           100,           50,       89,     60.9,       50,      210, 27.98
           100,           46,       82,     57.9,       46,      210, 28.66
           100,           53,       78,     54.6,       46,      210, 27.50
           100,           90,       80,     51.2,       46,      210, 27.98
           100,           51,       76,     49.1,       46,      210, 28.38
           100,           73,       76,     46.6,       46,      210, 26.83
           100,           50,       74,     45.1,       46,      210, 29.05
           100,           48,       72,     43.7,       46,      210, 27.70
           100,           50,       70,     42.4,       46,      210, 27.98

One-way Latency Summary:
Length (Bytes), Ave (us), Std (us), Min (us), Max (us), 50% (us), 90% (us), 99% (us), 99.99% (us), 99.9999% (us), Serialization (us), Deserialization (us), Total (us), CPU %
           100,       70,     42.4,       46,      210,       51,       90,      210,         210,           210,              0.343,                0.324,      0.667, 26.43
```

### Subscriber side

```
Interval Throughput for 100 Bytes:
Length (Bytes), Total Samples,  Samples/s, Ave Samples/s,     Mbps,  Ave Mbps, Lost Samples, Lost Samples (%), CPU %
           100,       3082293,    2086856,       2086856,   1669.5,    1669.5,            0,             0.00, 12.83
           100,       5053023,    1970385,       2028620,   1576.3,    1622.9,            0,             0.00, 23.00
           100,       7156998,    2103777,       2053673,   1683.0,    1642.9,            0,             0.00, 23.00
           100,       9214965,    2057637,       2054664,   1646.1,    1643.7,            0,             0.00, 23.25

Throughput Summary:
Length (Bytes), Total Samples, Ave Samples/s,    Ave Mbps, Lost Samples, Lost Samples (%), CPU %
           100,      10215733,       2043193,      1634.6,            0,             0.00, 20.52
```

## Scan Test -- noPrintHeaders, just print intervals

### Publisher side

```
            32,          120,     2222,   2102.5,      120,     4325
            32,          118,     1521,   1982.7,      118,     4325
            32,           96,     1165,   1824.6,       96,     4325
            32,          100,      952,   1686.6,       96,     4325
            32,          117,      813,   1570.8,       96,     4325
            32,           97,      710,   1475.7,       96,     4325
            32,           96,      634,   1395.2,       96,     4325
            32,           99,      574,   1326.1,       96,     4325
            32,      574,   1326.1,       96,     4325,      100,     4325,     4325,        4325,          4325
            64,           64,      231,    167.0,       64,      398
            64,           59,      174,    158.6,       59,      398
            64,           68,      147,    144.8,       59,      398
            64,           83,      134,    132.0,       59,      398
            64,           70,      124,    122.9,       59,      398
            64,           61,      115,    115.9,       59,      398
            64,          134,      117,    108.6,       59,      398
            64,           60,      111,    103.9,       59,      398
            64,      111,    103.9,       59,      398,       68,      398,      398,         398,           398

```

### Subscriber side

```
            32,       4420352,    2118012,       2118012,    542.2,     542.2,            0,             0.00
            32,       6669824,    2249222,       2183617,    575.8,     559.0,            0,             0.00
            32,       9379840,    2709777,       2359004,    693.7,     603.9,            0,             0.00
            32,      11520009,       2393652,       612.8,            0,             0.00
            64,       4344576,    2460007,       2460007,   1259.5,    1259.5,            0,             0.00
            64,       6576256,    2231434,       2345720,   1142.5,    1201.0,            0,             0.00
            64,       8841472,    2265003,       2318815,   1159.7,    1187.2,            0,             0.00
            64,      10993024,    2151351,       2276949,   1101.5,    1165.8,            0,             0.00
            64,      11520009,       2208087,      1130.5,            0,             0.00
```
# JSON

## Regular Test -- printing intervals and only one size

### Publisher side

```
{"perftest":
	[
		{
			"length":100,
			"intervals":[

				{
					"latency": 2161,
					"latency_ave": 1361.00,
					"latency_std": 800.00,
					"latency_min": 561,
					"latency_max": 2161
				},
				{
					"latency": 54,
					"latency_ave": 925.33,
					"latency_std": 897.93,
					"latency_min": 54,
					"latency_max": 2161
				},
				{
					"latency": 54,
					"latency_ave": 707.50,
					"latency_std": 864.33,
					"latency_min": 54,
					"latency_max": 2161
				},
				{
					"latency": 64,
					"latency_ave": 578.80,
					"latency_std": 814.80,
					"latency_min": 54,
					"latency_max": 2161
				},
				{
					"latency": 57,
					"latency_ave": 491.83,
					"latency_std": 768.81,
					"latency_min": 54,
					"latency_max": 2161
				},
				{
					"latency": 53,
					"latency_ave": 429.14,
					"latency_std": 728.16,
					"latency_min": 53,
					"latency_max": 2161
				},
				{
					"latency": 57,
					"latency_ave": 382.62,
					"latency_std": 692.16,
					"latency_min": 53,
					"latency_max": 2161
				}
			],
			"summary":{
				"latency_ave": 382.62,
				"latency_std": 692.16,
				"latency_min": 53,
				"latency_max": 2161,
				"latency_50": 57,
				"latency_90": 2161,
				"latency_99": 2161,
				"latency_99.99": 2161,
				"latency_99.9999": 2161
			}
		}
	]
}
```

### Subscriber side

```
{"perftest":
	[
		{
			"length":100,
			"intervals":[

				{
					"length": 100,
					"packets": 2312550,
					"packets/s": 2018963,
					"packets/s_ave": 2018963.00,
					"mbps": 1615.2,
					"mbps_ave": 1615.2,
					"lost": 0,
					"lost_percent": 0.00
				},
				{
					"length": 100,
					"packets": 4456458,
					"packets/s": 2143640,
					"packets/s_ave": 2081301.50,
					"mbps": 1714.9,
					"mbps_ave": 1665.0,
					"lost": 0,
					"lost_percent": 0.00
				}			],
			"summary":{
				"packets": 6164888,
				"packets/s_ave": 2055211,
				"mbps_ave": 1644.2,
				"lost": 0,
				"lost_percent": 0.00
			}
		}
	]
}
```

## Regular Test -- NOT printing intervals and only one size (no scan)

### Publisher side

```
{"perftest":
	[
		{
			"length":100,
			"summary":{
				"latency_ave": 71.38,
				"latency_std": 34.67,
				"latency_min": 51,
				"latency_max": 159,
				"latency_50": 55,
				"latency_90": 159,
				"latency_99": 159,
				"latency_99.99": 159,
				"latency_99.9999": 159
			}
		}
	]
}

```

### Subscriber side

```
{"perftest":
	[
		{
			"length":100,
			"summary":{
				"packets": 6065896,
				"packets/s_ave": 2022067,
				"mbps_ave": 1617.7,
				"lost": 0,
				"lost_percent": 0.00
			}
		}
	]
}
```

## Regular Test -- printing intervals, show cpu and show serialization time

### Publisher side

```
{"perftest":
	[
		{
			"length":100,
			"intervals":[

				{
					"latency": 72,
					"latency_ave": 372.00,
					"latency_std": 300.00,
					"latency_min": 72,
					"latency_max": 672,
					"cpu": 9.56
				},
				{
					"latency": 52,
					"latency_ave": 265.33,
					"latency_std": 287.67,
					"latency_min": 52,
					"latency_max": 672,
					"cpu": 27.44
				},
				{
					"latency": 76,
					"latency_ave": 218.00,
					"latency_std": 262.27,
					"latency_min": 52,
					"latency_max": 672,
					"cpu": 28.29
				},
				{
					"latency": 52,
					"latency_ave": 184.80,
					"latency_std": 243.80,
					"latency_min": 52,
					"latency_max": 672,
					"cpu": 28.12
				},
				{
					"latency": 53,
					"latency_ave": 162.83,
					"latency_std": 227.92,
					"latency_min": 52,
					"latency_max": 672,
					"cpu": 28.21
				},
				{
					"latency": 53,
					"latency_ave": 147.14,
					"latency_std": 214.48,
					"latency_min": 52,
					"latency_max": 672,
					"cpu": 27.70
				},
				{
					"latency": 526,
					"latency_ave": 194.50,
					"latency_std": 236.54,
					"latency_min": 52,
					"latency_max": 672,
					"cpu": 27.98
				}
			],
			"summary":{
				"latency_ave": 194.50,
				"latency_std": 236.54,
				"latency_min": 52,
				"latency_max": 672,
				"latency_50": 72,
				"latency_90": 672,
				"latency_99": 672,
				"latency_99.99": 672,
				"latency_99.9999": 672,
				"serialize": 0.577,
				"deserialize": 0.485,
				"total_s": 1.062,
				"cpu": 25.33
			}
		}
	]
}
```

### Subscriber side

```
{"perftest":
	[
		{
			"length":100,
			"intervals":[

				{
					"length": 100,
					"packets": 2762019,
					"packets/s": 2020500,
					"packets/s_ave": 2020500.00,
					"mbps": 1616.4,
					"mbps_ave": 1616.4,
					"lost": 0,
					"lost_percent": 0.00,
					"cpu": 11.25
				},
				{
					"length": 100,
					"packets": 4904226,
					"packets/s": 2141823,
					"packets/s_ave": 2081161.50,
					"mbps": 1713.5,
					"mbps_ave": 1664.9,
					"lost": 0,
					"lost_percent": 0.00,
					"cpu": 22.75
				}			],
			"summary":{
				"packets": 6066172,
				"packets/s_ave": 2022647,
				"mbps_ave": 1618.1,
				"lost": 0,
				"lost_percent": 0.00,
				"cpu": 17.00
			}
		}
	]
}
```

## Scan Test -- printing intervals + -scan

### Publisher side

```
{"perftest":
	[
		{
			"length":32,
			"intervals":[

				{
					"latency": 97,
					"latency_ave": 1727.00,
					"latency_std": 1630.00,
					"latency_min": 97,
					"latency_max": 3357
				},
				{
					"latency": 100,
					"latency_ave": 1184.67,
					"latency_std": 1536.07,
					"latency_min": 97,
					"latency_max": 3357
				},
				{
					"latency": 108,
					"latency_ave": 915.50,
					"latency_std": 1409.61,
					"latency_min": 97,
					"latency_max": 3357
				},
				{
					"latency": 124,
					"latency_ave": 757.20,
					"latency_std": 1299.93,
					"latency_min": 97,
					"latency_max": 3357
				},
				{
					"latency": 97,
					"latency_ave": 647.17,
					"latency_std": 1211.91,
					"latency_min": 97,
					"latency_max": 3357
				},
				{
					"latency": 106,
					"latency_ave": 569.86,
					"latency_std": 1137.88,
					"latency_min": 97,
					"latency_max": 3357
				}
			],
			"summary":{
				"latency_ave": 569.86,
				"latency_std": 1137.88,
				"latency_min": 97,
				"latency_max": 3357,
				"latency_50": 106,
				"latency_90": 3357,
				"latency_99": 3357,
				"latency_99.99": 3357,
				"latency_99.9999": 3357
			}
		},
		{
			"length":64,
			"intervals":[

				{
					"latency": 113,
					"latency_ave": 360.00,
					"latency_std": 247.00,
					"latency_min": 113,
					"latency_max": 607
				},
				{
					"latency": 68,
					"latency_ave": 262.67,
					"latency_std": 244.17,
					"latency_min": 68,
					"latency_max": 607
				},
				{
					"latency": 69,
					"latency_ave": 214.25,
					"latency_std": 227.48,
					"latency_min": 68,
					"latency_max": 607
				},
				{
					"latency": 71,
					"latency_ave": 185.60,
					"latency_std": 211.38,
					"latency_min": 68,
					"latency_max": 607
				}
			],
			"summary":{
				"latency_ave": 185.60,
				"latency_std": 211.38,
				"latency_min": 68,
				"latency_max": 607,
				"latency_50": 71,
				"latency_90": 607,
				"latency_99": 607,
				"latency_99.99": 607,
				"latency_99.9999": 607
			}
		}
	]
}
```

### Subscriber side

```
{"perftest":
	[
		{
			"length":32,
			"intervals":[

				{
					"length": 32,
					"packets": 5538304,
					"packets/s": 2816593,
					"packets/s_ave": 2816593.00,
					"mbps": 721.0,
					"mbps_ave": 721.0,
					"lost": 0,
					"lost_percent": 0.00
				},
				{
					"length": 32,
					"packets": 8233728,
					"packets/s": 2695178,
					"packets/s_ave": 2755885.50,
					"mbps": 690.0,
					"mbps_ave": 705.5,
					"lost": 0,
					"lost_percent": 0.00
				}			],
			"summary":{
				"packets": 8960007,
				"packets/s_ave": 2750097,
				"mbps_ave": 704.0,
				"lost": 0,
				"lost_percent": 0.00
			}
		},
		{
			"length":64,
			"intervals":[

				{
					"length": 64,
					"packets": 4537984,
					"packets/s": 2495077,
					"packets/s_ave": 2495077.00,
					"mbps": 1277.5,
					"mbps_ave": 1277.5,
					"lost": 0,
					"lost_percent": 0.00
				}			],
			"summary":{
				"packets": 6400005,
				"packets/s_ave": 2326674,
				"mbps_ave": 1191.3,
				"lost": 0,
				"lost_percent": 0.00
			}
		}
	]
}
```

## Scan Test -- NOT printing intervals + -scan

### Publisher side

```
{"perftest":
	[
		{
			"length":32,
			"summary":{
				"latency_ave": 914.00,
				"latency_std": 1828.21,
				"latency_min": 93,
				"latency_max": 5002,
				"latency_50": 98,
				"latency_90": 5002,
				"latency_99": 5002,
				"latency_99.99": 5002,
				"latency_99.9999": 5002
			}
		},
		{
			"length":64,
			"summary":{
				"latency_ave": 110.67,
				"latency_std": 25.47,
				"latency_min": 79,
				"latency_max": 145,
				"latency_50": 117,
				"latency_90": 145,
				"latency_99": 145,
				"latency_99.99": 145,
				"latency_99.9999": 145
			}
		}
	]
}

```

### Subscriber side

```
{"perftest":
	[
		{
			"length":32,
			"summary":{
				"packets": 7680006,
				"packets/s_ave": 2647584,
				"mbps_ave": 677.8,
				"lost": 0,
				"lost_percent": 0.00
			}
		},
		{
			"length":64,
			"summary":{
				"packets": 6400005,
				"packets/s_ave": 1999456,
				"mbps_ave": 1023.7,
				"lost": 0,
				"lost_percent": 0.00
			}
		}
	]
}
```