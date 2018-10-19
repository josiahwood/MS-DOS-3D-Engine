# MS-DOS 3D Engine

## Introduction
I wrote this in my junior year of high school (2001) during my extra time in my programming class.  I had also been learning trigonometry, so this entire 3D engine is written with a high school trigonometry and geometry level of knowledge.  It's definitely got some divide by zero bug sprinkled throughout, but works overall.

## Compiling
I'm not even going to attempt to figure out how to compile this thing again.  At my high school we were just given MS-DOS machines running [Turbo C++](http://www.developerinsider.in/turbocpp/).

## Environment
I was able to get this to run on [FreeDOS 1.2](http://www.freedos.org/) using [VirtualBox 5.2](https://www.virtualbox.org/).  I followed the directions [here](http://wiki.freedos.org/wiki/index.php/VirtualBox) to do the installation.  However, I did experience the problem listed in the "Problems with VirtualBox?" section [here](http://www.freedos.org/download/), but the recommended solution worked.

When prompted for "What FreeDOS packages do you want to install?  I only needed to select "Base packages only".

## Floppy Disk
OK, so this is the most unfortunate part of this.  In high school we were required to do all of our work from a standard 3.5" 1.44MB floppy.  So part of this program is hard coded to look at the A: drive.

I used [Magic ISO Maker](http://www.magiciso.com/tutorials/miso-createfloppyimage.htm) to create a floppy disk image with a copy of the <code>Turbo</code> folder.

Attach this image to your VirtualBox machine.

## Execution
### Start the VM
In an attempt to preserve as much memory address space as possible I run FreeDOS with option "1 - Load FreeDOS with JEMMEX, no EMS (most UMBs), max RAM free".  Although I still get out of memory errors after playing with it for a while.  I don't remember having this problem in high school, so maybe someone knows how to configure FreeDOS better than me.

### Copy Files
Copy the <code>Turbo</code> folder to the C: drive.  For some reason I've had trouble running it directly from the A: drive.

<code>xcopy a:\turbo c:\turbo /e</code>

### World File
The program is hard coded to load <code>A:\TURBO\WORLD.TXT</code>.  There are a few different world files included.

### Execute
Run <code>C:\TURBO\3DENGINE.EXE</code>.

### Controls
|Key|Action|
|---|------|
|Up|Look Up|
|Down|Look Down|
|Left|Look Left|
|Right|Look Right|
|NumPad 8|Look Up|
|NumPad 2|Look Down|
|NumPad 4|Move Left (strafe)|
|NumPad 6|Move Right (strafe)|
|PageUp|Move Up|
|PageDown|Move Down|
|Tilde (~)|Stop|
|1|10% Speed|
|2|20% Speed|
|3|30% Speed|
|4|40% Speed|
|5|50% Speed|
|6|60% Speed|
|7|70% Speed|
|8|80% Speed|
|9|90% Speed|
|0|100% Speed|
|Backspace|Move Backward at 25% Speed|
|+|Increase Speed by 2%|
|-|Decrease Speed by 2%|
|A|Rotate the first object in WORLD.TXT|
|O|Options Menu|
|Z|Zoom Radar In|
|X|Zoom Radar Out|
|Left Mouse Button|Fire and Cycle Weapon|
|Escape|Exit|
