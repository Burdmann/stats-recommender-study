import argparse
import csv
import math
import re
from pathlib import Path

import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt

from plot_colors import (
    APPROACH_COLORS,
    add_color_argument,
    color_for,
    parse_color_overrides,
)
from plot_time_breakdown import (
    UNIT_TO_MILLISECONDS,
    parse_configurations,
    parse_query_groups,
)


QUERY_SELECTIVITY_PATTERN = re.compile(r"^(q\d+)_([0-9]+(?:\.[0-9]+)?)$")


def query_and_selectivity(query, parser):
    match = QUERY_SELECTIVITY_PATTERN.fullmatch(query)
    if match is None:
        parser.error(
            f"query {query!r} must have a base query and dot-free selectivity suffix"
        )

    suffix = match.group(2)
    if "." in suffix:
        selectivity = float(suffix)
    elif suffix.startswith("0") and len(suffix) > 1:
        selectivity = float(f"0.{suffix[1:]}")
    else:
        selectivity = float(suffix)
    if not math.isfinite(selectivity) or not 0 < selectivity <= 1:
        parser.error(f"query {query!r} has invalid selectivity {selectivity}")
    return match.group(1), selectivity


def read_metric_csv(path, parser):
    try:
        with path.open(newline="", encoding="utf-8") as csv_file:
            reader = csv.DictReader(csv_file)
            if reader.fieldnames is None:
                parser.error(f"{path} is empty")
            required_columns = {"Measurement", "Unit", "Query"}
            if not required_columns.issubset(reader.fieldnames):
                parser.error(
                    f"{path} must contain Measurement, Unit, and Query columns"
                )

            configurations = [
                column for column in reader.fieldnames if column not in required_columns
            ]
            if not configurations:
                parser.error(f"{path} does not contain configuration columns")

            values = {}
            units = set()
            for row_number, row in enumerate(reader, start=2):
                query = row["Query"]
                if not query:
                    parser.error(f"{path}, row {row_number}: Query is empty")
                if query in values:
                    parser.error(f"{path}, row {row_number}: duplicate query {query!r}")
                units.add(row["Unit"] or "")
                values[query] = {}
                for configuration in configurations:
                    try:
                        value = float(row[configuration])
                    except (TypeError, ValueError):
                        parser.error(
                            f"{path}, row {row_number}: invalid value for {configuration}"
                        )
                    if not math.isfinite(value) or value < 0:
                        parser.error(
                            f"{path}, row {row_number}: {configuration} must be a "
                            "finite, non-negative number"
                        )
                    values[query][configuration] = value
    except OSError as error:
        parser.error(str(error))

    if not values:
        parser.error(f"{path} does not contain any query rows")
    if len(units) != 1:
        parser.error(f"{path} must use one consistent Unit")
    return values, configurations, units.pop()


def index_queries(queries, parser):
    query_ids = {}
    for query in queries:
        base_query, selectivity = query_and_selectivity(query, parser)
        selectivities = query_ids.setdefault(base_query, {})
        if selectivity in selectivities:
            parser.error(
                f"multiple query rows for {base_query} at selectivity {selectivity}"
            )
        selectivities[selectivity] = query
    return query_ids


def aggregate_group(groups, query_ids, values, configurations, parser):
    aggregated = {}
    for name, members in groups:
        shared_selectivities = set.intersection(
            *(set(query_ids[member]) for member in members)
        )
        if not shared_selectivities:
            parser.error(f"query group {name!r} has no shared selectivity values")

        aggregated[name] = {}
        for selectivity in sorted(shared_selectivities):
            query_rows = [query_ids[member][selectivity] for member in members]
            aggregated[name][selectivity] = {
                configuration: sum(
                    values[query][configuration] for query in query_rows
                ) / len(query_rows)
                for configuration in configurations
            }
    return aggregated


def make_parser(kind):
    parser = argparse.ArgumentParser(
        description=(
            f"Plot {kind} against query selectivity, with one figure per query group."
        )
    )
    parser.add_argument(
        "folder",
        type=Path,
        help="result folder or processed folder containing the metric CSV files",
    )
    parser.add_argument(
        "--group",
        action="append",
        metavar="NAME=QUERY,QUERY",
        help=(
            "plot the listed base queries as one group (repeatable); use \\n in "
            "NAME for a line break"
        ),
    )
    parser.add_argument(
        "--configuration",
        action="append",
        metavar="CONFIG=LABEL",
        help="plot only this approach and use LABEL in the legend (repeatable)",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        help="directory for generated PNGs (defaults to the result folder)",
    )
    add_color_argument(parser)
    return parser


