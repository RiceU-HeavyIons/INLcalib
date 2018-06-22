#! /bin/csh -f

set filename = $argv[1]

rm -f $filename-filescopy.csh

./inl_analysis -f $filename 

#./corr_analysis -f $filename

./summary_analysis -f $filename

chmod 755 $filename-filescopy.csh

./$filename-filescopy.csh
