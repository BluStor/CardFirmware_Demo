# CardFirmware

### Building the firmware

#### Dev Build (.s19 file)

1. On IAR select subproject `GC010-Bootloader 48K DEBUG` and press `F7`
2. On IAR select subproject `GC010-Firmware 48K DEBUG` and press `F7`
	* At this point the file `GC010-Firmware 48k.s19` has been created at `EWARM/GC010-Firmware 48K DEBUG/Exe`

#### Prod Build (.bin file)

1. On IAR select subproject `GC010-Firmware 48K BINARY` and press `F7`
	* At this point the file `Firmware.bin` has been created at `EWARM/GC010-Firmware 48K BINARY/Exe`
	* Usually is not necessary but you can algo create a .bin with an updated bootloader (there's an special build for this [here](http://drive.google.com/a/blustor.co/file/d/0BxVMhGBPtAnRLVVkRXQ3SW9SQms/view?ths=true) under BINARY_DEBUG instructions).

### Developing and branching

1. `git checkout development` Step into dev branch.
2. `git pull` Be sure to have the last commit.
3. `git checkout -b <new-branch-issue-num>` Create your new branch from here.
4. Work here and commit your work.
5. `git push -u origin <new-branch-issue-num>` Push your changes to a remote branch named the same as yours (you can also push as a different/more propper name).
6. On github:
	* Select your branch.
	* Click on `New pull request`.
	* On `base` select `development`.
	* Create pull request.
