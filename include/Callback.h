#pragma once
/*
 * Callback functions for FlTk
 */
class Phase2Window;
class Fl_Widget;


void idleCallback(Phase2Window *window);

void animateButtonCallback(Fl_Widget *widget, Phase2Window *window);
