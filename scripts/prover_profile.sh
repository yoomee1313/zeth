
CIRCUIT=2_2

ZETH_TOOL=./zeth_tool/zeth-tool
PK=~/dev/neodizk/test_data/zeth/zeth_${CIRCUIT}_pk.bin
VK=~/dev/neodizk/test_data/zeth/zeth_${CIRCUIT}_vk.bin
ASSIGNMENT=~/dev/neodizk/test_data/zeth/zeth_${CIRCUIT}_assignment.bin
PRIMARY_INPUT=~/dev/neodizk/test_data/zeth/zeth_${CIRCUIT}_primary.bin

if [ "$1" == "" ] ; then
    echo Usage: $0 '<tag>'
    exit 1
fi
TAG=$1

set -e
set -x

${ZETH_TOOL} prove --profile ${PK} ${ASSIGNMENT} proof.bin > zeth_${CIRCUIT}_${TAG}_proof_1.txt
${ZETH_TOOL} verify ${VK} ${PRIMARY_INPUT} proof.bin
${ZETH_TOOL} prove --profile ${PK} ${ASSIGNMENT} proof.bin > zeth_${CIRCUIT}_${TAG}_proof_2.txt
${ZETH_TOOL} verify ${VK} ${PRIMARY_INPUT} proof.bin
${ZETH_TOOL} prove --profile ${PK} ${ASSIGNMENT} proof.bin > zeth_${CIRCUIT}_${TAG}_proof_3.txt
${ZETH_TOOL} verify ${VK} ${PRIMARY_INPUT} proof.bin
