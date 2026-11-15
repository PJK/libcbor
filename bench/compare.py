#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
# SPDX-License-Identifier: MIT
"""Compares two libcbor_bench result files and prints a Markdown report.

Each input line is "case<TAB>metric<TAB>value". bench/compare.sh produces
them; see bench/README.md for what the metrics mean.
"""

import argparse
import sys

# Instruction count changes, in percent.
INSTRUCTIONS_FAIL = 1.0
INSTRUCTIONS_WARN = 0.3
# Library text size change that is worth a warning, in percent.
SIZE_WARN = 1.0

# Allocation and reallocation counts are exact: any increase is a regression.
COUNT_METRICS = ("allocs", "reallocs")
# Requested and peak bytes are exact too, but small increases are often an
# accepted cost of a struct change, so they only fail above this, in percent.
BYTE_METRICS = ("alloc_bytes", "peak_bytes")
BYTES_FAIL = 1.0
ALLOC_METRICS = COUNT_METRICS + BYTE_METRICS
# Metric order in the report. Unknown metrics follow, in file order.
METRIC_ORDER = ("instructions",) + ALLOC_METRICS + (
    "ns_min",
    "ns_median",
    "text_bytes",
)

REGRESSION, WARNING, IMPROVEMENT, SAME, INFO = range(5)
SYMBOLS = {
    REGRESSION: "❌",
    WARNING: "⚠️",
    IMPROVEMENT: "✅",
    SAME: "",
    INFO: "",
}


def read(path):
    results = {}
    with open(path, encoding="utf-8") as f:
        for number, line in enumerate(f, 1):
            line = line.rstrip("\n")
            if not line:
                continue
            fields = line.split("\t")
            if len(fields) != 3:
                sys.exit(f"{path}:{number}: expected 3 tab-separated fields")
            case, metric, value = fields
            results[(case, metric)] = float(value)
    return results


def percent(base, head):
    if base == head:
        return 0.0
    if base == 0:
        return float("inf")
    return (head - base) / base * 100


def classify(metric, base, head):
    change = percent(base, head)
    if metric == "instructions":
        if change > INSTRUCTIONS_FAIL:
            return REGRESSION
        if change > INSTRUCTIONS_WARN:
            return WARNING
        if change < -INSTRUCTIONS_WARN:
            return IMPROVEMENT
        return SAME
    if metric in COUNT_METRICS:
        if head > base:
            return REGRESSION
        if head < base:
            return IMPROVEMENT
        return SAME
    if metric in BYTE_METRICS:
        if change > BYTES_FAIL:
            return REGRESSION
        if head > base:
            return WARNING
        if head < base:
            return IMPROVEMENT
        return SAME
    if metric == "text_bytes":
        if change > SIZE_WARN:
            return WARNING
        return SAME
    # Wall-clock time is too noisy to judge automatically.
    return INFO


def fmt_value(value):
    if value == int(value):
        return f"{int(value):,}"
    return f"{value:,.1f}"


def fmt_change(base, head):
    if base is None or head is None:
        return "n/a"
    change = percent(base, head)
    if change == float("inf"):
        return "new"
    return f"{change:+.2f}%"


def table(rows):
    lines = ["| Case | Metric | Base | Head | Change | |", "|---|---|--:|--:|--:|:-:|"]
    for case, metric, base, head, status in rows:
        lines.append(
            f"| `{case}` | {metric} | "
            f"{fmt_value(base) if base is not None else '—'} | "
            f"{fmt_value(head) if head is not None else '—'} | "
            f"{fmt_change(base, head)} | {SYMBOLS[status]} |"
        )
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("base")
    parser.add_argument("head")
    parser.add_argument("--base-label", default="base")
    parser.add_argument("--head-label", default="head")
    parser.add_argument("--config", default="")
    parser.add_argument(
        "--fail",
        action="store_true",
        help="exit with status 1 if there is a regression",
    )
    args = parser.parse_args()

    base = read(args.base)
    head = read(args.head)
    keys = list(base) + [k for k in head if k not in base]
    metrics = sorted(
        {m for _, m in keys},
        key=lambda m: (METRIC_ORDER.index(m) if m in METRIC_ORDER else 99, m),
    )

    rows = []
    for metric in metrics:
        for case, m in keys:
            if m != metric:
                continue
            b = base.get((case, m))
            h = head.get((case, m))
            if b is None or h is None:
                status = WARNING
            else:
                status = classify(m, b, h)
            rows.append((case, m, b, h, status))

    counts = {s: sum(1 for r in rows if r[4] == s) for s in SYMBOLS}
    cases = len({r[0] for r in rows})
    print("## libcbor benchmark comparison\n")
    print(f"- Base: {args.base_label}")
    print(f"- Head: {args.head_label}")
    if args.config:
        print(f"- Configuration: {args.config}")
    print(
        f"- Thresholds: instructions ❌ above +{INSTRUCTIONS_FAIL}%, "
        f"⚠️ above +{INSTRUCTIONS_WARN}%; allocation counts ❌ on any increase; "
        f"allocated bytes ❌ above +{BYTES_FAIL}%, ⚠️ on any increase; "
        f"library size ⚠️ above +{SIZE_WARN}%; wall-clock time is not judged"
    )
    print()
    print(
        f"**{counts[REGRESSION]} regressions, {counts[WARNING]} warnings, "
        f"{counts[IMPROVEMENT]} improvements** across {cases} cases.\n"
    )

    flagged = [r for r in rows if r[4] in (REGRESSION, WARNING, IMPROVEMENT)]
    if flagged:
        flagged.sort(key=lambda r: r[4])
        print(table(flagged))
    else:
        print("No changes above the thresholds.")
    print()

    for metric in metrics:
        metric_rows = [r for r in rows if r[1] == metric]
        print(f"<details><summary>All results: {metric}</summary>\n")
        print(table(metric_rows))
        print("\n</details>\n")

    if args.fail and counts[REGRESSION]:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
