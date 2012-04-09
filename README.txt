Project 2 

Authors:
Brian Ploeckelman
Matthew Bayer
4/9/2012

This program draws a curve based on a collection of control points, and moves a train along the curve.  Control points can be added, removed, reoriented, and moved around the scene in order to alter the shape of the curve.

There is console output that prints the selected point's position and orientation, or prints that nothing is selected if that is the case.


Controls: 
---------
Left mouse button  - select a control point (or deselect if no point is under the mouse)

Right mouse button - if in ArcBallCam mode, clicking and dragging reorients the view
Alt + Right mouse  - if in ArcBallcam mode, this pans the view

'a' - switches to the arcball cam viewpoint
't' - switches to the train viewpoint
'o' - switches to the overhead viewpoint


Moving a selected control point:
(selected point is highlighted red)
-----------------------------------
Left mouse button - dragging moves the point in the XZ (ground) plane
Ctrl + Left mouse - dragging moves the point along the vertical axis (up-down)


Interface:
----------
Animate - toggles the movement of the train along the curve

Add     - adds a new control point next to the currently selected point
Delete  - deletes the currently selected control point (if one is selected)
(Note: the program will not delete points if there are only four left)

Curve chooser - drop down box to select between supported curve types
'Lines'
'Catmull-Rom'
'Cardinal Cubic' (with adjustable tension parameter)
'B-Spline'

Shadows - toggles shadow drawing

View - drop down box to choose between view points
'Arcball'  - view the scene using the arcball camera mode
'Train'    - view the scene from the train's perspective
'Overhead' - view the scene from a static top-down perspective

Speed - adjust the speed parameter 

Arclength Param - toggles arc-length reparameterization (constant speed)
<< / >> - moves the train forward or back a step at a time

Text box 1 - displays the current interpolation value 't'
Text box 2 - displays the current arc-length parameterization step value 's'

Highlight Current Segment - highlights the curve segment that the train is currently on, also colors the current start and end control points blue and the segment's other control points purple.

Reset - changes control point configuration to default
Load  - loads a new set of control points from a user-specified file
Save  - saves the current control points to a user-specified file

Reset Point - resets the selected control point's orientation to straight up
Pitch+/-    - adjusts the pitch of the selected control point
Roll+/-     - adjusts the roll of the selected control point

Tension - adjust the tension parameter for cardinal cubic curves


Features:
---------
- Calculation of local coordinate system for the train/curve in order to achieve correct orientation
- Arc-length parameterization, calculated by taking two steps along the curve, finding the distance between them, and using that to calculate a new step value for the animation parameter
- Adjustable speed parameter
- Movement in steps (forward and back)
- C0 curves (lines and cardinal cubics with a tension of 0)
- C1 curves (catmull-rom splines and cardinal cubics)
- C2 curves (cubic b-splines)
- Simple projected shadows
- Addition/removal of control points in place on the curve
- Direct manipulation of control point position and orientation
- Saving/loading control point configurations to/from text files while the program is running via fltk gui elements
- Different viewpoints
- Adjustable tension parameter for cardinal cubics
- Highlighting of the current segment and its corresponding control points
- Basic lighting (3 sources)
- Nice looking track with parallel rails and ties


Implementation Details:
-----------------------
- Nearly all the code in this project is written from scratch, the provided framework code was used as a guide, but the only code that stayed in from the framework is the ArcBallCam and 3d picking code

- Coordinate systems are calculated by taking the cross product of the tangent vector for the curve and the local orientation (normal) vector at a given location to get a binormal vector, then crossing the tangent vector with that binormal vector in order to get a proper orthonormal normal vector

- Local orientation (normal) vectors are calculated by lerping between the orientation vectors of the start and end control points for a curve segment (except for lines which just use the orientation of the starting control point for a segment)

- A local coordinate system for the train viewpoint is done this way also, however the orientation matrix used is the transpose of the standard orientation matrix

- The fltk libraries are included in the project for ease of deployment, although this increases the overall size of the project it eliminates the need to setup environment variables and ensure the proper version of fltk is installed... the program should be able to be run by simply opening the Visual Studio project file and hitting F5.
