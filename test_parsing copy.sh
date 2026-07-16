#!/bin/bash
# Test harness for Codexion argument parsing + memory safety.
# Usage: ./test_parsing.sh ./codexion [timeout_seconds]
#   (pass the path to your compiled binary as the first argument)
#
# Each test case is run once, under valgrind memcheck, and is judged on TWO
# independent axes:
#   1. Exit code: did the program accept/reject the arguments as expected?
#   2. Valgrind: did memcheck report zero errors and zero definite/indirect
#      leaks for that run?
# A test only PASSes if both axes are clean. This catches cases where parsing
# logic is correct but cleanup on that particular path corrupts/leaks memory
# (e.g. a rejected run that still crashes or leaks on the way out).

BIN="${1:-./codexion}"
TIMEOUT_SECS="${2:-15}"

if [ ! -x "$BIN" ]; then
    echo "Binary '$BIN' not found or not executable."
    echo "Usage: $0 <path_to_binary> [timeout_seconds]"
    exit 1
fi

if ! command -v valgrind >/dev/null 2>&1; then
    echo "valgrind not found in PATH. Install it (e.g. apt install valgrind) and re-run."
    exit 1
fi

# All results (summary + per-test valgrind logs) are saved on disk, not /tmp,
# so they survive after the run and can be attached/reviewed later.
STAMP="$(date +%Y%m%d_%H%M%S)"
RESULTS_DIR="test_results_${STAMP}"
mkdir -p "$RESULTS_DIR"
SUMMARY_FILE="$RESULTS_DIR/summary.log"

# Everything printed from here on is mirrored into $SUMMARY_FILE via `tee`.
exec > >(tee -a "$SUMMARY_FILE") 2>&1

PASS=0
FAIL=0
VG_FAIL=0

# Each test: "expected_result|description|arg1 arg2 arg3 ..."
# expected_result: 0 = should be ACCEPTED, 1 = should be REJECTED
# NOTE: some "expected" values below reflect a starting assumption
# (only number_of_coders must be > 0; the other 6 numeric fields
# only need to be >= 0). Adjust the expected column yourself once
# you've made your own field-by-field decision.
TESTS=(
"0|valid baseline|5 800 200 200 200 3 200 fifo"
"0|valid baseline edf|5 800 200 200 200 3 200 edf"
"1|no arguments at all|"
"1|too few arguments|5 800 200 200 200 3 200"
"1|too many arguments|5 800 200 200 200 3 200 fifo extra"
"1|number_of_coders is negative|-5 800 200 200 200 3 200 fifo"
"1|number_of_coders is zero|0 800 200 200 200 3 200 fifo"
"1|number_of_coders is non-integer|5a 800 200 200 200 3 200 fifo"
"1|time_to_burnout is negative|5 -800 200 200 200 3 200 fifo"
"0|time_to_burnout is zero|5 0 200 200 200 3 200 fifo"
"1|time_to_burnout has a decimal point|5 800.5 200 200 200 3 200 fifo"
"1|time_to_compile is negative|5 800 -200 200 200 3 200 fifo"
"0|time_to_compile is zero|5 800 0 200 200 3 200 fifo"
"1|time_to_debug is non-integer|5 800 200 2a0 200 3 200 fifo"
"0|time_to_debug is zero|5 800 200 0 200 3 200 fifo"
"1|time_to_refactor is negative|5 800 200 200 -200 3 200 fifo"
"0|time_to_refactor is zero|5 800 200 200 0 3 200 fifo"
"1|number_of_compiles_required is negative|5 800 200 200 200 -3 200 fifo"
"0|number_of_compiles_required is zero|5 800 200 200 200 0 200 fifo"
"1|dongle_cooldown is negative|5 800 200 200 200 3 -200 fifo"
"0|dongle_cooldown is zero|5 800 200 200 200 3 0 fifo"
"1|scheduler is invalid word|5 800 200 200 200 3 200 round_robin"
"1|scheduler wrong case FIFO|5 800 200 200 200 3 200 FIFO"
"1|scheduler wrong case EDF|5 800 200 200 200 3 200 EDF"
"1|scheduler has trailing space|5 800 200 200 200 3 200 fifo\ "
"1|scheduler is empty string|5 800 200 200 200 3 200 "
"1|one argument is an empty string|5 800  200 200 3 200 fifo"
"1|one argument has a plus sign|+5 800 200 200 200 3 200 fifo"
"1|one argument has leading whitespace|\" 5\" 800 200 200 200 3 200 fifo"
"1|huge number overflows int|99999999999999999999 800 200 200 200 3 200 fifo"
"1|number_of_coders is 1.5|1.5 800 200 200 200 3 200 fifo"
"0|leading zeros still valid number|05 800 200 200 200 3 200 fifo"
"1|valid coder count, invalid scheduler after it|4 800 200 200 200 3 200 notascheduler"
"1|valid coder count, negative field after it|4 800 200 200 -200 3 200 fifo"
)

