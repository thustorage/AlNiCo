set term pdfcairo size 6.8999999999999995in,2.3in font ',10'
set output OUT

set title 'Throughput'
set xlabel 'times(10ms)'
set ylabel 'M txns/ms'
plot [0:][0:] IN using 1:2 title 'A' with lp ps 0.5

set output