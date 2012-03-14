#pragma once
/*
 * Callback functions for FlTk
 */
class Fl_Widget;
class Phase2Window;


void idleCallback(void *pData);

void animateButtonCallback(Fl_Widget *widget, Phase2Window *window);
