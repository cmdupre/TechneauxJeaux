# Development Environment

## Setup Environment (Jutta Protocol)
* install clang libbluetooth-dev glib-2.0 libpcre3 libpcre3-dev doxygen libc++-dev libc++abi-dev libspdlog-dev
* git clone https://github.com/Jutta-Proto/protocol-bt-cpp.git
    * (forked copy here: https://github.com/cmdupre/protocol-bt-cpp.git)
* copy machinefiles directory to [clone-dir]/src/resources/
    * If you don't have machinefiles directory, it will need to be built from the J.O.E. app apk.
    * See Jutta-Proto github for more.
    * I had to create a custom script to replace the included extract script because the xml file location in the apk has changed.
* Create python venv in [clone-dir]/build
    * python3 -m venv build
* cd build
* bin/pip install conan==1.66.0
    * Jutta-Proto github says to use an older version but I could not get that version to work.
* cmake -DCMAKE_BUILD_TYPE=Release ..
* make
* sudo make install
* For some reason, date.hpp does not get installed, do it manually.
    * sudo cp -r [clone-dir]/src/include/date /usr/local/include/

## Setup Environment (MariaDB)
* install mariadb-server libmariadb3 libmariadb-dev
* follow installation instructions for new connector
    * https://mariadb.com/docs/server/connect/programming-languages/cpp/install/
* run sudo mysql_secure_installation
* database setup
    * https://mariadb.com/kb/en/mariadb-basics/

# Runtime Environment

* Jutta Protocol library requires the "machinefiles" directory to be place next to the application executable.
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
    /home/cdupre/run_jeaux/jeaux >> "$LOGFILE" 2>&1 &
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
}
```

## Database Setup

```
cdupre@Mini:~/src/TechneauxJeaux$ mariadb -u root -p
Enter password:

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
3 rows in set (0.002 sec)

MariaDB [jeaux]> describe maintenancePercentages;
+------------+---------------------+------+-----+---------+-------+
| Field      | Type                | Null | Key | Default | Extra |
+------------+---------------------+------+-----+---------+-------+
| name       | varchar(255)        | NO   | PRI | NULL    |       |
| percentage | tinyint(3) unsigned | NO   |     | NULL    |       |
| timestamp  | datetime(3)         | NO   |     | NULL    |       |
+------------+---------------------+------+-----+---------+-------+
3 rows in set (0.002 sec)

MariaDB [jeaux]> describe productCounters;
+-----------+------------------+------+-----+---------+-------+
| Field     | Type             | Null | Key | Default | Extra |
+-----------+------------------+------+-----+---------+-------+
| name      | varchar(255)     | NO   | PRI | NULL    |       |
| code      | varchar(255)     | NO   | PRI | NULL    |       |
| count     | int(10) unsigned | NO   |     | NULL    |       |
| timestamp | datetime(3)      | NO   |     | NULL    |       |
+-----------+------------------+------+-----+---------+-------+
4 rows in set (0.002 sec)

MariaDB [jeaux]>
```