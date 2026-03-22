python3 generate_unsorted.py unsorted.csv 1000000000
python3 generate_sorted.py unsorted.csv sorted.csv
python3 generate_unsorted sorted.csv outliers.csv 1000000000 10000
python3 generate_cardinality sorted.csv cardinality_100.csv 100
python3 generate_cardinality sorted.csv cardinality_1000.csv 1000
python3 generate_cardinality sorted.csv cardinality_10000.csv 10000
python3 generate_cardinality sorted.csv cardinality_100000.csv 100000
