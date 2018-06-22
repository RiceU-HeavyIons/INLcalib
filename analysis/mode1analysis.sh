#! /bin/csh -f

set filename = $argv[1]

./inl_analysis -f $filename -mode 1

./corr_analysis -f $filename -mode 1 -tdc 0

./corr_analysis -f $filename -mode 1 -tdc 1

./corr_analysis -f $filename -mode 1 -tdc 2

./summary_analysis -f $filename -mode 1

