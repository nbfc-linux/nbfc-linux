#!/bin/sh

NBFC_SUB_DIR="info"

# Created below with mktemp -d. A fixed path under /tmp is predictable and lets
# any local user pre-create it (or plant symlinks inside it), which would make
# the root-owned redirections further down follow those symlinks.
NBFC_TEMP_DIR=
NBFC_DEST_DIR=

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

[ "$(id -u)" -eq 0 ] || {
  echo "Please run this script as root" >&2
  exit 1
}

NBFC_TEMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/nbfc-linux.make-archive.XXXXXXXX") || {
  echo "Failed to create temporary directory" >&2
  exit 1
}
NBFC_DEST_DIR="$NBFC_TEMP_DIR/$NBFC_SUB_DIR"

mkdir "$NBFC_DEST_DIR" || {
  cleanup
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
