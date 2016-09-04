#!/bin/bash

export PATH=/bin

email="${1}"
otp="${2}"

echo "${otp}" | mail -s "otp" "${email}"
