#!/bin/sh
# 
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, you can obtain one at https://mozilla.org/MPL/2.0/.
#
# Check that gzipped output can be decompressed and is the same is the
# uncompressed output.

COMP=./compactor
INSP=./inspector
DATAFILE=./dns.pcap
CMP=/usr/bin/cmp

tmpdir=`mktemp -d -t "same-output-xz.XXXXXX"`

cleanup()
{
    rm -rf $tmpdir
    exit $1
}

trap "cleanup 1" HUP INT TERM

# Run the converter twice, once with xz output.
$COMP -c /dev/null --raw-pcap $tmpdir/out.pcap -o $tmpdir/out.cbor $DATAFILE
if [ $? -ne 0 ]; then
    cleanup 1
fi

$COMP -c /dev/null --xz-pcap --raw-pcap $tmpdir/out2.pcap --xz-output -o $tmpdir/out2.cbor $DATAFILE
if [ $? -ne 0 ]; then
    cleanup 1
fi

# Run the inspector twice, once with xz output.
$INSP -o $tmpdir/out.insp.pcap $tmpdir/out.cbor
if [ $? -ne 0 ]; then
    cleanup 1
fi

$INSP --xz-output -o $tmpdir/out2.insp.pcap $tmpdir/out.cbor
if [ $? -ne 0 ]; then
    cleanup 1
fi

# Uncompress compressed files and compare to uncompressed versions.
unxz $tmpdir/out2.cbor.xz $tmpdir/out2.pcap.xz $tmpdir/out2.insp.pcap.xz
if [ $? -ne 0 ]; then
    cleanup 1
fi

cmp -s $tmpdir/out.cbor $tmpdir/out2.cbor && \
    cmp -s $tmpdir/out.pcap $tmpdir/out2.pcap && \
    cmp -s $tmpdir/out.insp.pcap $tmpdir/out2.insp.pcap
cleanup $?
