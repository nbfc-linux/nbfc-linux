#!/bin/sh
which autoreconf &>/dev/null || {
  echo "autoreconf not found. please install the 'autoconf' package"
  exit 1
}
autoreconf --force --install
