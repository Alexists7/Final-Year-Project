#!/bin/bash

# Bash script to calculate metrics of embedded application and save it to a csv file

OUTPUT_FILE="system_metrics2.csv"
FPS=60
INTERVAL_SECONDS=60

echo "Timestamp,CPU(%),Memory(%),CPU_Per_Frame(%)" > "$OUTPUT_FILE"

while true; do
    TIMESTAMP=$(date +"%Y-%m-%d %H:%M:%S")

    CPU_USAGE=$(top -bn1 | grep "Cpu(s)" | sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk '{print 100 - $1}')
    CPU_PER_FRAME=$(echo "scale=2; $CPU_USAGE / $FPS" | bc)

    MEMORY_USAGE=$(free -m | awk '/Mem:/ {print $3/$2 * 100}')

    echo "$TIMESTAMP,$CPU_USAGE,$MEMORY_USAGE,$CPU_PER_FRAME" >> "$OUTPUT_FILE"

    sleep $INTERVAL_SECONDS
done
