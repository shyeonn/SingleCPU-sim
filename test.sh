echo "Please enter a number(1-ari/2-shift/3-logical/4-compare/5-branch/6-jump/7-memory"
read number

# Check if the input is a number
if [[ $number =~ ^[0-9]+$ ]]; then
    echo "The entered number is $number."
case $number in
	1)
		./singleCPU testfile/ari.mem dmem.mem
	;;
	2)
		./singleCPU testfile/shift.mem dmem.mem
	;;
	3)
		./singleCPU testfile/logi.mem dmem.mem
	;;
	4)
		./singleCPU testfile/comp.mem dmem.mem
	;;
	5)
		./singleCPU testfile/branch.mem dmem.mem
	;;
	6)
		./singleCPU testfile/jump.mem dmem.mem
	;;
	7)
		./singleCPU testfile/memtest.mem dmem.mem
	;;
esac
else
    echo "Non-numeric value entered. Exiting the program."
fi

