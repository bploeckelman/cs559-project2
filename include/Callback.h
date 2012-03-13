#pragma once
/*
 * Callback functions for FlTk
 */
class Fl_Widget;


void idleCallback(void *pData);

void animateButtonCallback(Fl_Widget *widget, Phase2Window *window);
