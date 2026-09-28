import argparse
import csv

import matplotlib.pyplot as plt

STATISTIC_COLORS = {
	"NONE": "gray",
	"MIN_MAX": "darkgreen",
	"CLUSTER_SMALL": "indianred",
	"CLUSTER_MEDIUM": "firebrick",
	"CLUSTER_LARGE": "darkred",
	"BLOOM_SMALL": "cornflowerblue",
	"BLOOM_MEDIUM": "mediumblue",
	"BLOOM_LARGE": "darkblue",
	"DICTIONARY": "olive",
}

def main():
	parser = argparse.ArgumentParser(
		description="Plot pruning power against statistic size from a CSV file."
	)
	parser.add_argument("csv_file", help="CSV file containing size and pruning_power columns")
	parser.add_argument("output", help="Image file the plot should be written to")
	args = parser.parse_args()

	try:
		with open(args.csv_file, newline="", encoding="utf-8") as csv_file:
			reader = csv.DictReader(csv_file)
			required_columns = {"size", "pruning_power", "statistic"}
			if reader.fieldnames is None or not required_columns.issubset(reader.fieldnames):
				parser.error("CSV must contain 'size', 'pruning_power', and 'statistic' columns")

			points_by_statistic = {}
			for row_number, row in enumerate(reader, start=2):
				try:
					size = float(row["size"])
					pruning_power = float(row["pruning_power"])
				except (TypeError, ValueError):
					parser.error(f"invalid numeric value in CSV row {row_number}")
				statistic = row["statistic"]
				if statistic not in points_by_statistic:
					points_by_statistic[statistic] = ([], [])
				statistic_sizes, statistic_pruning_powers = points_by_statistic[statistic]
				statistic_sizes.append(size)
				statistic_pruning_powers.append(pruning_power)
	except OSError as error:
		parser.error(str(error))

	plt.figure(figsize=(8, 6))
	for statistic, (sizes, pruning_powers) in points_by_statistic.items():
		plt.scatter(
			sizes,
			pruning_powers,
			label=statistic,
			color=STATISTIC_COLORS.get(statistic),
		)
	x_min, x_max = plt.xlim()
	plt.xlim(x_min, x_min + (x_max - x_min) * 1.5)
	plt.legend(title="Statistic", loc="upper left", bbox_to_anchor=(1.02, 1))
	plt.xlabel("Size")
	plt.ylabel("Pruning power")
	plt.title("Pruning Power vs. Size")
	plt.grid(True, alpha=0.3)
	plt.tight_layout()
	plt.savefig(args.output, bbox_inches="tight")


if __name__ == "__main__":
	main()
