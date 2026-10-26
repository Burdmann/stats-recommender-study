cd ssb-dbgen
scale=1
./dbgen -f -F -O h -s $scale
for f in *.tbl; do
    sed -i 's/,$//' "$f"
done
cd ..
duckdb -f load.sql