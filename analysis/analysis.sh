#!/bin/csh -f
# OSX apparently requires us to redefine DYLD_LIBRARY_PATH for versions >10.10

setenv DYLD_LIBRARY_PATH `root-config --libdir`
set filename = $argv[1]

rm -f $filename-filescopy.csh

./inl_analysis -f $filename 

./corr_analysis -f $filename 

./summary_analysis -f $filename 

chmod 755 $filename-filescopy.csh
./$filename-filescopy.csh
mv $filename-filescopy.csh ./scripts/
