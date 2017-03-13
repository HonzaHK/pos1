SOURCE="p1.c"
EXECUTABLE="p1"
FLAGS="-std=gnu99 -Wall -Wextra -pedantic -pthread"

rm $EXECUTABLE
gcc $FLAGS $SOURCE -o $EXECUTABLE

./$EXECUTABLE $1 $2