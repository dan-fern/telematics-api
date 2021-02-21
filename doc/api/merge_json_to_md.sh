#!/usr/bin/env bash

# CoPilot Mobile-TCU API
# Copyright (C) 2020, Jaguar Land Rover - All Rights Reserved.
# CONFIDENTIAL INFORMATION - DO NOT DISTRIBUTE

# Usage: generate_file "inputTemplateFile" "outputFile"

generate_file() {
  local inputFile="$1"
  local outputFile="$2"

  rm -f tempFile.md

  echo -n "## Version: " >> tempFile.md
  cat version >> tempFile.md

  while IFS= read -r line
  do
    if [[ $line =~ \.json$ ]] ; then
      cat $line >> tempFile.md
    else
      echo "$line" >> tempFile.md
    fi
  done < "$inputFile"

  rm -f $outputFile
  mv tempFile.md $outputFile
}

generate_file "API_template.md" "API.md"
generate_file "challenge_response_template.md" "challenge_response.md"
generate_file "crc_calculation_template.md" "crc_calculation.md"
generate_file "read/Vehicle_Status_Codes_template.md" "read/Vehicle_Status_Codes.md"

exit 0
