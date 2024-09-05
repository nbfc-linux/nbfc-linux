#!/bin/bash

argparse_shell_complete=argparse-shell-complete

$argparse_shell_complete bash ec_probe.py      > bash/ec_probe.in
$argparse_shell_complete bash nbfc.py          > bash/nbfc.in
$argparse_shell_complete bash nbfc_service.py  > bash/nbfc_service.in

$argparse_shell_complete fish ec_probe.py      > fish/ec_probe.fish.in
$argparse_shell_complete fish nbfc.py          > fish/nbfc.fish.in
$argparse_shell_complete fish nbfc_service.py  > fish/nbfc_service.fish.in

$argparse_shell_complete zsh ec_probe.py       > zsh/_ec_probe.in
$argparse_shell_complete zsh nbfc.py           > zsh/_nbfc.in
$argparse_shell_complete zsh nbfc_service.py   > zsh/_nbfc_service.in
