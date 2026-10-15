python3 generate_unsorted.py unsorted.csv 100000000
python3 generate_sorted.py unsorted.csv sorted.csv
python3 generate_outliers.py sorted.csv outliers.csv 100000000 10000
python3 generate_combined.py unsorted.csv 0.333 sorted.csv 0.333 outliers.csv 0.334 combined.csv 8192
