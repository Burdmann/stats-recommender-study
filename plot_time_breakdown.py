import argparse
import csv
import math
import re
from pathlib import Path

import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt
from matplotlib.colors import to_rgb
from matplotlib.patches import Patch


SIZE_GROUPS = ("1", "10", "50", "200")
UNIT_TO_MILLISECONDS = {"s": 1000.0, "ms": 1.0, "us": 0.001, "µs": 0.001}


def read_timings(path: Path, parser: argparse.ArgumentParser):
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

            timings = {}
            units = set()
            for row_number, row in enumerate(reader, start=2):
                query = row["Query"]
                if not query:
                    parser.error(f"{path}, row {row_number}: Query is empty")
                if query in timings:
                    parser.error(f"{path}, row {row_number}: duplicate query {query!r}")

                units.add(row["Unit"])
                timings[query] = {}
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
                    timings[query][configuration] = value
    except OSError as error:
        parser.error(str(error))

    if len(units) != 1 or not units:
        parser.error(f"{path} must use one consistent, non-empty Unit")
    return timings, configurations, units.pop()


def configuration_groups(configurations):
    groups = {"1": ["MIN_MAX"], "10": [], "50": [], "200": []}
    for configuration in configurations:
        match = re.search(r"_(1|10|50|200)$", configuration)
        if match:
            groups[match.group(1)].append(configuration)
    return {size: [name for name in names if name in configurations]
            for size, names in groups.items()}


def parse_query_groups(group_specs, available_queries, parser):
    if not group_specs:
        return [(query, [query]) for query in available_queries]

    groups = []
    seen_names = set()
    seen_queries = set()
    for specification in group_specs:
        if "=" not in specification:
            parser.error(
                f"invalid group {specification!r}; expected NAME=QUERY,QUERY"
            )
        name, query_list = specification.split("=", 1)
        name = name.strip()
        members = [query.strip() for query in query_list.split(",")]
        if not name or not query_list.strip() or any(not query for query in members):
            parser.error(
                f"invalid group {specification!r}; expected NAME=QUERY,QUERY"
            )
        if name in seen_names:
            parser.error(f"duplicate query group name {name!r}")
        if len(set(members)) != len(members):
            parser.error(f"group {name!r} contains a query more than once")
        unknown_queries = [query for query in members if query not in available_queries]
        if unknown_queries:
            parser.error(
                f"group {name!r} contains unknown queries: {', '.join(unknown_queries)}"
            )
        overlapping_queries = [query for query in members if query in seen_queries]
        if overlapping_queries:
            parser.error(
                f"queries can only belong to one group: {', '.join(overlapping_queries)}"
            )
        groups.append((name, members))
        seen_names.add(name)
        seen_queries.update(members)
    return groups


def aggregate_query_groups(groups, configurations, total_times, stats_times):
    grouped_total_times = {}
    grouped_stats_times = {}
    for name, members in groups:
        grouped_total_times[name] = {}
        grouped_stats_times[name] = {}
        for configuration in configurations:
            member_totals = [total_times[query][configuration] for query in members]
            member_stats = [stats_times[query][configuration] for query in members]
            for query, total, stats in zip(members, member_totals, member_stats):
                if stats > total:
                    raise ValueError(
                        f"{query}/{configuration}: stats_eval_time ({stats}) "
                        f"exceeds total_time ({total})"
                    )
            grouped_total_times[name][configuration] = sum(member_totals) / len(members)
            grouped_stats_times[name][configuration] = sum(member_stats) / len(members)
    return grouped_total_times, grouped_stats_times


def lighten_color(color, amount=0.5):
    red, green, blue = to_rgb(color)
    return (red + (1 - red) * amount, green + (1 - green) * amount, blue + (1 - blue) * amount)


def parse_configurations(configuration_specs, available_configurations, parser):
    if not configuration_specs:
        return available_configurations, {
            configuration: configuration for configuration in available_configurations
        }

    configurations = []
    labels = {}
    for specification in configuration_specs:
        if "=" not in specification:
            parser.error(
                f"invalid configuration {specification!r}; expected CONFIG=LABEL"
            )
        configuration, label = specification.split("=", 1)
        configuration = configuration.strip()
        label = label.strip()
        if not configuration or not label:
            parser.error(
                f"invalid configuration {specification!r}; expected CONFIG=LABEL"
            )
        if configuration not in available_configurations:
            parser.error(f"unknown configuration {configuration!r}")
        if configuration in labels:
            parser.error(f"configuration {configuration!r} was specified more than once")
        configurations.append(configuration)
        labels[configuration] = label
    return configurations, labels


