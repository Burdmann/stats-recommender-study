import argparse
import csv
from itertools import islice
from pathlib import Path
from typing import Iterator


def iter_csv_partitions(
    csv_path: Path, partition_size: int
) -> Iterator[list[list[str]]]:
    with csv_path.open(newline="", encoding="utf-8-sig") as csv_file:
        reader = csv.reader(csv_file)
        while True:
            partition = list(islice(reader, partition_size))
            if not partition:
                break
            yield partition


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Iterate through a CSV file in fixed-size partitions."
    )
    parser.add_argument("csv_file", type=Path, help="Path to the input CSV file")
    parser.add_argument("query_file", type=Path, help="Path to the query CSV file")
    parser.add_argument(
        "partition_size",
        type=int,
        help="Number of CSV rows to include in each partition",
    )
    parser.add_argument(
        "N",
        type=int,
        help="length of n-grams considered",
    )
    args = parser.parse_args()

    if args.partition_size <= 0:
        parser.error("partition_size must be greater than zero")

    queries = []
    with args.query_file.open(newline="", encoding="utf-8-sig") as csv_file:
        reader = csv.reader(csv_file)
        for query in reader:
            queries.append(query)
    print(queries)

    for partition_number, partition in enumerate(
        iter_csv_partitions(args.csv_file, args.partition_size), start=1
    ):
        ngrams = set()
        for s in partition:
            s=s[0]
            for i in range(len(s)-args.N):
                ngrams.add(s[i:i+args.N])

        queries_pruned = 0
        for q in queries:
            q=q[0]
            for i in range(len(q)-args.N):
                qp = q[i:i+args.N]
                if not qp in ngrams:
                    queries_pruned += 1
                    break

        print(partition_number,len(ngrams),queries_pruned)


if __name__ == "__main__":
    main()