adb remount
adb shell setenforce 0
adb shell rm -rf /system/bin/egis_log/*
adb push libs\arm64-v8a\Talgo /system/bin
adb shell ./system/bin/Talgo
adb pull /system/bin/egis_log .
pause

