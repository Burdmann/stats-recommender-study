import argparse
import csv
import math
from pathlib import Path

import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
from matplotlib.patches import Patch

from plot_time_breakdown import (
    SIZE_GROUPS,
    configuration_groups,
    parse_configurations,
    parse_query_groups,
    wrap_query_label,
)


def read_values(path: Path, parser: argparse.ArgumentParser):
    try:
        with path.open(newline="", encoding="utf-8") as csv_file:
            reader = csv.DictReader(csv_file)
            if reader.fieldnames is None:
                parser.error(f"{path} is empty")
            required_columns = {"Measurement", "Unit", "Query"}
            if not required_columns.issubset(reader.fieldnames):
                parser.error(f"{path} must contain Measurement, Unit, and Query columns")

            configurations = [
                column for column in reader.fieldnames if column not in required_columns
            ]
            if not configurations:
                parser.error(f"{path} does not contain any configuration columns")

            values = {}
            for row_number, row in enumerate(reader, start=2):
                query = row["Query"]
                if not query:
                    parser.error(f"{path}, row {row_number}: Query is empty")
                if query in values:
                    parser.error(f"{path}, row {row_number}: duplicate query {query!r}")

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

    return values, configurations


def aggregate_query_groups(groups, configurations, pruning_values, contributing_rows):
    grouped_pruning = {}
    grouped_lower_bounds = {}
    for name, members in groups:
        grouped_pruning[name] = {
            configuration: sum(
                pruning_values[query][configuration] for query in members
            ) / len(members)
            for configuration in configurations
        }
        grouped_lower_bounds[name] = sum(
            sum(contributing_rows[query].values()) / len(contributing_rows[query])
            for query in members
        ) / len(members)
    return grouped_pruning, grouped_lower_bounds


def plot_configuration_group(
    group_name,
    output_name,
    configurations,
    configuration_labels,
    queries,
    pruning_values,
    lower_bounds,
    output_dir,
):
    if not configurations:
        return None

    query_positions = list(range(len(queries)))
    cluster_width = 0.8
    bar_width = cluster_width / len(configurations)
    figure_width = max(14, len(queries) * 0.55)
    figure, axis = plt.subplots(figsize=(figure_width, 7))
    palette = plt.get_cmap("tab10")
    configuration_colors = [
        palette(index % palette.N) for index in range(len(configurations))
    ]

    for index, configuration in enumerate(configurations):
        offsets = [
            position + (index - (len(configurations) - 1) / 2) * bar_width
            for position in query_positions
        ]
        axis.bar(
            offsets,
            [pruning_values[query][configuration] for query in queries],
            width=bar_width * 0.9,
            color=configuration_colors[index],
            edgecolor="#333333",
            linewidth=0.35,
        )

    axis.scatter(
        query_positions,
        [lower_bounds[query] for query in queries],
        facecolor="white",
        edgecolor="#222222",
        marker="D",
        s=55,
        zorder=5,
    )
    axis.set_xticks(query_positions)
    axis.set_xticklabels(
        [wrap_query_label(query) for query in queries],
        rotation=45,
        ha="right",
    )
    axis.set_xlabel("Query")
    axis.set_ylabel("Rows scanned (lower is better)")
    axis.set_title(f"Rows scanned by approach ({group_name})")
    axis.tick_params(axis="both", labelsize=13)
    axis.xaxis.label.set_size(15)
    axis.yaxis.label.set_size(15)
    axis.title.set_size(17)
    axis.grid(axis="y", alpha=0.25)
    axis.set_axisbelow(True)

    legend_handles = [
        Patch(
            facecolor=configuration_colors[index],
            edgecolor="#333333",
            label=configuration_labels[configuration],
        )
        for index, configuration in enumerate(configurations)
    ]
    legend_handles.append(
        Line2D(
            [0],
            [0],
            color="#222222",
            linestyle="None",
            markerfacecolor="white",
            markeredgecolor="#222222",
            marker="D",
            label="Minimum possible",
        )
    )
    figure.legend(
        handles=legend_handles,
        loc="upper center",
        bbox_to_anchor=(0.5, 0.99),
        ncol=min(len(legend_handles), 5),
        fontsize=12,
    )
    figure.tight_layout(rect=(0, 0, 1, 0.9))
    output_path = output_dir / f"query_pruning_{output_name}.png"
    figure.savefig(output_path, dpi=160)
    plt.close(figure)
    return output_path


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Plot rows scanned by each approach with the qualifying-row lower bound."
        )
    )
    parser.add_argument(
        "folder",
        type=Path,
        help="folder containing pruning.csv and contributing_rows.csv",
    )
    parser.add_argument(
        "--group",
        action="append",
        metavar="NAME=QUERY,QUERY",
        help=(
            "plot the listed queries as one named group (repeatable); when groups "
            "are provided, queries not listed are omitted; use \\n in a group name "
            "to force a line break"
        ),
    )
    parser.add_argument(
        "--configuration",
        action="append",
        metavar="CONFIG=LABEL",
        help=(
            "plot only this configuration and show LABEL in the legend "
            "(repeatable); without this option, all configurations use their "
            "original names"
        ),
    )
    args = parser.parse_args()

    pruning_values, configurations = read_values(args.folder / "pruning.csv", parser)
    contributing_rows, contributing_configurations = read_values(
        args.folder / "contributing_rows.csv", parser
    )
    if configurations != contributing_configurations:
        parser.error("pruning.csv and contributing_rows.csv configurations differ")
    if pruning_values.keys() != contributing_rows.keys():
        parser.error("pruning.csv and contributing_rows.csv query rows differ")
    selected_configurations, configuration_labels = parse_configurations(
        args.configuration, configurations, parser
    )
    query_groups = parse_query_groups(args.group, list(pruning_values), parser)
    grouped_pruning, grouped_lower_bounds = aggregate_query_groups(
        query_groups, configurations, pruning_values, contributing_rows
    )
    plot_queries = [name for name, _ in query_groups]

    groups = configuration_groups(selected_configurations)
    for size in SIZE_GROUPS:
        if not groups[size]:
            continue
        output_path = plot_configuration_group(
            f"{size}x",
            f"{size}x",
            groups[size],
            configuration_labels,
            plot_queries,
            grouped_pruning,
            grouped_lower_bounds,
            args.folder,
        )
        print(f"Saved {output_path}")

    dictionary_configurations = [
        configuration
        for configuration in selected_configurations
        if configuration == "DICTIONARY"
    ]
    if dictionary_configurations:
        output_path = plot_configuration_group(
            "DICTIONARY",
            "dictionary",
            dictionary_configurations,
            configuration_labels,
            plot_queries,
            grouped_pruning,
            grouped_lower_bounds,
            args.folder,
        )
        print(f"Saved {output_path}")


if __name__ == "__main__":
    main()