This assignment asks you to write a simple game akin to old text adventure games like Adventure:

http://en.wikipedia.org/wiki/Colossal_Cave_Adventure (Links to an external site.)Links to an external site.

You'll write two programs that will introduce you to programming in C on UNIX based systems, and will get you familiar with reading and writing files.

# Overview
This assignment is split up into two C programs (no other languages is allowed). The first program (hereafter called the "rooms program") will be contained in a file named `<STUDENT ONID USERNAME>.buildrooms.c`, which when compiled with the same name (minus the extension) and run creates a series of files that hold descriptions of the in-game rooms and how the rooms are connected.

The second program (hereafter called the "game") will be called `<STUDENT ONID USERNAME>.adventure.c` and when compiled with the same name (minus the extension) and run provides an interface for playing the game using the most recently generated rooms.

In the game, the player will begin in the "starting room" and will win the game automatically upon entering the "ending room", which causes the game to exit, displaying the path taken by the player.

During the game, the player can also enter a command that returns the current time - this functionality utilizes mutexes and multithreading.

For this assignment, do not use the C99 standard: this should be done using raw C (which is C89). In the complete example and grading instructions below, note the absence of the -c99 compilation flag.

# Specifications
## Rooms Program

The first thing your rooms program must do is create a directory called `<YOUR STUDENT ONID USERNAME>.rooms.<PROCESS ID OF ROOMS PROGRAM>`. Next, it must generate 7 different room files, which will contain one room per file, in the directory just created. You may use any filenames you want for these 7 room files, and these names should be hard-coded into your program. For example, if John Smith was writing the program, he might see this directory and filenames. Note that 19903 was the PID of the rooms program at the time it was executed, and was not hard-coded:

```sh
$ ls smithj.rooms.19903
Crowther_room Dungeon_room PLUGH_room PLOVER_room twisty_room XYZZY_room Zork_room
```

The elements that make up an actual room defined inside a room file are listed below, along with some additional specifications:

- A Room Name
  - A room name cannot be assigned to more than one room.
  - Each name can be at max 8 characters long, with only uppercase and lowercase letters allowed (thus, no numbers, special characters, or spaces). This restriction is not extended to the room file's filename.
  - You must hard code a list of ten different Room Names into your rooms program and have your rooms program randomly assign one of these to each room generated. Thus, for a given run of your rooms program, 7 of the 10 hard-coded room names will be used.
- A Room Type
  - The possible room type entries are: START_ROOM, END_ROOM, and MID_ROOM.
  - The assignment of which room gets which type should be randomly generated each time the rooms program is run.
  - Naturally, only one room should be assigned the START_ROOM type, and only one room should be assigned the END_ROOM type. The rest of the rooms will receive the MID_ROOM type.
- Outbound connections to other rooms
  - There must be at least 3 outbound connections and at most 6 outbound connections from this room to other rooms.
  - The oubound connections from one room to other rooms should be assigned randomly each time the rooms program is run.
  - Outbound connections must have matching connections coming back: if room A connects to room B, then room B must have a connection back to room A. Because of all of these specs, there will always be at least one path through.
  - A room cannot have an outbound connection that points to itself.
  - A room cannot have more than one outbound connection to the same room.

Each file that stores a room must have exactly this format, where the … is additional outbound room connections, as randomly generated:

```
ROOM NAME: <room name>
CONNECTION 1: <room name>
…
ROOM TYPE: <room type>
```

Here are the contents of files representing three sample rooms from a full set of room files (remember, you must use this same format). My list of room names includes the following, among others: XYZZY, PLUGH, PLOVER, twisty, Zork, Crowther, and Dungeon.

```
ROOM NAME: XYZZY
CONNECTION 1: PLOVER
CONNECTION 2: Dungeon
CONNECTION 3: twisty
ROOM TYPE: START_ROOM

ROOM NAME: twisty
CONNECTION 1: PLOVER
CONNECTION 2: XYZZY
CONNECTION 3: Dungeon
CONNECTION 4: PLUGH
ROOM TYPE: MID_ROOM

... (Other rooms) ...

ROOM NAME: Dungeon
CONNECTION 1: twisty
CONNECTION 2: PLOVER
CONNECTION 3: XYZZY
CONNECTION 4: PLUGH
CONNECTION 5: Crowther
CONNECTION 6: Zork
ROOM TYPE: END_ROOM
```

