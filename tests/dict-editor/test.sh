exe=$1
echo $1
$exe < test0.in > test0.m.out
diff test0.out test0.m.out
rm test0.m.out
exit $?