def plot_group(
    kind,
    output_path,
    group_name,
    series,
    configurations,
    labels,
    color_overrides,
    lower_bound=None,
):
    figure, axis = plt.subplots(figsize=(11, 6.5))
    selectivities = sorted(series)

    for index, configuration in enumerate(configurations):
        axis.plot(
            selectivities,
            [series[value][configuration] for value in selectivities],
            color=color_for(configuration, color_overrides, APPROACH_COLORS),
            marker="o",
            linewidth=1.8,
            markersize=5,
            label=labels[configuration],
        )

    if lower_bound is not None:
        axis.plot(
            selectivities,
            [lower_bound[value] for value in selectivities],
            color="#222222",
            linestyle="--",
            marker="D",
            linewidth=1.6,
            markersize=5,
            label="Minimum possible",
        )

    axis.set_xscale("log")
    axis.set_xticks(selectivities)
    axis.set_xticklabels([f"{value:g}" for value in selectivities])
    axis.set_xlabel("Selectivity")
    if kind == "time":
        axis.set_ylabel("Query time (ms)")
        axis.set_title(f"Query time vs. selectivity ({group_name})")
    else:
        axis.set_ylabel("Rows scanned (lower is better)")
        axis.set_title(f"Pruning vs. selectivity ({group_name})")
        axis.ticklabel_format(axis="y", style="plain", useOffset=False)
    axis.tick_params(axis="both", labelsize=12)
    axis.xaxis.label.set_size(14)
    axis.yaxis.label.set_size(14)
    axis.title.set_size(16)
    axis.grid(axis="both", alpha=0.25)
    axis.set_axisbelow(True)
    axis.legend(
        loc="center left",
        bbox_to_anchor=(1.02, 0.5),
        frameon=False,
        fontsize=9,
    )
    figure.tight_layout(rect=(0, 0, 0.76, 1))
    figure.savefig(output_path, dpi=180, bbox_inches="tight")
    plt.close(figure)


def run_plotter(kind):
    parser = make_parser("query time" if kind == "time" else "pruning")
    args = parser.parse_args()
    data_folder = args.folder / "processed" if (args.folder / "processed").is_dir() else args.folder
    metric_name = "total_time.csv" if kind == "time" else "pruning.csv"
    values, configurations, unit = read_metric_csv(data_folder / metric_name, parser)
    query_ids = index_queries(values, parser)

    selected_configurations, labels = parse_configurations(
        args.configuration, configurations, parser
    )
    color_overrides = parse_color_overrides(args.color, configurations, parser)
    groups = parse_query_groups(args.group, sorted(query_ids), parser)
    grouped_values = aggregate_group(groups, query_ids, values, configurations, parser)

    if kind == "time":
        if unit not in UNIT_TO_MILLISECONDS:
            parser.error(f"unsupported time unit {unit!r}")
        multiplier = UNIT_TO_MILLISECONDS[unit]
        for group_values in grouped_values.values():
            for point in group_values.values():
                for configuration in configurations:
                    point[configuration] *= multiplier

    lower_bounds = None
    if kind == "pruning":
        contributing_rows, lower_configurations, _ = read_metric_csv(
            data_folder / "contributing_rows.csv", parser
        )
        if lower_configurations != configurations:
            parser.error("pruning.csv and contributing_rows.csv configurations differ")
        if contributing_rows.keys() != values.keys():
            parser.error("pruning.csv and contributing_rows.csv query rows differ")
        lower_query_ids = index_queries(contributing_rows, parser)
        grouped_lower_bounds = aggregate_group(
            groups, lower_query_ids, contributing_rows, lower_configurations, parser
        )
        lower_bounds = {
            name: {
                selectivity: sum(point.values()) / len(point)
                for selectivity, point in group_values.items()
            }
            for name, group_values in grouped_lower_bounds.items()
        }

    output_dir = args.output_dir or args.folder
    output_dir.mkdir(parents=True, exist_ok=True)
    output_prefix = "query_time_selectivity" if kind == "time" else "query_pruning_selectivity"
    for index, (name, _) in enumerate(groups, start=1):
        slug = re.sub(r"[^a-z0-9]+", "_", name.lower()).strip("_") or "group"
        output_path = output_dir / f"{output_prefix}_{index:02d}_{slug}.png"
        plot_group(
            kind,
            output_path,
            name.replace("\n", " "),
            grouped_values[name],
            selected_configurations,
            labels,
            color_overrides,
            lower_bounds[name] if lower_bounds is not None else None,
        )
        print(f"Saved {output_path}")
