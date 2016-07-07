#!/bin/bash 

#echo $1 $2
exec ./sim_game $1 $2 | ./game_log.pl 
