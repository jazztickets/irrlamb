#!/bin/bash
version=`grep 'GAME_VERSION=".*"' -o ../CMakeLists.txt | sed -r "s/GAME_VERSION=\"(.*)\"/\1/"`
gitver=`git log --oneline | wc -l`
base=irrlamb-${version}r${gitver}
pkg=${base}-src.tar.gz

mkdir -p out
tar --transform "s,^,${base}/," -czvf out/${pkg} -C ../ \
--exclude=${pkg} \
--exclude=move.{sh,bat} \
--exclude=*.swp \
--exclude=.git \
--exclude=working/irrlamb \
--exclude=deployment/out \
--exclude=deployment/*.sh \
src/ \
tools/ \
working/ \
deployment/ \
cmake/ \
CMakeLists.txt \
README \
CHANGELOG \
LICENSE

echo -e "\nMade ${pkg}"
