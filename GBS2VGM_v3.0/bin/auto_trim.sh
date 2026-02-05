#!/bin/bash
# Auto-trim VGM files based on loop_report.txt
# This script reads the loop report and automatically trims VGM files with detected loop points

INPUT_DIR="DQ12_VGM_v2.2"
OUTPUT_DIR="DQ12_VGM_v2.2_trimmed"
LOOP_REPORT="$INPUT_DIR/loop_report.txt"

# Create output directory
mkdir -p "$OUTPUT_DIR"

echo "=== VGM Auto-Trim Script ==="
echo "Input directory: $INPUT_DIR"
echo "Output directory: $OUTPUT_DIR"
echo "Loop report: $LOOP_REPORT"
echo ""

# Check if loop report exists
if [ ! -f "$LOOP_REPORT" ]; then
    echo "Error: Loop report not found: $LOOP_REPORT"
    exit 1
fi

# Check if vgm_trim.exe exists
if [ ! -f "vgm_trim.exe" ]; then
    echo "Error: vgm_trim.exe not found. Please run 'make utils' first."
    exit 1
fi

# Parse loop report and trim files
trimmed_count=0
skipped_count=0
current_file=""
best_loop_start=""
best_loop_end=""
has_exclamation=false

while IFS= read -r line; do
    # Check if this is a file header line
    if [[ $line =~ ^##\ (.+\.vgm)$ ]]; then
        # Process previous file
        if [ -n "$current_file" ]; then
            if [ -n "$best_loop_start" ] && [ -n "$best_loop_end" ]; then
                # Has loop point, trim it
                input_file="$INPUT_DIR/$current_file"
                output_file="$OUTPUT_DIR/$current_file"

                if [ -f "$input_file" ]; then
                    echo "Trimming: $current_file"
                    if [ "$has_exclamation" = true ]; then
                        echo "  Loop: $best_loop_start -> $best_loop_end (best)"
                    else
                        echo "  Loop: $best_loop_start -> $best_loop_end"
                    fi

                    # Run vgm_trim with command-line parameters
                    ./vgm_trim.exe "$input_file" 0 "$best_loop_start" "$best_loop_end" "$output_file" > /dev/null 2>&1

                    if [ -f "$output_file" ]; then
                        # Get file sizes
                        original_size=$(stat -c%s "$input_file" 2>/dev/null || stat -f%z "$input_file" 2>/dev/null)
                        trimmed_size=$(stat -c%s "$output_file" 2>/dev/null || stat -f%z "$output_file" 2>/dev/null)

                        if [ -n "$original_size" ] && [ -n "$trimmed_size" ]; then
                            reduction=$((100 - (trimmed_size * 100 / original_size)))
                            echo "  Size: $(numfmt --to=iec-i --suffix=B $original_size 2>/dev/null || echo "$original_size bytes") -> $(numfmt --to=iec-i --suffix=B $trimmed_size 2>/dev/null || echo "$trimmed_size bytes") (${reduction}% reduction)"
                        fi

                        trimmed_count=$((trimmed_count + 1))
                    else
                        echo "  Error: Failed to create trimmed file"
                    fi
                else
                    echo "Warning: File not found: $input_file"
                fi
            else
                # No loop point
                echo "Skipping: $current_file (no loop point)"
                skipped_count=$((skipped_count + 1))
            fi
        fi

        # Start new file
        current_file="${BASH_REMATCH[1]}"
        best_loop_start=""
        best_loop_end=""
        has_exclamation=false
        continue
    fi

    # Check if this is a "no loop" line
    if [[ $line == "无循环点" ]]; then
        if [ -n "$current_file" ]; then
            echo "Skipping: $current_file (no loop point)"
            skipped_count=$((skipped_count + 1))
            current_file=""
            best_loop_start=""
            best_loop_end=""
            has_exclamation=false
        fi
        continue
    fi

    # Check if this is a loop point line with "!" (best loop point)
    if [[ $line =~ ^([0-9]+)[[:space:]]+[0-9:.]+[[:space:]]+![[:space:]]+([0-9]+) ]]; then
        loop_start="${BASH_REMATCH[1]}"
        loop_end="${BASH_REMATCH[2]}"

        # Loop point with "!" has highest priority
        best_loop_start="$loop_start"
        best_loop_end="$loop_end"
        has_exclamation=true
        continue
    fi

    # Check if this is a regular loop point line (without "!")
    if [[ $line =~ ^([0-9]+)[[:space:]]+[0-9:.]+[[:space:]]+([0-9]+)[[:space:]]+[0-9:.]+[[:space:]]+[0-9]+ ]]; then
        loop_start="${BASH_REMATCH[1]}"
        loop_end="${BASH_REMATCH[2]}"

        # Use this loop point if we haven't found one yet, or if we don't have a "!" marked one
        if [ -z "$best_loop_start" ] && [ "$has_exclamation" = false ]; then
            best_loop_start="$loop_start"
            best_loop_end="$loop_end"
        fi
        continue
    fi
done < "$LOOP_REPORT"

# Process last file
if [ -n "$current_file" ]; then
    if [ -n "$best_loop_start" ] && [ -n "$best_loop_end" ]; then
        # Has loop point, trim it
        input_file="$INPUT_DIR/$current_file"
        output_file="$OUTPUT_DIR/$current_file"

        if [ -f "$input_file" ]; then
            echo "Trimming: $current_file"
            if [ "$has_exclamation" = true ]; then
                echo "  Loop: $best_loop_start -> $best_loop_end (best)"
            else
                echo "  Loop: $best_loop_start -> $best_loop_end"
            fi

            # Run vgm_trim with command-line parameters
            ./vgm_trim.exe "$input_file" 0 "$best_loop_start" "$best_loop_end" "$output_file" > /dev/null 2>&1

            if [ -f "$output_file" ]; then
                # Get file sizes
                original_size=$(stat -c%s "$input_file" 2>/dev/null || stat -f%z "$input_file" 2>/dev/null)
                trimmed_size=$(stat -c%s "$output_file" 2>/dev/null || stat -f%z "$output_file" 2>/dev/null)

                if [ -n "$original_size" ] && [ -n "$trimmed_size" ]; then
                    reduction=$((100 - (trimmed_size * 100 / original_size)))
                    echo "  Size: $(numfmt --to=iec-i --suffix=B $original_size 2>/dev/null || echo "$original_size bytes") -> $(numfmt --to=iec-i --suffix=B $trimmed_size 2>/dev/null || echo "$trimmed_size bytes") (${reduction}% reduction)"
                fi

                trimmed_count=$((trimmed_count + 1))
            else
                echo "  Error: Failed to create trimmed file"
            fi
        else
            echo "Warning: File not found: $input_file"
        fi
    else
        # No loop point
        echo "Skipping: $current_file (no loop point)"
        skipped_count=$((skipped_count + 1))
    fi
fi

echo ""
echo "=== Summary ==="
echo "Trimmed: $trimmed_count files"
echo "Skipped: $skipped_count files (no loop point)"
echo "Output directory: $OUTPUT_DIR"
echo ""
echo "Done!"
