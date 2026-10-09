sql_file=$1
folder=$2
bin_dir="./duckdb/build/release"

mkdir -p $folder/1x
mkdir -p $folder/10x
mkdir -p $folder/50x
mkdir -p $folder/200x
mkdir -p $folder/log
mkdir -p $folder/processed

rm test.db
./analyser.sh $folder/analysis $sql_file

python3 assigners/greedy_simple.py $folder/analysis/final.csv $folder/1x/assignment_simple.csv 4182
python3 assigners/greedy.py $folder/analysis/final.csv $folder/1x/assignment.csv 4182

python3 assigners/greedy_simple.py $folder/analysis/final.csv $folder/10x/assignment_simple.csv 41820
python3 assigners/greedy.py $folder/analysis/final.csv $folder/10x/assignment.csv 41820

python3 assigners/greedy_simple.py $folder/analysis/final.csv $folder/50x/assignment_simple.csv 209100
python3 assigners/greedy.py $folder/analysis/final.csv $folder/50x/assignment.csv 209100

python3 assigners/greedy_simple.py $folder/analysis/final.csv $folder/200x/assignment_simple.csv 836400
python3 assigners/greedy.py $folder/analysis/final.csv $folder/200x/assignment.csv 836400

cat template.csv > $folder/log/min_max.log
cat template.csv > $folder/log/greedy_simple_1.log
cat template.csv > $folder/log/greedy_1.log
$bin_dir/duckdb -statistic 1 -f $sql_file 2>> $folder/log/min_max.log
rm test.db
$bin_dir/duckdb -statistic-file $folder/1x/assignment_simple.csv -f $sql_file 2>> $folder/log/greedy_simple_1.log
rm test.db
$bin_dir/duckdb -statistic-file $folder/1x/assignment.csv -f $sql_file 2>> $folder/log/greedy_1.log

cat template.csv > $folder/log/cluster_10.log
cat template.csv > $folder/log/bloom_10.log
cat template.csv > $folder/log/greedy_simple_10.log
cat template.csv > $folder/log/greedy_10.log
rm test.db
$bin_dir/duckdb -statistic 2 -f $sql_file 2>> $folder/log/cluster_10.log
rm test.db
$bin_dir/duckdb -statistic 5 -f $sql_file 2>> $folder/log/bloom_10.log
rm test.db
$bin_dir/duckdb -statistic-file $folder/10x/assignment_simple.csv -f $sql_file 2>> $folder/log/greedy_simple_10.log
rm test.db
$bin_dir/duckdb -statistic-file $folder/10x/assignment.csv -f $sql_file 2>> $folder/log/greedy_10.log

cat template.csv > $folder/log/cluster_50.log
cat template.csv > $folder/log/bloom_50.log
cat template.csv > $folder/log/greedy_simple_50.log
cat template.csv > $folder/log/greedy_50.log
rm test.db
$bin_dir/duckdb -statistic 3 -f $sql_file 2>> $folder/log/cluster_50.log
rm test.db
$bin_dir/duckdb -statistic 6 -f $sql_file 2>> $folder/log/bloom_50.log
rm test.db
$bin_dir/duckdb -statistic-file $folder/50x/assignment_simple.csv -f $sql_file 2>> $folder/log/greedy_simple_50.log
rm test.db
$bin_dir/duckdb -statistic-file $folder/50x/assignment.csv -f $sql_file 2>> $folder/log/greedy_50.log

cat template.csv > $folder/log/cluster_200.log
cat template.csv > $folder/log/bloom_200.log
cat template.csv > $folder/log/greedy_simple_200.log
cat template.csv > $folder/log/greedy_200.log
rm test.db
$bin_dir/duckdb -statistic 4 -f $sql_file 2>> $folder/log/cluster_200.log
rm test.db
$bin_dir/duckdb -statistic 7 -f $sql_file 2>> $folder/log/bloom_200.log
rm test.db
$bin_dir/duckdb -statistic-file $folder/200x/assignment_simple.csv -f $sql_file 2>> $folder/log/greedy_simple_200.log
rm test.db
$bin_dir/duckdb -statistic-file $folder/200x/assignment.csv -f $sql_file 2>> $folder/log/greedy_200.log
rm test.db

python3 data_processor.py 1 100 1 MIN_MAX $folder/log/min_max.log GREEDY_SIMPLE_1 $folder/log/greedy_simple_1.log GREEDY_1 $folder/log/greedy_1.log \
        CLUSTER_10 $folder/log/cluster_10.log BLOOM_10 $folder/log/bloom_10.log GREEDY_SIMPLE_10 $folder/log/greedy_simple_10.log GREEDY_10 $folder/log/greedy_10.log \
        CLUSTER_50 $folder/log/cluster_50.log BLOOM_50 $folder/log/bloom_50.log GREEDY_SIMPLE_50 $folder/log/greedy_simple_50.log GREEDY_50 $folder/log/greedy_50.log \
        CLUSTER_200 $folder/log/cluster_200.log BLOOM_200 $folder/log/bloom_200.log GREEDY_SIMPLE_200 $folder/log/greedy_simple_200.log GREEDY_200 $folder/log/greedy_200.log

mv pruning.csv $folder/processed/
# mv query_time.csv $folder/processed/
mv size.csv $folder/processed/
# mv ingestion.csv $folder/processed/