#!/bin/bash
pushd $(pwd)
cd docker/
docker build -t android-ndk-r20 .

popd
docker run -it -v $(pwd):/src android-ndk-r20 \
                /opt/android-ndk-r20/build/ndk-build -j8
echo "Done! checkout libs/"
