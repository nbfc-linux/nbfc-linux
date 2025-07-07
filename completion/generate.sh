#!/bin/bash

crazy_complete=${CRAZY_COMPLETE:-crazy-complete}

$crazy_complete bash ec_probe.yaml      > bash/ec_probe.in
$crazy_complete bash nbfc.yaml          > bash/nbfc.in
$crazy_complete bash nbfc_service.yaml  > bash/nbfc_service.in

$crazy_complete fish ec_probe.yaml      > fish/ec_probe.fish.in
$crazy_complete fish nbfc.yaml          > fish/nbfc.fish.in
$crazy_complete fish nbfc_service.yaml  > fish/nbfc_service.fish.in

$crazy_complete zsh ec_probe.yaml       > zsh/_ec_probe.in
$crazy_complete zsh nbfc.yaml           > zsh/_nbfc.in
$crazy_complete zsh nbfc_service.yaml   > zsh/_nbfc_service.in