def plot_configuration_group(
    group_name,
    output_name,
    configurations,
    configuration_labels,
    queries,
    total_times,
    stats_times,
    unit,
    output_dir,
):
    if not configurations:
        return None

    to_milliseconds = UNIT_TO_MILLISECONDS[unit]
    query_positions = list(range(len(queries)))
    cluster_width = 0.8
    bar_width = cluster_width / len(configurations)
    figure_width = max(14, len(queries) * 0.55)
    figure, axis = plt.subplots(figsize=(figure_width, 8))
    palette = plt.get_cmap("tab10")
    configuration_colors = [
        palette(index % palette.N) for index in range(len(configurations))
    ]

    for index, configuration in enumerate(configurations):
        offsets = [
            position + (index - (len(configurations) - 1) / 2) * bar_width
            for position in query_positions
        ]
        stats_values = [stats_times[query][configuration] for query in queries]
        remainder_values = []
        for query, stats_value in zip(queries, stats_values):
            total_value = total_times[query][configuration]
            if stats_value > total_value:
                raise ValueError(
                    f"{query}/{configuration}: stats_eval_time ({stats_value}) "
                    f"exceeds total_time ({total_value})"
                )
            remainder_values.append(total_value - stats_value)

        color = configuration_colors[index]
        axis.bar(
            offsets,
            [value * to_milliseconds for value in remainder_values],
            width=bar_width * 0.9,
            color=color,
            edgecolor="#333333",
            linewidth=0.35,
        )
        axis.bar(
            offsets,
            [value * to_milliseconds for value in stats_values],
            width=bar_width * 0.9,
            bottom=[value * to_milliseconds for value in remainder_values],
            color=lighten_color(color),
            edgecolor="#333333",
            linewidth=0.35,
        )

    axis.set_xticks(query_positions)
    axis.set_xticklabels(queries, rotation=45, ha="right")
    axis.set_xlabel("Query")
    axis.set_ylabel("Time (ms)")
    axis.set_title(f"Query time breakdown ({group_name})")
    axis.grid(axis="y", alpha=0.25)
    axis.set_axisbelow(True)

    figure.legend(
        handles=[
            Patch(
                facecolor=configuration_colors[index],
                edgecolor="#333333",
                label=configuration_labels[configuration],
            )
            for index, configuration in enumerate(configurations)
        ],
        title="Configuration",
        loc="upper center",
        bbox_to_anchor=(0.5, 0.99),
        ncol=min(len(configurations), 4),
    )
    figure.legend(
        handles=[
            Patch(facecolor="#666666", label="Other query time (darker shade)"),
            Patch(facecolor="#D9D9D9", label="Statistics evaluation (lighter shade)"),
        ],
        title="Time component",
        loc="upper center",
        bbox_to_anchor=(0.5, 0.91),
        ncol=2,
    )

    figure.tight_layout(rect=(0, 0, 1, 0.84))
    output_path = output_dir / f"query_time_breakdown_{output_name}.png"
    figure.savefig(output_path, dpi=160)
    plt.close(figure)
    return output_path


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Create clustered, stacked query-time charts from total_time.csv and "
            "stats_eval_time.csv."
        )
    )
    parser.add_argument(
        "folder",
        type=Path,
        help="folder containing total_time.csv and stats_eval_time.csv",
    )
    parser.add_argument(
        "--group",
        action="append",
        metavar="NAME=QUERY,QUERY",
        help=(
            "plot the listed queries as one named group (repeatable); when groups "
            "are provided, queries not listed are omitted"
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

    total_times, configurations, total_unit = read_timings(
        args.folder / "total_time.csv", parser
    )
    stats_times, stats_configurations, stats_unit = read_timings(
        args.folder / "stats_eval_time.csv", parser
    )
    if configurations != stats_configurations:
        parser.error("total_time.csv and stats_eval_time.csv configurations differ")
    if total_times.keys() != stats_times.keys():
        parser.error("total_time.csv and stats_eval_time.csv query rows differ")
    if total_unit != stats_unit:
        parser.error("total_time.csv and stats_eval_time.csv use different units")
    if total_unit not in UNIT_TO_MILLISECONDS:
        parser.error(
            f"unsupported time unit {total_unit!r}; expected s, ms, us, or µs"
        )
    selected_configurations, configuration_labels = parse_configurations(
        args.configuration, configurations, parser
    )

    query_groups = parse_query_groups(args.group, list(total_times), parser)
    try:
        plot_total_times, plot_stats_times = aggregate_query_groups(
            query_groups, configurations, total_times, stats_times
        )
    except ValueError as error:
        parser.error(str(error))
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
            plot_total_times,
            plot_stats_times,
            total_unit,
            args.folder,
        )
        print(f"Saved {output_path}")

    dictionary_configurations = [
        configuration for configuration in selected_configurations
        if configuration == "DICTIONARY"
    ]
    if dictionary_configurations:
        output_path = plot_configuration_group(
            "DICTIONARY",
            "dictionary",
            dictionary_configurations,
            configuration_labels,
            plot_queries,
            plot_total_times,
            plot_stats_times,
            total_unit,
            args.folder,
        )
        print(f"Saved {output_path}")


if __name__ == "__main__":
    main()
