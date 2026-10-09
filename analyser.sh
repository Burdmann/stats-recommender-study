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

python3 analyser_processor.py 1 7 1 $out_dir/final.csv "NONE" $out_dir/0.log "MIN_MAX" $out_dir/1.log "CLUSTER_SMALL" $out_dir/2.log "CLUSTER_MEDIUM" $out_dir/3.log "CLUSTER_LARGE" $out_dir/4.log "BLOOM_SMALL" $out_dir/5.log "BLOOM_MEDIUM" $out_dir/6.log "BLOOM_LARGE" $out_dir/7.log "DICTIONARY" $out_dir/8.log
rm test.db
python3 analyser_scatter_plotter.py $out_dir/final.csv final.png