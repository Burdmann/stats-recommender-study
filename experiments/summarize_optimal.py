"""Summarizes experiments_results/optimal: partitions scanned per query set and statistic.

Reads <dir>/<query set>/<binary>/raw_data.csv (stderr log of the modified DuckDB shell), checks the query
results against min/max (no false negatives), and writes <dir>/pruning.csv.
"""
import csv
import filecmp
import json
import os
import sys

out = sys.argv[1]


def read_run(raw):
    commands, scanned, sizes = {}, {}, []
    with open(raw) as f:
        for rec in csv.DictReader(f):
            data = json.loads(rec["Data"])
            if rec["Type"] == "SQL_COMMAND_RUN_START":
                commands[rec["CommandID"]] = data["command"]
            elif rec["Type"] == "SCANNED_PARTITIONS":
                scanned[rec["CommandID"]] = data["count"]
            elif rec["Type"] == "END_INITIALISE_ADDITIONAL_STATS":
                sizes.append(data["size"])
    queries = [cid for cid, cmd in commands.items() if cmd.lstrip().upper().startswith("SELECT") and cid in scanned]
    return queries, scanned, sizes


rows = []
for query_set in sorted(os.listdir(out)):
    set_dir = os.path.join(out, query_set)
    if not os.path.isdir(set_dir):
        continue
    # partitions (row groups) and the min/max baseline come from the min/max run; the last, partial row
    # group has no additional statistic, so counting statistics would miss it
    baseline_raw = os.path.join(set_dir, "min_max", "raw_data.csv")
    baseline = read_run(baseline_raw) if os.path.exists(baseline_raw) else None
    partitions = max(baseline[1][c] for c in baseline[0]) if baseline and baseline[0] else None
    baseline_total = sum(baseline[1][c] for c in baseline[0]) if baseline else None
    for binary in sorted(os.listdir(set_dir)):
        run_dir = os.path.join(set_dir, binary)
        raw = os.path.join(run_dir, "raw_data.csv")
        if not os.path.exists(raw):
            continue
        queries, scanned, sizes = read_run(raw)
        if len(queries) < 1000:
            continue  # still running
        total = sum(scanned[c] for c in queries)
        possible = partitions * len(queries) if partitions else 0
        same = filecmp.cmp(os.path.join(run_dir, "output.txt"), os.path.join(set_dir, "min_max", "output.txt"),
                           shallow=False) if os.path.exists(os.path.join(set_dir, "min_max", "output.txt")) else None
        rows.append({
            "query_set": query_set,
            "statistic": binary,
            "queries": len(queries),
            "partitions": partitions,
            "stat_bytes_per_partition": max(sizes) if sizes else 0,
            "partitions_scanned": total,
            "pruned_fraction": round(1 - total / possible, 6) if possible else None,
            "pruned_beyond_min_max": round(1 - total / baseline_total, 6) if baseline_total else None,
            "results_match_min_max": same,
        })

with open(os.path.join(out, "pruning.csv"), "w", newline="") as f:
    writer = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
    writer.writeheader()
    writer.writerows(rows)
for r in rows:
    print(f"{r['query_set']:24} {r['statistic']:15} scanned {r['partitions_scanned']:>10} of "
          f"{r['partitions'] * r['queries']:>10}  pruned {100 * r['pruned_fraction']:8.4f}%  "
          f"beyond min/max {100 * r['pruned_beyond_min_max']:8.4f}%  "
          f"bytes {r['stat_bytes_per_partition']:>6}  correct {r['results_match_min_max']}")
