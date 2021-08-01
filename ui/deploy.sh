#!/bin/sh

#
# Copyright 2021 Kioshi Morosin <glam@hex.lc>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

root=$(dirname "$(realpath "$0")")
wd=$(pwd)

cd "$root" || exit
rev=$(git rev-parse master | head -c 8)

echo "deploying version $rev..."
cd "$root/deploy" || exit
git rm -rf ./*
git checkout HEAD -- CNAME
cp -r "$root/build/"* ./
git add ./*
git commit -S -m "[auto] deploy tracking $rev"
git push -f origin master
echo "deploy complete!"
cd "$wd" || exit
