bin_dir="./duckdb/build/release"
out_dir=$1
file=$2
for i in $(seq 0 8);
do
    echo $i
    mkdir -p $out_dir/
    cat template.csv > $out_dir/$i.csv
    $bin_dir/duckdb -statistic $i -f $file 2>> $out_dir/$i.csv
    rm test.db
done

python3 analyser_processor.py 1 7 1 output/final.csv "NONE" output/0.csv "MIN_MAX" output/1.csv "CLUSTER_SMALL" output/2.csv "CLUSTER_MEDIUM" output/3.csv "CLUSTER_LARGE" output/4.csv "BLOOM_SMALL" output/5.csv "BLOOM_MEDIUM" output/6.csv "BLOOM_LARGE" output/7.csv "DICTIONARY" output/8.csv
rm test.db