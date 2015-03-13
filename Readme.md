Mac OS X driver for the NetLink BCM5906M Fast Ethernet PCI Express Adapter.  
Based on FreeBSD's bge(4) source code.

Download the .kext in the ["Releases"](https://github.com/vmatyushin/osx-bcm5906m/releases) section.

Available at http://code.google.com/p/osx-bcm5906m/ until the end of 2016.  
Tested in Mac OS X 10.6 and 10.7.  
Flow control and wake-on-lan is not implemented.  
**Note:** the kext is not signed as required by OS X 10.9 and 10.10.

#####Manual installation:
* Copy the "BCM5906MEthernet.kext" file to /System/Library/Extensions/
```bash
sudo cp -R BCM5906MEthernet.kext /System/Library/Extensions/
```
* Change the file permissions using the following commands:
```bash
sudo chown -R root:wheel /System/Library/Extensions/BCM5906MEthernet.kext
sudo chmod -R 755 /System/Library/Extensions/BCM5906MEthernet.kext
sudo touch /System/Library/Extensions
```

#####Automatic installation:
Use the Kext Utility (http://cvad-mac.narod2.ru/Kext_Utility)

License: see the file "Licence.txt"  
Broadcom is not responsible for this driver.
