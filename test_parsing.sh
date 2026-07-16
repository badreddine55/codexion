#!/bin/bash
# Test harness for Codexion argument parsing.
# Usage: ./test_parsing.sh ./codexion
#   (pass the path to your compiled binary as the first argument)

BIN="${1:-./codexion}"

if [ ! -x "$BIN" ]; then
    echo "Binary '$BIN' not found or not executable."
    echo "Usage: $0 <path_to_binary>"
    exit 1
fi

PASS=0
FAIL=0

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
)

echo "Testing binary: $BIN"
echo "------------------------------------------------------------"

for entry in "${TESTS[@]}"; do
    expected="${entry%%|*}"
    rest="${entry#*|}"
    desc="${rest%%|*}"
    args="${rest#*|}"

    # shellcheck disable=SC2086
    "$BIN" $args > /tmp/codexion_test_out.txt 2>&1
    actual=$?

    if [ "$actual" -ne 0 ]; then actual=1; fi

    if [ "$actual" -eq "$expected" ]; then
        echo "PASS  | $desc"
        PASS=$((PASS+1))
    else
        echo "FAIL  | $desc  (expected exit=$expected, got exit=$actual)"
        echo "       args: [$args]"
        echo "       output: $(cat /tmp/codexion_test_out.txt)"
        FAIL=$((FAIL+1))
    fi
done

echo "------------------------------------------------------------"
echo "Passed: $PASS   Failed: $FAIL   Total: $((PASS+FAIL))"

rm -f /tmp/codexion_test_out.txt