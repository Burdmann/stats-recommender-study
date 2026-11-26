import argparse
import zlib

import matplotlib.colors as mcolors
import matplotlib.pyplot as plt

DARK_RED = "#860000"
RED = "#CA0000"
DARK_GREEN = "#007D02"
DARK_PURPLE = "#3F0054"
TEAL = "#008C95"
ORANGE = "#E68600"
BLUE = "#0050A0"

APPROACH_COLORS = {
    "MIN_MAX": DARK_RED,
    "BLOOM_1": RED,
    "GREEDY_SIMPLE_1": DARK_GREEN,
    "GREEDY_1": DARK_GREEN,
    "NULLSPACE_1": TEAL,
    "OCCUPANCY_1": ORANGE,
    "NGRAM_1": BLUE,
    "CLUSTER_10": DARK_RED,
    "BLOOM_10": RED,
    "GREEDY_SIMPLE_10": DARK_GREEN,
    "GREEDY_10": DARK_GREEN,
    "NULLSPACE_10": TEAL,
    "OCCUPANCY_10": ORANGE,
    "NGRAM_10": BLUE,
    "CLUSTER_50": DARK_RED,
    "BLOOM_50": RED,
    "GREEDY_SIMPLE_50": DARK_GREEN,
    "GREEDY_50": DARK_GREEN,
    "NULLSPACE_50": TEAL,
    "OCCUPANCY_50": ORANGE,
    "NGRAM_50": BLUE,
    "CLUSTER_200": DARK_RED,
    "BLOOM_200": RED,
    "GREEDY_SIMPLE_200": DARK_GREEN,
    "GREEDY_200": DARK_GREEN,
    "NULLSPACE_200": TEAL,
    "OCCUPANCY_200": ORANGE,
    "NGRAM_200": BLUE,
    "DICTIONARY": DARK_PURPLE,
}


def add_color_argument(parser: argparse.ArgumentParser):
    parser.add_argument(
        "--color",
        action="append",
        metavar="NAME=COLOR",
        help="override a configuration or statistic color (repeatable; e.g. BLOOM_1=#e15759)",
    )


def parse_color_overrides(color_specs, available_names, parser):
    if not color_specs:
        return {}

    overrides = {}
    for specification in color_specs:
        if "=" not in specification:
            parser.error(f"invalid color {specification!r}; expected NAME=COLOR")
        name, color = specification.split("=", 1)
        name = name.strip()
        color = color.strip()
        if not name or not color:
            parser.error(f"invalid color {specification!r}; expected NAME=COLOR")
        if name not in available_names:
            parser.error(f"unknown color name {name!r}")
        if name in overrides:
            parser.error(f"color for {name!r} was specified more than once")
        try:
            mcolors.to_rgba(color)
        except ValueError:
            parser.error(f"invalid Matplotlib color {color!r} for {name!r}")
        overrides[name] = color
    return overrides


def color_for(name, overrides, defaults=None):
    if name in overrides:
        return overrides[name]
    if defaults and name in defaults:
        return defaults[name]

    palette = plt.get_cmap("tab20")
    palette_index = zlib.crc32(name.encode("utf-8")) % palette.N
    return palette(palette_index)