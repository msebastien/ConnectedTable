# Connected Tables for LILLIAD learning center

## InterQarpe Server for Arduino

We recommand using the PlatformIO IDE for developing with the InterQarpe Server for Arduino.

## Information about the other programs

The datapoller and datasender programs compile only on GNU/Linux.

## Installation (Datapoller and Datasender)

'git clone https://github.com/msebastien/ConnectedTable.git
cd ~/ConnectedTable/datapoller && make
cd ~/ConnectedTable/datasender && make'

## Check status of these daemons

'sudo systemctl status qarpediem_datapoller'
'sudo systemctl status qarpediem_datasender'



