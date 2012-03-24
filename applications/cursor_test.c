/* Test program for the cursor library
 * Runs the mouse around in a circle, and then clicks
 * The click doesn't seem to register on some window types(?)
 * */
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include "cursor.h"

#define PI 3.14159

int
main (int argc, char *argv[])
{
    init_cursor();

    int iterations = 100;
    int r = 300;
    double x, y;
    int xbase = 960;
    int ybase = 540;
    int time = 20000;

    for (int i = 0; i < iterations; i++) {
        double theta = 2.0 * PI * i / iterations;
        x = r * cos(theta);
        y = r * sin(theta);
        x += xbase;
        y += ybase;
        printf("%f, %f, %f\n", theta, x, y);
        set_cursor(x, y);
        usleep(time);
    }

    printf("clicking mouse...\n");
    click_mouse(Button1);


}
