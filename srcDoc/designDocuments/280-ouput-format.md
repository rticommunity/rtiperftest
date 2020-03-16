# RTI Perftest Output Formats

## Introduction and goals

The aim of this project is to store all the data printed during the execution of RTI Perftest in a file or output in the stdout. This can be very useful because it's easier to work with each parameter divided into fields, and we could use it to generate graphs, compare with different platforms, etc.


## Tasks

- [ ] Discuss what parts of the output and how can we save in the document.
- [ ] What type of file formats can be supported (json, csv, etc).
- [ ] Which libraries need to be added to the project.
- [ ] Investigate file system compatibility between different platforms.


## Desired output examples

The objective to be achieved would be use some flags to control the output of the program. Some ideas are shown below:

| Flag                                   | Description               |  Type   |   Default value    |
| :------------------------------------- | :------------------------ | :-----: | :----------------: |
| `-printFormat {csv, regular, json...}` | Select format type        | String  |       '.csv'       |
| `-noPrintHeaders`                      | No add header row to file | boolean |       false        |
| `-printFileName $filename`             | Name of the document      | String  | 'perftest+output'. |
| `-printSummary`                        | Add summary to file       | boolean |       false        |

```

====> All samples have -printFormat csv
===> Scan (NOT USING -noPrint)

Data Length, Packets, Packets/s, Packets/s(ave), Mbps, Mbps(ave), Lost, Lost(%)
32, 5242112,2677596,2677596,685.5,685.5,0,0.00
32, 7974912,2732570,2705083,699.5,692.5,0,0.00
32, 10717952,2742782,2717649,702.2,695.7,0,0.00
Length, Packets, Packets/s (ave), Mbps (ave), Lost, Lost(%)
32,12800020,2683530,687.0,0,0.00


Data Length, Packets, Packets/s, Packets/s(ave), Mbps, Mbps(ave), Lost, Lost(%)
64, 4534400,2497951,2497951,1279.0,1279.0,0,0.00
64, 6970752,2435967,2466959,1247.2,1263.1,0,0.00
64, 9434368,2463502,2465807,1261.3,1262.5,0,0.00
Length, Packets, Packets/s (ave), Mbps (ave), Lost, Lost(%)
64,11520018,2360357,1208.5,0,0.00


Data Length, Packets, Packets/s, Packets/s(ave), Mbps, Mbps(ave), Lost, Lost(%)
128, 3709601,1927273,1927273,1973.5,1973.5,0,0.00
128, 5609920,1900146,1913710,1945.7,1959.6,0,0.00
128, 7504000,1893909,1907109,1939.4,1952.9,0,0.00
128, 9465984,1961815,1920786,2008.9,1966.9,0,0.00
Length, Packets, Packets/s (ave), Mbps (ave), Lost, Lost(%)
128,9600015,1886726,1932.0,0,0.00



===> Scan (USING -noPrint)

Length, Packets, Packets/s (ave), Mbps (ave), Lost, Lost(%)
32,10240016,2406518,616.1,0,0.00
64,9600015,2191135,1121.9,0,0.00
128,7680012,1745687,1787.6,0,0.00


===> Scan (USING -noPrint -noPrintHeaders)

32,10240016,2406518,616.1,0,0.00
64,9600015,2191135,1121.9,0,0.00
128,7680012,1745687,1787.6,0,0.00


===> Regular (NOT USING -noPrint)

Data Length, Packets, Packets/s, Packets/s(ave), Mbps, Mbps(ave), Lost, Lost(%)
32, 5242112,2677596,2677596,685.5,685.5,0,0.00
32, 7974912,2732570,2705083,699.5,692.5,0,0.00
32, 10717952,2742782,2717649,702.2,695.7,0,0.00
Length, Packets, Packets/s (ave), Mbps (ave), Lost, Lost(%)
32,12800020,2683530,687.0,0,0.00


===> Regular (USING -noPrint)

Length, Packets, Packets/s (ave), Mbps (ave), Lost, Lost(%)
32,12800020,2683530,687.0,0,0.00


===> Regular (USING -noPrint -noPrintHeaders)

32,12800020,2683530,687.0,0,0.00
```


## Notes

It would be fine to choose what kind of data we want to print(and not all the columns), join `-printFormat` and `-printFileName`, generate graphs, etc.
