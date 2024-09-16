#!/bin/bash

argparse_shell_complete=argparse-shell-complete

$argparse_shell_complete bash ec_probe.yaml      > bash/ec_probe.in
$argparse_shell_complete bash nbfc.yaml          > bash/nbfc.in
$argparse_shell_complete bash nbfc_service.yaml  > bash/nbfc_service.in

$argparse_shell_complete fish ec_probe.yaml      > fish/ec_probe.fish.in
$argparse_shell_complete fish nbfc.yaml          > fish/nbfc.fish.in
$argparse_shell_complete fish nbfc_service.yaml  > fish/nbfc_service.fish.in

$argparse_shell_complete zsh ec_probe.yaml       > zsh/_ec_probe.in
$argparse_shell_complete zsh nbfc.yaml           > zsh/_nbfc.in
$argparse_shell_complete zsh nbfc_service.yaml   > zsh/_nbfc_service.in
