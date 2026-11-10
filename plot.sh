plot_pair() {
  local folder="$1"
  shift
  python3 plot_time_breakdown.py "$folder" "$@"
  python3 plot_pruning.py "$folder" "$@"
}

plot_selectivity_pair() {
  local folder="$1"
  shift
  python3 plot_time_selectivity.py "$folder" "$@"
  python3 plot_pruning_selectivity.py "$folder" "$@"
}

plot_pair experiment_results/flights/processed \
  --group "temporal\nrange query=q1" \
  --group "range query=q9" \
  --group "point query\n(low NDV)=q7,q8" \
  --group "point query\n(medium NDV)=q2,q3" \
  --group "point query\n(high NDV)=q5,q6" \
  --group "point queries\nmulti-column=q4,q16,q17,q19,q21" \
  --group "open-ended\nranges=q10,q11" \
  --group "temporal range\n+ point query=q12,q13,q14,q15,q18" \
  --group "temporal range\n+ muliple point queries=q20" \
  --group "unclusted range\n+ point query=q22,q23,q24" \
  --group "conjunction range\n+ queries=q25" \
  --group "conjunction range\n+ open-ended range=q26" \
  --configuration "MIN_MAX=Min/max" \
  --configuration "BLOOM_1=Bloom filter" \
  --configuration "GREEDY_1=Greedy" \
  --configuration "CLUSTER_10=Min/max clusters" \
  --configuration "BLOOM_10=Bloom filter" \
  --configuration "GREEDY_10=Greedy" \
  --configuration "CLUSTER_50=Min/max clusters" \
  --configuration "BLOOM_50=Bloom filter" \
  --configuration "GREEDY_50=Greedy" \
  --configuration "CLUSTER_200=Min/max clusters" \
  --configuration "BLOOM_200=Bloom filter" \
  --configuration "GREEDY_200=Greedy" \
  --configuration "DICTIONARY=Dictionary"

plot_pair experiment_results/github/processed \
  --group "temporal\nrange query=q1,q2,q3" \
  --group "point query\n(low NDV)=q4,q5,q6" \
  --group "point query\n(medium NDV)=q7,q8,q9" \
  --group "point query\n(high NDV)=q10,q11,q12,q13" \
  --group "point query\n(near-unique)=q14,q15" \
  --group "point queries\nmulti-column=q16,q17,q18,q19" \
  --group "temporal range\n+ point query=q20,q21,q22,q23" \
  --group "numeric\nrange query=q26,q27,q28" \
  --group "string queries=q29,q30" \
  --configuration "MIN_MAX=Min/max" \
  --configuration "BLOOM_1=Bloom filter" \
  --configuration "GREEDY_1=Greedy" \
  --configuration "CLUSTER_10=Min/max clusters" \
  --configuration "BLOOM_10=Bloom filter" \
  --configuration "GREEDY_10=Greedy" \
  --configuration "CLUSTER_50=Min/max clusters" \
  --configuration "BLOOM_50=Bloom filter" \
  --configuration "GREEDY_50=Greedy" \
  --configuration "CLUSTER_200=Min/max clusters" \
  --configuration "BLOOM_200=Bloom filter" \
  --configuration "GREEDY_200=Greedy" \
  --configuration "DICTIONARY=Dictionary"

plot_selectivity_pair experiment_results/flights_line_plot \
  --group "range=q9,q10,q11" \
  --group "range + point=q22,q23,q24" \
  --group "range conjunction=q25,q26" \
  --configuration "MIN_MAX=Min/max" \
  --configuration "BLOOM_1=Bloom filter" \
  --configuration "GREEDY_1=Greedy"

plot_selectivity_pair experiment_results/github_line_plot \
  --group "range (low NDV)=q24" \
  --group "range (medium NDV)=q25" \
  --group "range + point=q26,q27,q28" \
  --configuration "MIN_MAX=Min/max" \
  --configuration "BLOOM_1=Bloom filter" \
  --configuration "GREEDY_1=Greedy"