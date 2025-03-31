#!/bin/bash

OUTPUT_FILE="system_metrics2.csv"
FPS=60  # Your fixed frame rate
INTERVAL_SECONDS=60  # Log every minute

# CSV Header
echo "Timestamp,CPU(%),Memory(%),CPU_Per_Frame(%)" > "$OUTPUT_FILE"

while true; do
    TIMESTAMP=$(date +"%Y-%m-%d %H:%M:%S")

    # CPU usage (total)
    CPU_USAGE=$(top -bn1 | grep "Cpu(s)" | sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk '{print 100 - $1}')
    CPU_PER_FRAME=$(echo "scale=2; $CPU_USAGE / $FPS" | bc)  # CPU load per frame

    # Memory usage (%)
    MEMORY_USAGE=$(free -m | awk '/Mem:/ {print $3/$2 * 100}')

    # Log to CSV
    echo "$TIMESTAMP,$CPU_USAGE,$MEMORY_USAGE,$CPU_PER_FRAME" >> "$OUTPUT_FILE"

    sleep $INTERVAL_SECONDS
done
