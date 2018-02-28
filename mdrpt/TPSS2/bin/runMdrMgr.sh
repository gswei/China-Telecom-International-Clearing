#!/usr/bin/ksh

export MDR_SERVERID=TPSS2
cd $MDR_HOME/$MDR_SERVERID/var
if [ `uname` = "HP-UX" ]; then
    export JAVA_HOME=/opt/java6
    export JAVA_CMD="$JAVA_HOME/bin/java -d64"
else
    export JAVA_CMD="$JAVA_HOME/bin/java"
fi
export JAVA_LIB=$MDR_HOME/lib
export WHOAMI=`whoami`

export CLASSPATH=.:\
$JAVA_LIB:\
$JAVA_LIB/dom4j-1.6.1.jar:\
$JAVA_LIB/gson-2.2.2.jar:\
$JAVA_LIB/h2-1.3.169.jar:\
$JAVA_LIB/jaxen-1.1.1.jar:\
$JAVA_LIB/mina-core-2.0.5.jar:\
$JAVA_LIB/slf4j-api-1.6.6.jar:\
$JAVA_LIB/slf4j-nop-1.6.6.jar:\
$JAVA_LIB/mdr.jar

export MAIN_CLASS=com.hp.mdr.mgr.main.MdrmgrMain
export JAVA_OPTS="-Duser.timezone=GMT+8 -XX:+UseGetTimeOfDay"

start_app()
{
    OUT=./log/console.out
    export ARGS="TPSS2SLAVE"
    nohup $JAVA_CMD $JAVA_OPTS $MAIN_CLASS $ARGS >> $OUT 2>&1 &
}

kill_app()
{
    CNT=1
    if [ $1 -gt 0 ]; then
	kill $1
	while [ $CNT -lt 10 ]; do
	    sleep 1
	    kill -0 $1 2>/dev/null
	    if [ $? -eq 0 ]; then
		echo "waiting, CNT=$CNT"
	    else
		echo "stop ok"
		exit 0
	    fi
	    CNT=`expr $CNT + 1`
	done
    fi
}

PS_PATTERN="java.+$MAIN_CLASS.+TPSS2SLAVE"

if [ `uname` = "HP-UX" ]; then
    PID=`ps -efx|egrep "$PS_PATTERN"|grep $WHOAMI|grep -v grep|awk '{print $2}'`
else
    PID=`ps -ef |egrep "$PS_PATTERN"|grep $WHOAMI|grep -v grep|awk '{print $2}'`
fi

PID=${PID:=0}

RUN_MODE="info"
if [ $# -gt 0 ]; then
    RUN_MODE=$1
fi

if [ $RUN_MODE = "start" ]; then
    if [ $PID -gt 0 ]; then
	echo "Warning: [$PS_PATTERN] is still running, PID=$PID, can not start"
	exit 1
    fi
    start_app
elif [ $RUN_MODE = "info" ]; then
    if [ $PID -gt 0 ]; then
	echo "Info: [$PS_PATTERN] is still running, PID=$PID"
    else
	echo "Info: [$PS_PATTERN] not running, use '$0 start' to start"
    fi
    exit 0
elif [ $RUN_MODE = "stop" ]; then
    kill_app $PID
else
    echo "Error: bad script argument $RUN_MODE"
    exit 1
fi

