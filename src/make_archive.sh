#!/bin/sh

NBFC_TEMP_DIR="/tmp/nbfc-linux.make-archive.dir.temp"
NBFC_SUB_DIR="info"
NBFC_DEST_DIR="$NBFC_TEMP_DIR/$NBFC_SUB_DIR"

task_begin() {
  echo -n "$1 ... " >&2
}

task_end() {
  if [ $? -ne 0 ]; then
    echo "Failed" >&2
    cleanup
    exit 1
  else
    echo "Done" >&2
  fi
}

cleanup() {
  echo "Removing: '$NBFC_TEMP_DIR'" >&2
  rm -rf "$NBFC_TEMP_DIR"
}

copy_acpi_table() {
  task_begin "Copying: /sys/firmware/acpi/tables/$1"
  cat "/sys/firmware/acpi/tables/$1" > "$NBFC_DEST_DIR/$1"
  task_end
}

[ $# -eq 1 ] || {
  echo "Usage: $0 <FILE>" >&2
  exit 1
}

[ "$EUID" -eq 0 ] || {
  echo "Please run this script as root" >&2
  exit 1
}

mkdir -p "$NBFC_DEST_DIR" || {
  exit 1
}

task_begin "Running: nbfc sensors list"
nbfc sensors list > "$NBFC_DEST_DIR/sensors"
task_end

task_begin "Running: nbfc get-model-name"
nbfc get-model-name > "$NBFC_DEST_DIR/model_name"
task_end

copy_acpi_table DSDT

for i in $(seq 1 64); do
  if test -f "/sys/firmware/acpi/tables/SSDT$i"; then
    copy_acpi_table "SSDT$i"
  fi
done

task_begin "Running: tar -czf '$1' -C '$NBFC_TEMP_DIR' '$NBFC_SUB_DIR'"
tar -czf "$1" -C "$NBFC_TEMP_DIR" "$NBFC_SUB_DIR"
task_end

cleanup
