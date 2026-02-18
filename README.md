# Development Environment

## Linux Mint
* May need to install build-essential
* apt install git python3-venv python3-pip cmake libbluetooth-dev libglib2.0-dev libpcre3-dev mariadb-server libmariadb3 libmariadb-dev libfmt-dev libspdlog-dev doxygen

## Techneaux Jeaux - Clone Repository
* git clone https://github.com/cmdupre/TechneauxJeaux.git

## Jutta Protocol
* cd src/repos
* git clone https://github.com/Jutta-Proto/protocol-bt-cpp.git
   * But… use my forked and modified copy.
      * https://github.com/cmdupre/protocol-bt-cpp.git
      * git checkout TechneauxJeaux
* copy machinefiles directory from TechneauxJeaux repo to [clone-dir]/src/resources/
* If you don't have machinefiles directory, it will need to be built from the J.O.E. app apk.
* See Jutta-Proto github for more.
* I had to create a custom script to replace the included extract script because the xml file location in the apk has changed.
* Create python venv in [clone-dir]/build
   * mkdir build
   * python3 -m venv build
   * cd build
   * bin/pip install conan==1.66.0
      * Jutta-Proto github says to use an older version but I could not get that version to work.
   * cmake -DCMAKE_BUILD_TYPE=Release ..
   * make
   * sudo make install
* For some reason, date.hpp does not get installed, do it manually.
   * sudo cp -r [clone-dir]/src/include/date /usr/local/include/

## MariaDB
* sudo mysql_secure_installation
* follow installation instructions for new (c++) connector
    * https://mariadb.com/docs/server/connect/programming-languages/cpp/install/
    * I was able to install the .deb package with dpkg -i and didn't have to run any of the other install commands listed.
* database setup
    * https://mariadb.com/kb/en/mariadb-basics/
 * connection example
    * https://mariadb.com/resources/blog/how-to-connect-c-programs-to-mariadb/
  
## Techneaux Jeaux - Build
* git checkout prototype-dev
* create src/environment.h
```
#ifndef _ENVIRONMENT_H

const std::string ENVIRONMENT_DB_USER = "asdf";
const std::string ENVIRONMENT_DB_PASS = "asdf";

#define _ENVIRONMENT_H
#endif//_ENVIRONMENT_H
```
* cmake --preset prototype
* cmake --build --preset prototype
* cmake --install ./out/build/prototype

## Custom script to replace Jutta Protocol extract script

```
#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Invalid amount of arguments!" >&2
    echo "$0 JURA_JOE_APK_PATH.apk" >&2
    exit -1
fi

APK=$1

[ -e "machinefiles" ] && rm -rf "machinefiles"
mkdir "machinefiles"

for file in `find $1 -type f`
do
        cp $file machinefiles/
done

echo
echo "Done, manually copy JOE_MACHINES.TXT, ex: cp JOE_MACHINES.TXT ../../../../machinefiles/"
echo
```

# Runtime Environment

* Jutta Protocol library requires the "machinefiles" directory to be place next to the application executable. This should be handled by cmake install above.
* The check_and_run_jeaux script is a watchdog script to be run via crontab.

## Executable Directory

```
cdupre@Mini:~/src/TechneauxJeaux$ ll /home/cdupre/run_jeaux
total 6316
drwxrwxr-x  3 cdupre cdupre    4096 Feb  3 14:52 ./
drwxr-x--- 24 cdupre cdupre    4096 Feb  3 15:09 ../
-rwxrwxr-x  1 cdupre cdupre     145 Feb  3 14:46 check_and_run_jeaux.sh*
-rwxrwxr-x  1 cdupre cdupre 6449648 Feb  3 14:43 jeaux*
drwxrwxr-x  2 cdupre cdupre    4096 Feb  3 14:43 machinefiles/
```

## Run Script
*(check_and_run_jeaux.sh)*

```
#!/bin/bash

LOGFILE="/var/log/jeaux/jeaux.log"

if ! pgrep -x 'jeaux' > /dev/null
then
    cd /home/cdupre/run_jeaux
    ./jeaux >> "$LOGFILE" 2>&1 &
fi
```

## Crontab Entry

```
* * * * * /home/cdupre/run_jeaux/check_and_run_jeaux.sh
```

