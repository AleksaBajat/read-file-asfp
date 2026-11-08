"""Check the C++ output against a reference computed in Python.

Usage:
    ./gas_stations gas_stations.csv | python3 verify.py gas_stations.csv
    python3 verify.py gas_stations.csv cpp_output.txt

Line order doesn't matter: both sides are compared as {name: fields} dicts.
"""

import sys
from collections import defaultdict

MAX_DIFFS_SHOWN = 20


def to_tenths(s: str) -> int:
    # "-12.3" -> -123, done exactly (no float parsing), same as the C++ side
    neg = s.startswith("-")
    digits = s.lstrip("-").replace(".", "")
    return -int(digits) if neg else int(digits)


def reference(csv_path: str) -> dict[str, dict[str, str]]:
    stats = defaultdict(lambda: [10**9, -(10**9), 0, 0])  # min, max, sum, count
    with open(csv_path, buffering=1 << 20) as f:
        for line in f:
            name, temp = line.rstrip("\n").split(";")
            t = to_tenths(temp)
            s = stats[name]
            s[0] = min(s[0], t)
            s[1] = max(s[1], t)
            s[2] += t
            s[3] += 1

    # Format exactly like print_record in main.cpp (std::fixed, setprecision(1))
    return {
        name: {
            "min": f"{mn / 10.0:.1f}",
            "max": f"{mx / 10.0:.1f}",
            "avg": f"{sm / cnt / 10.0:.1f}",
            "sum": f"{sm / 10.0:.1f}",
            "count": str(cnt),
        }
        for name, (mn, mx, sm, cnt) in stats.items()
    }


def parse_cpp(lines) -> dict[str, dict[str, str]]:
    # "name | min: -1.2 | max: 3.4 | avg: 0.5 | sum: 9.9 | count: 7"
    result = {}
    for line in lines:
        line = line.strip()
        if not line:
            continue
        name, *fields = (part.strip() for part in line.split("|"))
        if name in result:
            raise SystemExit(f"duplicate name in C++ output: {name!r}")
        result[name] = {
            k.strip(): v.strip() for k, v in (f.split(":", 1) for f in fields)
        }
    return result


def main() -> int:
    if len(sys.argv) not in (2, 3):
        print(__doc__, file=sys.stderr)
        return 2

    expected = reference(sys.argv[1])
    if len(sys.argv) == 3:
        with open(sys.argv[2]) as f:
            actual = parse_cpp(f)
    else:
        actual = parse_cpp(sys.stdin)

    missing = expected.keys() - actual.keys()
    extra = actual.keys() - expected.keys()
    mismatched = [
        (name, expected[name], actual[name])
        for name in sorted(expected.keys() & actual.keys())
        if expected[name] != actual[name]
    ]

    for name in sorted(missing)[:MAX_DIFFS_SHOWN]:
        print(f"MISSING  {name}")
    for name in sorted(extra)[:MAX_DIFFS_SHOWN]:
        print(f"EXTRA    {name}")
    for name, exp, act in mismatched[:MAX_DIFFS_SHOWN]:
        diffs = ", ".join(
            f"{k}: expected {exp.get(k)} got {act.get(k)}"
            for k in exp.keys() | act.keys()
            if exp.get(k) != act.get(k)
        )
        print(f"DIFF     {name}: {diffs}")

    print(
        f"\nstations: expected {len(expected)}, got {len(actual)} | "
        f"missing {len(missing)}, extra {len(extra)}, mismatched {len(mismatched)}"
    )
    ok = not (missing or extra or mismatched)
    print("OK" if ok else "FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
