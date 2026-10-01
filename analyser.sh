bin_dir="./duckdb/build/release"
out_dir=$1
file=$2
for i in $(seq 0 8);
do
    echo $i
    mkdir -p $out_dir/
    cat template.csv > $out_dir/$i.log
    $bin_dir/duckdb -statistic $i -f $file 2>> $out_dir/$i.log
    rm test.db
done

python3 analyser_processor.py 1 7 1 output/final.csv "NONE" output/0.log "MIN_MAX" output/1.log "CLUSTER_SMALL" output/2.log "CLUSTER_MEDIUM" output/3.log "CLUSTER_LARGE" output/4.log "BLOOM_SMALL" output/5.log "BLOOM_MEDIUM" output/6.log "BLOOM_LARGE" output/7.log "DICTIONARY" output/8.log
rm test.db
python3 analyser_scatter_plotter.py output/final.csv final.png