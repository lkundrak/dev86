./sclongtest < longtest.dat >scl.out
./cclongtest < longtest.dat >ccl.out
diff scl.out ccl.out >longtest.diff
cat longtest.diff
