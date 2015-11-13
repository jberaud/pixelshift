/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

/* shift on x and y a square raw image file */

int main(int argc, char **argv)
{
    int in_fd = -1;
    int out_fd = -1;
    char *out_path = NULL;
    float xshift, yshift, xshift_dec, yshift_dec;
    int32_t xshift_int, yshift_int;
    int ret;
    uint8_t *frame =  NULL;
    int i;
    uint32_t width;

    if (argc != 5) {
        fprintf(stderr, "wrong number of arguments usage : pixelshift file width xshift yshift\n");
        goto end;
    }

    in_fd = open(argv[1], O_RDONLY);
    if (in_fd == -1) {
        fprintf(stderr, "can't open file %s\n", argv[1]);
        goto end;
    }

    width = strtoul(argv[2], NULL, 10);

    if (width == 0 || width > 640) {
        fprintf(stderr, "width must be between 0 and 640\n");
        goto end;
    }

    xshift = atof(argv[3]);
    if (abs(xshift) > width) {
        fprintf(stderr, "shift must be between 0 and width\n");
        goto end;
    }
    xshift_int = 0;

    yshift = atof(argv[4]);
    if (abs(yshift) > width) {
        fprintf(stderr, "shift must be between 0 and width\n");
        goto end;
    }
    yshift_int = 0;

    ret = asprintf(&out_path, "%s_shifted%2.2f-%2.2f", argv[1], xshift, yshift);
    if (ret == 0) {
        fprintf(stderr, "couldn't allocate out file name\n");
        goto end;
    }

    out_fd = open(out_path, O_CREAT | O_WRONLY| O_APPEND, S_IRUSR | S_IWUSR |
            S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    if (out_fd == -1) {
        fprintf(stderr, "couldn't open %s\n", out_path);
        goto end;
    }

    frame = (uint8_t *) malloc(width*width);
    if (frame == NULL) {
        fprintf(stderr, "couldn't allocate frame\n");
        goto end;
    }

    if (read(in_fd, frame, width*width) != width*width) {
        fprintf(stderr, "couldn't read frame\n");
        goto end;
    } else {
        /* shift by the integer value */
        /* shift on the x axis */
        if (xshift > 0) {
            xshift_int = floor(xshift);
            for (i = width * width - 1; i >= 0; i--)
                if ((i % width) >= xshift_int)
                    frame[i] = frame[i - xshift_int];
        } else if (xshift < 0) {
            xshift_int = ceil(xshift);
            for (i = 0; i < width * width; i++)
                if ((i % width) <= width - 1 + xshift_int)
                    frame[i] = frame[i - xshift_int];
        }
        /* shift on the y axis */
        if (yshift > 0) {
            yshift_int = floor(yshift);
            for (i = width * width - 1; i >= 0; i--)
                if ((i / width) >= yshift_int)
                    frame[i] = frame[i - yshift_int * width];
        } else if (yshift < 0) {
            yshift_int = ceil(yshift);
            for (i = 0; i < width * width; i++)
                if ((i / width) <= width - 1 + yshift_int)
                    frame[i] = frame[i - yshift_int * width];
        }

        /* shift by sub-integer value */
        xshift_dec = xshift - xshift_int;
        yshift_dec = yshift - yshift_int;

        /* shift on the x axis */
        if (xshift_dec > 0) {
            for (i = width * width -1; i >= 0; i--)
                if ((i % width) >= 1)
                    frame[i] = (1 - xshift_dec)*frame[i] +
                                xshift_dec*frame[i-1];
        } else if (xshift_dec < 0) {
            for (i = 0; i < width * width; i++)
                if ((i % width) < width -1 - 1)
                    frame[i] = -xshift_dec * frame[i] + 
                               (1 + xshift_dec)*frame[i+1];
        }
        
        /* shift on the y axis */
        if (yshift > 0) {
            for (i = width * width - 1; i >= 0; i--)
                if ((i / width) >= 1)
                    frame[i] = (1 - yshift_dec)*frame[i] +
                                yshift_dec * frame[i - width];
        } else if (yshift_int < 0) {
            for (i = 0; i < width * width; i++)
                if ((i / width) <= width - 1 - 1)
                    frame[i] = -yshift_dec*frame[i] +
                               (1 + yshift_dec)*frame[i - width];
        }
    }

    if (write(out_fd, frame, width*width) != width*width) {
        fprintf(stderr, "couldn't write output file\n");
    }

end:
    if (frame != NULL)
        free(frame);
    if (out_fd != -1)
        close(out_fd);
    if (out_path != NULL)
        free(out_path);
    if (in_fd != -1)
        close(in_fd);
    return 0;
}
