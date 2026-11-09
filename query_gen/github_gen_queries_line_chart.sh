# COL NDV
# id 3909986
# "type" 15
# created_at 86382
# actor_id 419709
# actor_login 1161
# repo_id 683676
# repo_name 0
# org_id 42147
# org_login 0
# payload 3482938
# payload_repository_id 468942
# payload_push_id 2704149
# payload_size 45313
# payload_distinct_size 1155
# payload_ref 183365
# payload_action 8
# payload_ref_type 4
# payload_number 23614

count=100
for p in 0.1 0.01 0.001 0.0001; do

python3 query_gen/query_gen.py "gha/github.csv" \
"SELECT COUNT(*) FROM github WHERE payload_size BETWEEN ? AND ?;" \
$count "profiler/q24_${p}" \
"payload_size" 0 ${p} \
"payload_size" 0 ${p}

python3 query_gen/query_gen.py "gha/github.csv" \
"SELECT COUNT(*) FROM github WHERE payload_distinct_size BETWEEN ? AND ?;" \
$count "profiler/q25_${p}" \
"payload_distinct_size" 0 ${p} \
"payload_distinct_size" 0 ${p}

python3 query_gen/query_gen.py "gha/github.csv" \
"SELECT COUNT(*) FROM github WHERE payload_size BETWEEN ? AND ? AND repo_id = ?;" \
$count "profiler/q26_${p}" \
"payload_size" 0 ${p} \
"payload_size" 0 ${p} \
"repo_id" 1 -1

python3 query_gen/query_gen.py "gha/github.csv" \
"SELECT COUNT(*) FROM github WHERE payload_size BETWEEN ? AND ? AND org_id = ?;" \
$count "profiler/q27_${p}" \
"payload_size" 0 ${p} \
"payload_size" 0 ${p} \
"org_id" 1 -1

python3 query_gen/query_gen.py "gha/github.csv" \
"SELECT COUNT(*) FROM github WHERE payload_distinct_size BETWEEN ? AND ? AND actor_id = ?;" \
$count "profiler/q28_${p}" \
"payload_distinct_size" 0 ${p} \
"payload_distinct_size" 0 ${p} \
"actor_id" 1 -1

done