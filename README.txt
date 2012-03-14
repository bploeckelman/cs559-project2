Project 2 Phase 2 README

Authors:
Brian Ploeckelman
Matthew Bayer

3/13/12

To run this program, use visual studio to open the cs559-project2.vcxproj file.
You should not need to edit anything for the command line options.
Just click build all and run without debugging.

A window should pop up with some control points defined and one of the control
 points highlighted with 3 smaller cubes drawn by it.

If you click on the animate button, the three smaller cubes orbit the
 highlighted control point. If you click the animate button again, the
 animation stops.

Controls: 
---------
Left mouse button - select a control point
Right mouse button - if in ArcBallCam mode, clicking and dragging reorients the view
Space key - toggles between ArcBallCam mode and top-down view

There is console output that prints the selected point's position and orientation, 
or prints that nothing is selected if that is the case.