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
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE created_at BETWEEN ? AND ?;" $count "profiler/q1" "created_at" 0 -1 "created_at" 0 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE created_at BETWEEN ? AND ? AND type = ?;" $count "profiler/q2" "created_at" 0 -1 "created_at" 0 -1 "type" 1 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE created_at BETWEEN ? AND ? AND org_id = ?;" $count "profiler/q3" "created_at" 0 -1 "created_at" 0 -1 "org_id" 1 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE type = ?;" $count "profiler/q4" "type" 0 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE payload_action = ?;" $count "profiler/q5" "payload_action" 0 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE payload_ref_type = ?;" $count "profiler/q6" "payload_ref_type" 0 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE org_id = ?;" $count "profiler/q7" "org_id" 0 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE payload_number = ?;" $count "profiler/q8" "payload_number" 0 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE payload_distinct_size = ?;" $count "profiler/q9" "payload_distinct_size" 0 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE actor_id = ?;" $count "profiler/q10" "actor_id" 0 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE repo_id = ?;" $count "profiler/q11" "repo_id" 0 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE payload_repository_id = ?;" $count "profiler/q12" "payload_repository_id" 0 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE payload_ref = ?;" $count "profiler/q13" "payload_ref" 0 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE payload_push_id = ?;" $count "profiler/q14" "payload_push_id" 0 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE id = ?;" $count "profiler/q15" "id" 0 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE actor_id = ? AND repo_id = ?;" $count "profiler/q16" "actor_id" 0 -1 "repo_id" 1 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE repo_id = ? AND org_id = ?;" $count "profiler/q17" "repo_id" 0 -1 "org_id" 1 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE actor_id = ? AND org_id = ?;" $count "profiler/q18" "actor_id" 0 -1 "org_id" 1 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE actor_id = ? AND repo_id = ? AND type = ?;" $count "profiler/q19" "actor_id" 0 -1 "repo_id" 1 -1 "type" 2 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE actor_id = ? AND created_at BETWEEN ? AND ?;" $count "profiler/q20" "actor_id" 0 -1 "created_at" 1 -1 "created_at" 1 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE repo_id = ? AND created_at BETWEEN ? AND ?;" $count "profiler/q21" "repo_id" 0 -1 "created_at" 1 -1 "created_at" 1 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE org_id = ? AND created_at BETWEEN ? AND ?;" $count "profiler/q22" "org_id" 0 -1 "created_at" 1 -1 "created_at" 1 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE actor_id = ? AND repo_id = ? AND created_at BETWEEN ? AND ?;" $count "profiler/q23" "actor_id" 0 -1 "repo_id" 1 -1 "created_at" 2 -1 "created_at" 2 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE payload_size BETWEEN ? AND ?;" $count "profiler/q24" "payload_size" 0 0.01 "payload_size" 0 0.01
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE payload_distinct_size BETWEEN ? AND ?;" $count "profiler/q25" "payload_distinct_size" 0 0.01 "payload_distinct_size" 0 0.01
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE payload_size BETWEEN ? AND ? AND repo_id = ?;" $count "profiler/q26" "payload_size" 0 0.01 "payload_size" 0 0.01 "repo_id" 1 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE payload_size BETWEEN ? AND ? AND org_id = ?;" $count "profiler/q27" "payload_size" 0 0.01 "payload_size" 0 0.01 "org_id" 1 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE payload_distinct_size BETWEEN ? AND ? AND actor_id = ?;" $count "profiler/q28" "payload_distinct_size" 0 0.01 "payload_distinct_size" 0 0.01 "actor_id" 1 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE payload_ref = ?;" $count "profiler/q29" "payload_ref" 0 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE payload_ref LIKE ? || '%';" $count "profiler/q30" "payload_ref" 0 -1
python3 query_gen/query_gen.py "gha/github.csv" "SELECT COUNT(*) FROM github WHERE payload LIKE '%' || ? || '%';" $count "profiler/q31" "payload" 0 -1