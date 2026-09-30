#!/bin/bash

crazy_complete=${CRAZY_COMPLETE:-crazy-complete}

$crazy_complete --inherit-options=True --include=include/ec_probe.bash bash ec_probe.yaml > bash/ec_probe.in
$crazy_complete --inherit-options=True --include=include/ec_probe.fish fish ec_probe.yaml > fish/ec_probe.fish.in
$crazy_complete --inherit-options=True --include=include/ec_probe.zsh  zsh  ec_probe.yaml  > zsh/_ec_probe.in

$crazy_complete --inherit-options=True bash nbfc.yaml > bash/nbfc.in
$crazy_complete --inherit-options=True fish nbfc.yaml > fish/nbfc.fish.in
$crazy_complete --inherit-options=True zsh nbfc.yaml  > zsh/_nbfc.in

$crazy_complete --inherit-options=True bash nbfc_service.yaml > bash/nbfc_service.in
$crazy_complete --inherit-options=True fish nbfc_service.yaml > fish/nbfc_service.fish.in
$crazy_complete --inherit-options=True zsh nbfc_service.yaml  > zsh/_nbfc_service.in
