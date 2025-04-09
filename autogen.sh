#!/bin/sh

type autoreconf >/dev/null || {
  echo "autoreconf not found. please install the 'autoconf' package" >&2
  exit 1
}

autoreconf --force --install
