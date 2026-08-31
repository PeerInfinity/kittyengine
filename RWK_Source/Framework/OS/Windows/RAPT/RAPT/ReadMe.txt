
o Make the Swipe List able to put a text description under each item in the swipe list.

o Graphics

	o Render grayscale with a pixel shader...?

	o Just wanted to let myself know... I'm a fucking genius.  I came here to put a note to myself
	  that I needed to divorce image loading of gifs, jpegs, etc, from Image and put it in the OS
	  stuff... but it turns out I already did.


o TextEdit

	o Implement PASTE.

	o Implement Selection: 

		On a mouse device, right click brings up a box with cut/paste.
		On a touch device, touch and hold brings up a box with cut/paste.

		Make it look like IOS, that's better.

	o Implement right click and click/hold to pop up a cut/paste menu

	o I will need a way to create an in-OS edit box or dialog.  For advanced stuff,
	  it will be better to use an in-OS dialog... for instance, making it so a user
	  can enter username/password.


o Each newly created App should come with a simple DialogBox that can be overridden for drawing (but based off a
  DialogBox that is functional).  Should be able to invoke it thus:

	if (DialogBox(Font,Caption,Text,Type)==true) // Pressed yes


o A class to back up data on the Raptisoft Server would also be good!  Where you can just backup your whole game on the Raptisoft server...
  how about just zip up the game folder, and up it goes?

  Or how about "compress" an IOBuffer or savegame with 7zip?
	
o System

o Music
	
	o Allow me to put the jukebox file into a memory string instead of a file.

	o For some systems, we'll not be able to have music.  On those systems, the jukebox
	  will not exist, and there will just be a folder of WAVs.  So, just be prepared to 
	  stream the WAVs instead, if that's the case.

o Game Input

	o Controllers:

		o Allow an enumeration of "game controllers."  This will provide an ID for keyboard,
		  accelerometer, and all joysticks. It's always just a stick and buttons.

		o Keyboard controller should be customizable... which key provides what response.
		  Default is arrows are the stick, CTRL and ALT are the buttons.

		o Accelerometer is just another controller

		o Let me bind a Joypad to a controller





Later:

o Make fullscreen mode work better


THINGS TO TEST:

o Switching resolution on the fly
o Switching fullscreen/windowed on the fly

THINGS TO ADD WHEN NEEDED:

o A way to make sounds Dynamic.  This is especially set up for
  Solomon's Keep: those voices, they should load when needed.  BUT:
  they should also pre-cache when you go between levels (i.e. it should
  figure out what voices it will need for the next level up).

  So, you'll want to be able to make sounds Dynamic, a way to reference
  them so they load WITHOUT playing, and a way to flush them, too.  Like
  just FlushAllDynamics or something.

