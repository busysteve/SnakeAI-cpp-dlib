while [ 1 -eq 1 ] ; do 
name=`date +"%Y%m%d-%H%M%S"`
./snake-ai 40 40 2000 4 nets/snake.htan.18x18.net-$name 440
sleep 30
done;






