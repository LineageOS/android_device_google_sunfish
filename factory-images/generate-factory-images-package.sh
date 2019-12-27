#!/bin/sh

# Copyright 2019 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

source ../../../common/clear-factory-images-variables.sh
BUILD=6099722
DEVICE=sunfish
PRODUCT=sunfish
VERSION=qd4a.191227.001
SRCPREFIX=signed-
BOOTLOADER=s5-0.2-6085901
RADIO=g7150-00164.2-191220-B-6089074
source ../../../common/generate-factory-images-common.sh
