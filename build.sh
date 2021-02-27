#!/bin/bash

git submodule init depends/json
git submodule update --remote depends/json
echo "JSON parsing library downloaded."

git submodule update --init depends/picosha2
echo "SHA256 parsing library downloaded."

BUILD_COVERAGE_REPORT='BUILD_COVERAGE_REPORT=OFF'

while [ ! $# -eq 0 ]
do
	case "$1" in
		--sim | --simulator | -s | -S )
			BUILD_SIM='BUILD_SIM=ON'
            echo "NFS simulator to be built."
			;;
		--tests | --test | -t | -T )
			BUILD_TESTS='BUILD_TESTS=ON'
			git submodule init test/gtest
            git submodule update --remote test/gtest
            echo "googletest downloaded."
			;;
		--coverage | --cov | -c | -C )
			BUILD_COVERAGE_REPORT='BUILD_COVERAGE_REPORT=ON'
			echo "coverage report to be generated."
			;;
		--docs | --doc | -d | -D )
			BUILD_DOCS='BUILD_DOCS=ON'
			rm  ./doc/html/*.*
			echo "doxygen docs to be built."
			;;
		--help | -h | -H )
			echo
			echo "let there be help."
			echo
			;;
	esac
	shift
done

rm -rf ./doc/html/res
mkdir -p ./doc/html/res
cp -r ./doc/api/read ./doc/html/res/read
cp -r ./doc/api/write ./doc/html/res/write
cp ./doc/api/threat_data.png ./doc/html/res/threat_data.png
cp ./doc/vehiclestatuscodes.md ./doc/html/res/vehiclestatuscodes.md
echo "JSON message templates loaded for doxygen."

rm -rf ./build
mkdir build
cd ./build
cmake .. -D${BUILD_COVERAGE_REPORT}
make
cd ./../

if [ ${BUILD_SIM} ]
then
	  cd ./build
    cmake .. -D${BUILD_SIM}
    make
    cd ./../
	echo "Simulator built."
	echo "From build/ run '$ ./utils/aps_simulator/telematics-api-sim' for simulator."
	echo "- - -"
fi

if [ ${BUILD_TESTS} ]
then
    cd ./build
    cmake .. -D${BUILD_TESTS} -D${BUILD_COVERAGE_REPORT}
    make
    cd ./../
	echo "Tests built."
	echo "From build/ run '$ ./test/telematics-api-tests' for unit test suite."
	echo "- - -"
	if [ ${BUILD_COVERAGE_REPORT} == 'BUILD_COVERAGE_REPORT=ON' ]
	then
        cd build
		# ./test/telematics-api-tests
		# mkdir coverage
		cd ./..
		# gcovr -f src/ -f include/ -s --html --html-details -o ./build/coverage/coverage.html > cov_summary.txt
		# cat cov_summary.txt
		# rm cov_summary.txt
		# echo "Coverage report generated."
		# echo "Open build/coverage/coverage.html in your browser to see the report."
		# echo "- - -"
	fi
fi

if [ ${BUILD_DOCS} ]
then
	cd ./doc
	chmod +x build.sh
	./build.sh
	cd ./../
	echo "doxygen docs built."
	echo "From doc/ open 'html/index.html' for landing page."
	echo "- - -"
fi

echo "If no mobile device available, use mobile_placeholder.py to test JSON."
echo "From utils/ run '$ python mobile_placeholder.py' to send / receive JSON."
echo "- - -"

sleep 0.5s
echo "Repo built."
echo "From build/ run '$ ./telematics-api' for API."
