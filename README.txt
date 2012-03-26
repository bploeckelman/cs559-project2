Project 2 Phase 4 README

Authors:
Brian Ploeckelman
Matthew Bayer

3/25/2012

This program draws a catmull-rom curve based on a collection of control points, and moves a cube along the curve.  Control points can be added or removed, as well as moved around the scene in order to alter the shape of the curve.

There is console output that prints the selected point's position and orientation, 
or prints that nothing is selected if that is the case.


Controls: 
---------
Left mouse button  - select a control point (or deselect if no point is under the mouse)
Right mouse button - if in ArcBallCam mode, clicking and dragging reorients the view
Alt + Right mouse  - if in ArcBallcam mode, this pans the view

Space key - toggles between ArcBallCam mode and top-down view


Moving a selected control point:
(selected point is highlighted red)
-----------------------------------
Left mouse button - dragging moves the point in the XZ (ground) plane
Ctrl + Left mouse - dragging moves the point along the vertical axis (up-down)


Interface:
----------
Animate - toggles the movement of the cube along the curve
Add     - adds a new control point at a random location
Delete  - deletes the currently selected control point (if one is selected)
(Note: the program will not delete points if there are only two left)

Curve chooser - drop down box to select between supported curve types

Text box - displays the current interpolation value