OUT_LOG="$RESULTS_DIR/tmp_out.log"
VG_LOG="$RESULTS_DIR/tmp_vg.log"

echo "Testing binary: $BIN"
echo "Each run is wrapped in valgrind (--leak-check=full --show-leak-kinds=all --track-origins=yes)."
echo "Timeout per test: ${TIMEOUT_SECS}s"
echo "Results directory: $RESULTS_DIR"
echo "------------------------------------------------------------"

test_num=0
for entry in "${TESTS[@]}"; do
    test_num=$((test_num+1))
    expected="${entry%%|*}"
    rest="${entry#*|}"
    desc="${rest%%|*}"
    args="${rest#*|}"

    rm -f "$VG_LOG" "$OUT_LOG"

    # shellcheck disable=SC2086
    timeout "$TIMEOUT_SECS" valgrind \
        --leak-check=full \
        --show-leak-kinds=all \
        --track-origins=yes \
        --log-file="$VG_LOG" \
        "$BIN" $args > "$OUT_LOG" 2>&1
    raw_exit=$?

    timed_out=0
    if [ "$raw_exit" -eq 124 ] || [ "$raw_exit" -eq 137 ]; then
        timed_out=1
    fi

    actual_class=0
    if [ "$raw_exit" -ne 0 ]; then
        actual_class=1
    fi

    # --- axis 1: accept/reject exit-code check ---
    exit_ok=1
    if [ "$timed_out" -eq 1 ]; then
        exit_ok=0
    elif [ "$actual_class" -ne "$expected" ]; then
        exit_ok=0
    fi

    # --- axis 2: valgrind error / leak check ---
    vg_ok=1
    vg_notes=()
    if [ -f "$VG_LOG" ]; then
        if ! grep -q "ERROR SUMMARY: 0 errors" "$VG_LOG"; then
            vg_ok=0
            vg_notes+=("$(grep 'ERROR SUMMARY' "$VG_LOG" | tail -1)")
        fi
        if grep -q "definitely lost:" "$VG_LOG" && ! grep -q "definitely lost: 0 bytes" "$VG_LOG"; then
            vg_ok=0
            vg_notes+=("$(grep 'definitely lost:' "$VG_LOG" | tail -1)")
        fi
        if grep -q "indirectly lost:" "$VG_LOG" && ! grep -q "indirectly lost: 0 bytes" "$VG_LOG"; then
            vg_ok=0
            vg_notes+=("$(grep 'indirectly lost:' "$VG_LOG" | tail -1)")
        fi
    else
        vg_ok=0
        vg_notes+=("no valgrind log produced (process likely killed by timeout before it could report)")
    fi

    if [ "$exit_ok" -eq 1 ] && [ "$vg_ok" -eq 1 ]; then
        echo "PASS  | $desc"
        PASS=$((PASS+1))
    else
        echo "FAIL  | $desc"
        if [ "$timed_out" -eq 1 ]; then
            echo "       -> TIMEOUT after ${TIMEOUT_SECS}s (possible hang/deadlock)"
        elif [ "$exit_ok" -eq 0 ]; then
            echo "       -> exit code: expected=$expected got=$actual_class"
        fi
        safe_desc="$(echo "$desc" | tr -c 'a-zA-Z0-9' '_' | sed 's/_\+/_/g')"
        saved_vg="$RESULTS_DIR/test_${test_num}_${safe_desc}.valgrind.log"
        saved_out="$RESULTS_DIR/test_${test_num}_${safe_desc}.program_output.log"
        if [ "$vg_ok" -eq 0 ]; then
            VG_FAIL=$((VG_FAIL+1))
            for note in "${vg_notes[@]}"; do
                echo "       -> valgrind: $note"
            done
            cp "$VG_LOG" "$saved_vg" 2>/dev/null
            echo "       -> full valgrind log saved: $saved_vg"
        fi
        echo "       args: [$args]"
        echo "       program output: $(cat "$OUT_LOG" 2>/dev/null)"
        cp "$OUT_LOG" "$saved_out" 2>/dev/null
        FAIL=$((FAIL+1))
    fi
done

echo "------------------------------------------------------------"
echo "Passed: $PASS   Failed: $FAIL   Total: $((PASS+FAIL))"
echo "Failures involving a valgrind error/leak: $VG_FAIL"
echo "Full summary + all per-failure logs saved under: $RESULTS_DIR/"

rm -f "$OUT_LOG" "$VG_LOG"