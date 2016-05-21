#/bin/sh

#  Test stub on host

./pt1-stub <test-in.txt >test-out.txt
diff test-ref.txt test-out.txt
