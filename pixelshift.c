#define _GNU_SOURCE         /* See feature_test_macros(7) */
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
    int32_t xshift, yshift;
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

    xshift = strtol(argv[3], NULL, 10);
    if (abs(xshift) > width) {
        fprintf(stderr, "shift must be between 0 and width\n");
        goto end;
    }

    yshift = strtol(argv[4], NULL, 10);
    if (abs(yshift) > width) {
        fprintf(stderr, "shift must be between 0 and width\n");
        goto end;
    }

    ret = asprintf(&out_path, "%s_shifted%d%d", argv[1], xshift, yshift);
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
        /* shift on the x axis */
        if (xshift > 0) {
            for (i = width * width - 1; i >= 0; i--)
                if ((i % width) >= xshift)
                    frame[i] = frame[i - xshift];
        } else if (xshift < 0) {
            for (i = 0; i < width * width; i++)
                if ((i % width) <= width + xshift)
                    frame[i] = frame[i - xshift];
        }
        /* shift on the y axis */
        if (yshift > 0) {
            for (i = width * width - 1; i >= 0; i--)
                if ((i / width) >= yshift)
                    frame[i] = frame[i - yshift * width];
        } else if (yshift > 0) {
            for (i = 0; i < width * width; i++)
                if ((i / width) <= width + yshift)
                    frame[i] = frame[i - yshift * width];
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
