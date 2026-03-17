if test -f "$1/output_min_max.txt"; then
    echo "min/max"
    diff $1/output_correct.txt $1/output_min_max.txt
fi
if test -f "$1/output_cluster_10.txt"; then
    echo "cluster_10"
    diff $1/output_correct.txt $1/output_cluster_10.txt
fi
if test -f "$1/output_bloom_10.txt"; then
    echo "bloom_10"
    diff $1/output_correct.txt $1/output_bloom_10.txt
fi
if test -f "$1/output_dictionary_10.txt"; then
    echo "dictionary_10"
    diff $1/output_correct.txt $1/output_dictionary_10.txt
fi
if test -f "$1/output_cluster.txt"; then
    echo "cluster"
    diff $1/output_correct.txt $1/output_cluster.txt
fi
if test -f "$1/output_bloom.txt"; then
    echo "bloom"
    diff $1/output_correct.txt $1/output_bloom.txt
fi
if test -f "$1/output_dictionary.txt"; then
    echo "dictionary"
    diff $1/output_correct.txt $1/output_dictionary.txt
fi
if test -f "$1/output_cluster_1000.txt"; then
    echo "cluster_1000"
    diff $1/output_correct.txt $1/output_cluster_1000.txt
fi
if test -f "$1/output_bloom_1000.txt"; then
    echo "bloom_1000"
    diff $1/output_correct.txt $1/output_bloom_1000.txt
fi
if test -f "$1/output_dictionary_1000.txt"; then
    echo "dictionary_1000"
    diff $1/output_correct.txt $1/output_dictionary_1000.txt
fi