#pragma once
/*
 * Callback functions for FlTk
 */
class Fl_Widget;
class MainWindow;


void idleCallback(void *pData);

void animateButtonCallback(Fl_Widget *widget, MainWindow *window);

void addPointButtonCallback(Fl_Widget *widget, MainWindow *window);

void delPointButtonCallback(Fl_Widget *widget, MainWindow *window);

void curveTypeChoiceCallback(Fl_Widget *widget, MainWindow *window);

void viewTypeChoiceCallback(Fl_Widget *widget, MainWindow *window);

void shadowButtonCallback( Fl_Widget *widget, MainWindow *window );

void paramButtonCallback(Fl_Widget *widget, MainWindow *window);

void forwardButtonCallback(Fl_Widget *widget, MainWindow *window);

void backwardButtonCallback(Fl_Widget *widget, MainWindow *window);

void speedSliderCallback(Fl_Widget *widget, MainWindow *window);