## Log File

```
cdupre@Mini:~/src/TechneauxJeaux$ ll /var/log/jeaux
total 32
drwxrwxrwx  2 root   root    4096 Feb  3 14:53 ./
drwxrwxr-x 16 root   syslog  4096 Feb  3 14:44 ../
-rw-rw-r--  1 cdupre cdupre 17991 Feb  3 15:11 jeaux.log
```

## Log Rotate

```
cdupre@Mini:~/src/TechneauxJeaux$ cat /etc/logrotate.d/jeaux 
/var/log/jeaux/jeaux.log {
    daily
    missingok
    rotate 7
    compress
    delaycompress
    notifempty
    su cdupre cdupre
    postrotate
        /bin/kill -HUP `pgrep jeaux`
    endscript
}
```

## Database Setup

```
cdupre@Mini:~/src/TechneauxJeaux$ mariadb -u root -p
Enter password: 
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 272
Server version: 10.11.8-MariaDB-0ubuntu0.24.04.1 Ubuntu 24.04

Copyright (c) 2000, 2018, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

MariaDB [(none)]> use jeaux;
Reading table information for completion of table and column names
You can turn off this feature to get a quicker startup with -A

Database changed
MariaDB [jeaux]> show tables;
+------------------------+
| Tables_in_jeaux        |
+------------------------+
| alerts                 |
| maintenanceCounters    |
| maintenancePercentages |
| productCounters        |
+------------------------+
4 rows in set (0.001 sec)

MariaDB [jeaux]> describe alerts;
+-----------+--------------+------+-----+---------+----------------+
| Field     | Type         | Null | Key | Default | Extra          |
+-----------+--------------+------+-----+---------+----------------+
| id        | bigint(20)   | NO   | PRI | NULL    | auto_increment |
| name      | varchar(255) | NO   |     | NULL    |                |
| type      | varchar(255) | NO   |     | NULL    |                |
| timestamp | datetime(3)  | NO   |     | NULL    |                |
+-----------+--------------+------+-----+---------+----------------+
4 rows in set (0.002 sec)

MariaDB [jeaux]> describe maintenanceCounters;
+-----------+----------------------+------+-----+---------+-------+
| Field     | Type                 | Null | Key | Default | Extra |
+-----------+----------------------+------+-----+---------+-------+
| name      | varchar(255)         | NO   | PRI | NULL    |       |
| count     | smallint(5) unsigned | NO   |     | NULL    |       |
| timestamp | datetime(3)          | NO   |     | NULL    |       |
+-----------+----------------------+------+-----+---------+-------+
4 rows in set (0.002 sec)

MariaDB [jeaux]> describe maintenancePercentages;
+------------+---------------------+------+-----+---------+-------+
| Field      | Type                | Null | Key | Default | Extra |
+------------+---------------------+------+-----+---------+-------+
| name       | varchar(255)        | NO   | PRI | NULL    |       |
| percentage | tinyint(3) unsigned | NO   |     | NULL    |       |
| timestamp  | datetime(3)         | NO   |     | NULL    |       |
+------------+---------------------+------+-----+---------+-------+
4 rows in set (0.002 sec)

MariaDB [jeaux]> describe productCounters;
+-----------+------------------+------+-----+---------+-------+
| Field     | Type             | Null | Key | Default | Extra |
+-----------+------------------+------+-----+---------+-------+
| name      | varchar(255)     | NO   | PRI | NULL    |       |
| code      | varchar(255)     | NO   | PRI | NULL    |       |
| count     | int(10) unsigned | NO   |     | NULL    |       |
| timestamp | datetime(3)      | NO   |     | NULL    |       |
+-----------+------------------+------+-----+---------+-------+
5 rows in set (0.002 sec)

MariaDB [jeaux]> 
```

## Temporary Table for Placing Default Order (coffee)
This will be changed later to include options like product name, strength, water amount, etc.
```
MariaDB [jeaux]> describe orders;
+-------+------------+------+-----+---------+-------+
| Field | Type       | Null | Key | Default | Extra |
+-------+------------+------+-----+---------+-------+
| geaux | tinyint(1) | NO   |     | NULL    |       |
+-------+------------+------+-----+---------+-------+
1 row in set (0.002 sec)
```