The ordering of the connections from a room to the other rooms, in the file, does not matter. Note that the randomization you do here to define the layout is not all that important: just make sure the connections between rooms, the room names themselves and which room is which type, is somewhat different each time the rooms program is run, however you want to do that. We're not evaluating your randomization procedure, though it's not acceptable to just randomize the room names yet use the same room structure every time.

I highly recommend building up the room graph in this manner: adding outbound connections two at a time (forwards and backwards), to randomly chosen room endpoints, until the map of all the rooms satisfies the requirements listed above. It's easy, requires no backtracking, and tends to generate sparser layouts. As a warning, the alternate method of choosing the number of connections beforehand that each room will have is not recommended, as it's hard to make those chosen numbers match the constraints of the map. To help do this correctly, please read the article 2.2 Program Outlining in Program 2 and consider using the room-generating pseudo-code listed there!

Here is an example of the rooms program being compiled and then run. Note that it returns NO OUTPUT, unless there is an error:

```
$ gcc -o smithj.buildrooms smithj.buildrooms.c
$ smithj.buildrooms
$
```

## Complete Example

Here is a complete example of the game being compiled and run, including the building of the rooms. Note the time command, incorrect spelling of a room name, the winning messages and formatting, and the return to the prompt with some examination commands following:

```
$ gcc -o smithj.buildrooms smithj.buildrooms.c
$ smithj.buildrooms
$ gcc -o smithj.adventure smithj.adventure.c -lpthread
$ smithj.adventure
CURRENT LOCATION: XYZZY
POSSIBLE CONNECTIONS: PLOVER, Dungeon, twisty.
WHERE TO? >Twisty

HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.

CURRENT LOCATION: XYZZY
POSSIBLE CONNECTIONS: PLOVER, Dungeon, twisty.
WHERE TO? >time

 1:03pm, Tuesday, September 13, 2016

WHERE TO? >twisty

CURRENT LOCATION: twisty
POSSIBLE CONNECTIONS: PLOVER, XYZZY, Dungeon, PLUGH.
WHERE TO? >Dungeon

YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!
YOU TOOK 2 STEPS. YOUR PATH TO VICTORY WAS:
twisty
Dungeon
$ echo $?
0
$ ls
currentTime.txt smithj.adventure smithj.adventure.c smithj.buildrooms
smithj.buildrooms.c smithj.rooms.19903
$ ls smithj.rooms.19903
Crowther_room Dungeon_room PLUGH_room PLOVER_room
twisty_room XYZZY_room Zork_room
$ cat smithj.rooms.19903/XYZZY_room
ROOM NAME: XYZZY
CONNECTION 1: PLOVER
CONNECTION 2: Dungeon
CONNECTION 3: twisty
ROOM TYPE: START_ROOM
```

# What to Turn In and When
What you’ll submit is a .zip file that contains both of your correctly named programs, which compile according to the instructions given above. As our Syllabus says, please be aware that neither the Instructor nor the TA(s) are alerted to comments added to the text boxes in Canvas that are alongside your assignment submissions, and they may not be seen. No notifications (email or otherwise) are sent out when these comments are added, so we aren't aware that you have added content! If you need to make a meta-comment about this assignment, please include it in a README file in your .zip file, or email the person directly who will be grading it (see the Home page for grading responsibilities).

The due date given below is the last minute that this can be turned in for full credit. The "available until" date is NOT the due date, but instead closes off submissions for this assignment automatically once 48 hours past the due date has been reached, in accordance with our Syllabus Grading policies.

# Grading
Remember that if your programs don't compile or run on our class server, you may receive a zero for the grade.

144 points are available for meeting all of the listed specifications, while the final 16 points will be based on your style, readability, and commenting. Comment well, often, and verbosely, approximately every four or five lines or so, as appropriate: we want to see that you are telling us WHY you are doing things, in addition to telling us WHAT you are doing.

The TAs will use this exact set of instructions: Program2 Grading.pdfPreview the document to grade your submission.
