#!/usr/bin/env python3
import json
import re
import hashlib
from pathlib import Path

ROOT = Path.home() / ".codex" / "sessions"

THREAD_RE = re.compile(
    r"rollout-\d{4}-\d{2}-\d{2}T\d{2}-\d{2}-\d{2}-(.+)\.jsonl$"
)
GOAL_RE = re.compile(
    r"<untrusted_objective>\n(.*?)\n</untrusted_objective>\n\n"
    r"Budget:\n"
    r"- Time spent pursuing goal: (\d+) seconds\n"
    r"- Tokens used: (\d+)",
    re.S,
)

def clean_objective(objective: str) -> str:
    return " ".join(objective.split())


def thread_id(path: Path) -> str:
    match = THREAD_RE.match(path.name)
    return match.group(1) if match else str(path)


def goal_key(thread: str, objective: str) -> str:
    normalized = f"{thread}\0{clean_objective(objective)}"
    return hashlib.sha1(normalized.encode("utf-8")).hexdigest()

def duration(seconds: int) -> str:
    h, rem = divmod(seconds, 3600)
    m, s = divmod(rem, 60)
    return f"{h}h {m:02d}m {s:02d}s" if h else f"{m}m {s:02d}s"


def get_goal(goals, thread: str, objective: str):
    key = goal_key(thread, objective)
    if key not in goals:
        goals[key] = {
            "thread": thread,
            "objective": clean_objective(objective),
            "seconds": 0,
            "tokens": 0,
            "status": "seen",
            "first_seen": "",
            "last_seen": "",
            "source": "",
        }
    return key, goals[key]


def update_accounting(goal, seconds: int, tokens: int, source: str):
    if seconds > goal["seconds"] or (
        seconds == goal["seconds"] and tokens > goal["tokens"]
    ):
        goal["seconds"] = seconds
        goal["tokens"] = tokens
        goal["source"] = source


def ingest_tool_output(goals, output, ts: str, fallback_thread: str):
    if not isinstance(output, (str, bytes, bytearray)):
        return None

    try:
        data = json.loads(output)
    except (TypeError, json.JSONDecodeError):
        return None

    if not isinstance(data, dict) or not isinstance(data.get("goal"), dict):
        return None

    raw_goal = data["goal"]
    objective = raw_goal.get("objective")
    if not objective:
        return None

    thread = raw_goal.get("threadId") or fallback_thread
    key, goal = get_goal(goals, thread, objective)

    if not goal["first_seen"] or ts < goal["first_seen"]:
        goal["first_seen"] = ts
    if ts > goal["last_seen"]:
        goal["last_seen"] = ts

    seconds = int(raw_goal.get("timeUsedSeconds") or 0)
    tokens = int(raw_goal.get("tokensUsed") or 0)
    update_accounting(goal, seconds, tokens, "goal_tool")

    if raw_goal.get("status"):
        goal["status"] = raw_goal["status"]

    return key


def main() -> None:
    goals = {}

    for path in sorted(ROOT.rglob("*.jsonl")):
        current_key = None
        current_thread = thread_id(path)

        with path.open("r", encoding="utf-8", errors="replace") as f:
            for line in f:
                try:
                    event = json.loads(line)
                except json.JSONDecodeError:
                    continue

                ts = event.get("timestamp", "")
                payload = event.get("payload") or {}

                if payload.get("type") == "message" and payload.get("role") == "developer":
                    text = "".join(
                        item.get("text", "")
                        for item in payload.get("content", [])
                        if isinstance(item, dict)
                    )
                    match = GOAL_RE.search(text)
                    if not match:
                        continue

                    objective = match.group(1).strip()
                    seconds = int(match.group(2))
                    tokens = int(match.group(3))

                    current_key, goal = get_goal(goals, current_thread, objective)

                    if not goal["first_seen"] or ts < goal["first_seen"]:
                        goal["first_seen"] = ts
                    if ts > goal["last_seen"]:
                        goal["last_seen"] = ts

                    update_accounting(goal, seconds, tokens, "budget_block")

                elif payload.get("type") == "function_call_output":
                    key = ingest_tool_output(
                        goals,
                        payload.get("output") or "",
                        ts,
                        current_thread,
                    )
                    if key:
                        current_key = key

                elif (
                    payload.get("type") == "function_call"
                    and payload.get("name") == "update_goal"
                    and current_key
                ):
                    goals[current_key]["status"] = "complete"
                    if ts > goals[current_key]["last_seen"]:
                        goals[current_key]["last_seen"] = ts

    rows = sorted(goals.values(), key=lambda g: g["seconds"], reverse=True)

    total_seconds = sum(g["seconds"] for g in rows)
    total_tokens = sum(g["tokens"] for g in rows)

    print(f"goals: {len(rows)}")
    print(f"total time: {duration(total_seconds)}")
    print(f"total tokens: {total_tokens:,}")
    print()

    for goal in rows:
        objective = " ".join(goal["objective"].split())
        if len(objective) > 150:
            objective = objective[:147] + "..."

        print(
            f"{duration(goal['seconds']):>11}  "
            f"{goal['tokens']:>12,} tokens  "
            f"{goal['status']:<8}  "
            f"{objective}"
        )


if __name__ == "__main__":
    main()
