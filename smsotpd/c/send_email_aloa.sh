#!/bin/bash

export PATH=/bin

sms="${1}"
otp="${2}"

echo "${otp}" | mail -s "otp" "${sms}@ba-aloa.de"
