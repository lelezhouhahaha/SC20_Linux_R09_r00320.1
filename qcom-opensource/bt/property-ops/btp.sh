#! /bin/sh
i=0
	while [ $i -le 10 ]
	do
		sleep 1
		echo $i > /data/btpcnt
		if [ ! -S "/data/misc/bluetooth/btprop" ]; then
			systemctl stop btproperty
			systemctl start btproperty	
		else
			break
		fi
		let i++
	done
