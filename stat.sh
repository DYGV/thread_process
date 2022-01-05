#/bin/bash

bin=$1
if [ -z ${bin} ]; then
    echo "実行バイナリを指定してください"
    exit 1
fi

hander(){
  exit 1
}

trap hander SIGINT

readonly measure_repeat_num=10

avail_processors=($(cat /proc/cpuinfo | grep processor | sed -e 's/[^0-9]//g'))
perf_stat="sudo perf stat -e cache-references,cache-misses,context-switches,cpu-migrations "
perf_stat+="-r ${measure_repeat_num}"
echo "Starting measurement using processor ${avail_processors[@]}."

for processor in ${avail_processors[@]}; do
    cpu_use+=${processor}
    output_file=${avail_processors}_${processor}.txt
    echo "Using processor ${cpu_use} to get the statistics"
    ${perf_stat} -o ${output_file} taskset -c ${cpu_use} ${bin} > /dev/null
    echo "Wrote the result in ${output_file}"
    cpu_use+=,
    echo "--------------------------------------"
done
